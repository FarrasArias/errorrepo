#pragma once

#include <iostream>
#include <vector>
#include <tuple>
#include <map>
#include <set>

#include <iostream>
#include <fstream>
#include <sstream>

#include "../../../libraries/midifile/include/Binasc.h"
#include "../../../libraries/midifile/include/MidiFile.h"

#include "../../../libraries/protobuf/build/midi.pb.h"
#include "../../common/midi_parsing/util_protobuf.h"
#include "../../common/data_structures/track_type.h"
#include "../../common/data_structures/encoder_config.h"

#include "../../common/midi_parsing/adjacent_range.h"

#include <google/protobuf/util/json_util.h>

// START OF NAMESPACE
namespace midi_io {

#define QUIET_CALL(noisy) { \
    std::cout.setstate(std::ios_base::failbit);\
    std::cerr.setstate(std::ios_base::failbit);\
    (noisy);\
    std::cout.clear();\
    std::cerr.clear();\
}

int quantize_beat(double x, double TPQ, double SPQ, double cut=.5) {
  return (int)((x / TPQ * SPQ) + (1.-cut)) * (TPQ / SPQ);
}

bool event_comparator(const midi::Event a, const midi::Event b) { 
  if (a.time() != b.time()) {
    return a.time() < b.time();
  }
  if (std::min(a.velocity(),1) != std::min(b.velocity(),1)) {
    return std::min(a.velocity(),1) < std::min(b.velocity(),1);
  }
  return a.pitch() < b.pitch();
}

// TODO: Implement this as a Struct. Check the project (Rafa, local) mmm_for_testing_functions, main.cpp for the work in progress...
// Currently implements a structure that defines [track, channel, inst, track_type] in that order
using TRACK_IDENTIFIER = std::tuple<int,int,int,int>; // CHANGE

// currently we don't do quantization
// this can be done elsewhere

class MidiParsedData {
public:
    smf::MidiFile midi_file;
    int track_count;
    int ticks_per_quarter_note;

    MidiParsedData(std::string file_path) {
        QUIET_CALL(midi_file.read(file_path));
        midi_file.makeAbsoluteTicks();
        midi_file.linkNotePairs();
        track_count = midi_file.getTrackCount();
        ticks_per_quarter_note = midi_file.getTPQ();
    }
};

class Parser {
public:
  Parser(std::string filepath, midi::Piece *piece, data_structures::EncoderConfig *config) {
      Parse(filepath, piece, config);
  }
  static const int DRUM_CHANNEL = 9;
  data_structures::EncoderConfig *ec;
  int track_count;
  int TPQ;
  int SPQ;
  int current_track;
  int max_tick;
  smf::MidiEvent *mevent;
  std::map<TRACK_IDENTIFIER,int> track_map;
  std::map<int,TRACK_IDENTIFIER> rev_track_map;
  std::map<int,std::tuple<int,int,int>> timesigs;
  std::map<int,std::tuple<int,int,int,int>> bars; // value is (beatlength,count,num,dem)
  std::vector<std::vector<midi::Event>> events; // events split into tracks
  std::array<int,64> instruments; // instruments on each channel

  void SetMemberVariables(data_structures::EncoderConfig* config, MidiParsedData* parsed_file) {
      // TODO: Is this the best way?
      ec = config;
      TPQ = parsed_file->ticks_per_quarter_note;
      SPQ = ec->resolution;

      if (TPQ < SPQ) {
          throw std::runtime_error("MIDI FILE HAS INVALID TICKS PER QUARTER.");
      }
  }

  void FillPiece(midi::Piece* piece, MidiParsedData* parsed_file) {
      piece->set_resolution(SPQ);
      max_tick = 0;
      current_track = 0;

      for (int track = 0; track < parsed_file->track_count; track++) {
          current_track = track;
          std::fill(instruments.begin(), instruments.end(), 0); // zero instruments
          for (int event = 0; event < parsed_file->midi_file[track].size(); event++) {
              mevent = &(parsed_file->midi_file[track][event]);
              if (mevent->isPatchChange()) {
                  handle_patch_message(mevent);
              }
              else if (mevent->isTimeSignature()) {
                  handle_time_sig_message(mevent);
              }
              else if (mevent->isTempo()) {
                  piece->set_tempo(mevent->getTempoBPM());
              }
              else if (mevent->isNoteOn() || mevent->isNoteOff()) {
                  handle_note_message(mevent);
              }
          }
      }

      if (max_tick <= 0) {
          throw std::runtime_error("MIDI FILE HAS NO NOTES");
      }
  }

  void ProcessTimeSignatures(MidiParsedData* parsed_file){
      // add a timesig at beginning and end
        // and then make a mapping from tick to bar_number and bar_length
      int count = 0;
      if (timesigs.find(0) == timesigs.end()) {
          timesigs[0] = std::make_tuple(parsed_file->ticks_per_quarter_note * 4, 4, 4); // assume 4/4
      }
      // if we do max_tick + TPQ instead we end up with an extra bar
      timesigs[max_tick] = std::make_tuple(0, 0, 0); // no bar length
      for (const auto& p : midi_parsing::make_adjacent_range(timesigs)) {
          if (std::get<0>(p.first.second) > 0) {
              for (int t = p.first.first; t < p.second.first; t += std::get<0>(p.first.second)) {
                  auto ts = p.first.second;
                  bars[t] = std::make_tuple(std::get<0>(ts), count, std::get<1>(ts), std::get<2>(ts));
                  count++;
              }
          }
      }
  }

  void CreateMidiPiece(midi::Piece* piece, MidiParsedData* parsed_file) {
      // construct the piece
      midi::Track* track = NULL;
      midi::Bar* bar = NULL;
      midi::Event* event = NULL;

      for (int track_num = 0; track_num < events.size(); track_num++) {

          // sort the events in each track
          // at this point ticks are still absolute
          std::sort(events[track_num].begin(), events[track_num].end(), event_comparator);

          // add track and track metadata
          track = piece->add_tracks();
          track->set_instrument(std::get<2>(rev_track_map[track_num]));
          track->set_track_type(
              (midi::TRACK_TYPE)std::get<3>(rev_track_map[track_num]));
          //track->set_is_drum( is_drum_track(std::get<3>(rev_track_map[track_num])) );

          // apply force monophonic to bass track
          if (track->track_type() == midi::OPZ_BASS_TRACK) {
              std::vector<midi::Event> mono_events = util_protobuf::ConvertToMonophonic(events[track_num]);
              std::sort(mono_events.begin(), mono_events.end(), event_comparator);
              events[track_num] = mono_events;
          }

          // add bars and bar metadata
          for (const auto& bar_info : bars) {
              bar = track->add_bars();
              bar->set_internal_beat_length(std::get<0>(bar_info.second) / parsed_file->ticks_per_quarter_note);
              bar->set_ts_numerator(std::get<2>(bar_info.second));
              bar->set_ts_denominator(std::get<3>(bar_info.second));
          }

          // add events
          for (int j = 0; j < events[track_num].size(); j++) {
              int velocity = events[track_num][j].velocity();
              int tick = events[track_num][j].time();
              auto bar_info = get_bar_info(tick, velocity > 0);

              bar = track->mutable_bars(std::get<2>(bar_info)); // bar_num

              bar->add_events(piece->events_size());
              event = piece->add_events();
              event->CopyFrom(events[track_num][j]);

              int rel_tick = round((double)(tick - std::get<0>(bar_info)) / parsed_file->ticks_per_quarter_note * SPQ);
              event->set_time(rel_tick); // relative
          }
      }
  }

  void RemapTracksForPolyphony() {
      // remap track types based on max polyphony
          // TODO: IT SEEMS WERE NOT USING THIS>
          //if (ec->te) {
          //    UpdateMaxPolyphony(piece);
          //    for (int track_num = 0; track_num < piece->tracks_size(); track_num++) {
          //        midi::Track* track = piece->mutable_tracks(track_num);
          //        int max_polyphony = track->internal_features(0).max_polyphony();
          //        midi::TRACK_TYPE tt = track->track_type();
          //        if (tt == midi::AUX_INST_TRACK) {
          //            if (max_polyphony <= 2) {
          //                track->add_internal_train_types(midi::OPZ_ARP_TRACK);
          //            }
          //            if (max_polyphony <= 3) { // previous was else if ????
          //                track->add_internal_train_types(midi::OPZ_LEAD_TRACK);
          //            }
          //            if (max_polyphony <= 4) {
          //                track->add_internal_train_types(midi::OPZ_CHORD_TRACK);
          //            }
          //            else {
          //                track->add_internal_train_types(midi::AUX_INST_TRACK); // unused
          //            }
          //        }
          //        else if ((tt == midi::OPZ_KICK_TRACK) && (max_polyphony > 2)) {
          //            track->add_internal_train_types(
          //                midi::OPZ_INVALID_DRUM_TRACK);
          //        }
          //        else if ((tt == midi::OPZ_SNARE_TRACK) && (max_polyphony > 2)) {
          //            track->add_internal_train_types(
          //                midi::OPZ_INVALID_DRUM_TRACK);
          //        }
          //        else if ((tt == midi::OPZ_HIHAT_TRACK) && (max_polyphony > 2)) {
          //            track->add_internal_train_types(
          //                midi::OPZ_INVALID_DRUM_TRACK);
          //        }
          //        else if ((tt == midi::OPZ_BASS_TRACK) && (max_polyphony > 1)) {
          //            track->add_internal_train_types(
          //                midi::OPZ_INVALID_BASS_TRACK);
          //        }
          //        else if ((tt == midi::OPZ_SAMPLE_TRACK) && (max_polyphony > 2)) {
          //            track->add_internal_train_types(
          //                midi::OPZ_INVALID_SAMPLE_TRACK);
          //        }
          //        else {
          //            track->add_internal_train_types(tt);
          //        }
          //    }
          //}
  }

    
  void Parse(std::string filepath, midi::Piece* piece, data_structures::EncoderConfig* config) {
    MidiParsedData parsed_file = MidiParsedData(filepath);
    SetMemberVariables(config, &parsed_file);
    FillPiece(piece, &parsed_file);
    ProcessTimeSignatures(&parsed_file);
    CreateMidiPiece(piece, &parsed_file);
    //RemapTracksForPolyphony(); 
  }

  int infer_voice(int channel) {
    int track_type = midi::STANDARD_TRACK;
    if (channel == DRUM_CHANNEL) {
      track_type = midi::STANDARD_DRUM_TRACK;
    }
    return track_type;
  }

  TRACK_IDENTIFIER join_track_info(int track, int channel, int inst) {
      return std::make_tuple(track, channel, inst, infer_voice(channel));
  }

  std::tuple<int,int,int> get_bar_info(int tick, bool is_onset) {
    // returns bar_start, bar_length, bar_num tuple
    auto it = bars.upper_bound(tick);
    if (it == bars.begin()) {
      //cout << "TICK : " << tick << " - " << is_onset << std::endl;
      throw std::runtime_error("CAN'T GET BAR INFO FOR TICK!");
    }
    it = prev(it);
    if ((it->first == tick) && (!is_onset)) {
      // if the note is an offset and the time == the start of the bar
      // push it back to the previous bar
      if (it == bars.begin()) {
        //cout << "TICK : " << tick << " - " << is_onset << std::endl;
        throw std::runtime_error("CAN'T GET BAR INFO FOR TICK!");
      }
      it = prev(it);
    }
    return std::make_tuple(it->first, std::get<0>(it->second), std::get<1>(it->second));
  }

  void handle_patch_message(smf::MidiEvent *mevent) {
    int channel = mevent->getChannelNibble();
    instruments[channel] = (int)((*mevent)[1]);
  }

  void handle_time_sig_message(smf::MidiEvent *mevent) {
    int numerator = (*mevent)[3];
    int denominator = 1<<(*mevent)[4];
    int barlength = (double)(TPQ * 4 * numerator / denominator);
    if (barlength >= 0) {
      //timesigs[mevent->tick] = barlength;
      timesigs[mevent->tick] = std::make_tuple(barlength, numerator, denominator);
    }
  }

  bool is_event_offset(smf::MidiEvent *mevent) {
    return ((*mevent)[2]==0) || (mevent->isNoteOff());
  }

  void add_event(TRACK_IDENTIFIER &track_info, int tick, int pitch, int velocity) {
    midi::Event event;
    event.set_time( tick );
    //event.set_qtime( tick / (TPQ/SPQ) ); // we need this for force mono
    event.set_pitch( pitch );
    event.set_velocity( velocity );
    events[track_map[track_info]].push_back( event );
  }

  void handle_note_message(smf::MidiEvent *mevent) {
    int channel = mevent->getChannelNibble();
    int pitch = (int)(*mevent)[1];
    int velocity = (int)(*mevent)[2];

    if ((!mevent->isLinked()) && (channel != 9)) {
      // we do not include unlinked notes unless they are drum
      return;
    }

    if (mevent->isNoteOff()) {
      velocity = 0; // sometimes this is not the case
    }

    smf::MidiEvent *linked_event = mevent->getLinkedEvent();

    int tick = mevent->tick;
    if (!ec->unquantized) {
      tick = quantize_beat(mevent->tick, TPQ, SPQ);
    }

    bool is_offset = is_event_offset(mevent);

    // ignore note offsets at start of file
    if (is_offset && (tick==0)) {
      return;
    }

    TRACK_IDENTIFIER track_info = join_track_info(
      current_track,channel,instruments[channel]);

    // track_info has info for new tracks per channel. If we can't find that info, we update track_map indicating there's a new
    // track, and then we push a vector of events (preparing to fill that vector with events in the future).
    // update track map
    if (track_map.find(track_info) == track_map.end()) {
      int current_size = track_map.size();
      track_map[track_info] = current_size;
      rev_track_map[current_size] = track_info;
      events.push_back( std::vector<midi::Event>() );
    }

    // make all drum notes really short
    // TODO: isn't this already taken care of in the protobuf code??????
    if (channel == 9) {
      if (!is_offset) {
        add_event(track_info, tick, pitch, velocity);
        add_event(track_info, tick + (TPQ/SPQ), pitch, 0);
      }
    }
    else {
      add_event(track_info, tick, pitch, velocity);
    }

    max_tick = std::max(max_tick, mevent->tick);
  }
};

void ParseSong(std::string filepath, midi::Piece *midi_piece, data_structures::EncoderConfig *encoder_config) {
  Parser parser(filepath, midi_piece, encoder_config);
}

void write_midi(midi::Piece* p, std::string& path, int single_track = -1) {
    static const int DRUM_CHANNEL = 9;
    if (p->tracks_size() >= 15) {
        throw std::runtime_error("TOO MANY TRACKS FOR MIDI OUTPUT");
    }

    //set_beat_lengths(p); // make sure they exist

    // we should find a way not to output so many tracks

    smf::MidiFile outputfile;
    outputfile.absoluteTicks();
    outputfile.setTicksPerQuarterNote(p->resolution());
    outputfile.addTempo(0, 0, p->tempo());
    outputfile.addTrack(16); // ensure drum channel

    int track_num = 0;
    for (const auto track : p->tracks()) {
        if ((single_track < 0) || (track_num == single_track)) {
            int bar_start_time = 0;
            int patch = track.instrument();
            int channel = enums::SAFE_TRACK_MAP[track_num];
            if (data_structures::is_drum_track(track.track_type())) {
                channel = DRUM_CHANNEL;
            }
            outputfile.addPatchChange(channel, 0, channel, patch);

            for (const auto bar : track.bars()) {
                for (const auto event_index : bar.events()) {
                    const midi::Event e = p->events(event_index);
                    //std::cout << "WRITING :: " << bar_start_time + e.time() << " " << e.pitch() << " " << e.velocity() << std::endl;
                    outputfile.addNoteOn(
                        channel, // same as channel
                        bar_start_time + e.time(), // time
                        channel, // channel  
                        e.pitch(), // pitch
                        e.velocity()); // velocity (need some sort of conversion)
                }
                bar_start_time += bar.internal_beat_length() * p->resolution();
            }
        }
        track_num++;
    }

    outputfile.sortTracks();         // make sure data is in correct order
    outputfile.write(path.c_str()); // write Standard MIDI File twinkle.mid
}
}
// END OF NAMESPACE