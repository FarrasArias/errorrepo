#pragma once

#include <google/protobuf/util/json_util.h>

#include <cmath>
#include <vector>
#include <algorithm>
#include "../../../libraries/protobuf/build/midi.pb.h"
#include "../../common/data_structures/track_type.h"
#include "../../common/data_structures/encoder_config.h"

#include "../../inference/enum/density.h"
#include "../../inference/enum/density_opz.h"
#include "../../inference/enum/constants.h"
#include "../../inference/enum/gm.h"
#include "../../inference/random.h"


#define M_LOG2E 1.4426950408889634074

// START OF NAMESPACE
namespace util_protobuf {

	// Checks if tracks has features and returns them
	midi::TrackFeatures* GetTrackFeatures(midi::Piece* midi_piece, int track_num) {
		if ((track_num < 0) || (track_num >= midi_piece->tracks_size())) {
			throw std::runtime_error("TRACK FEATURE REQUEST OUT OF RANGE");
		}
		//we return a pointer to the mutable track object with index track_num and we store the pointer in midi_track
		midi::Track* midi_track = midi_piece->mutable_tracks(track_num);
		if (midi_track->internal_features_size() == 0) {
			//adds new element to end of field and returns a pointer. The returned track features is mutable and will have none of its fields set.
			return midi_track->add_internal_features();
		}
		//returns a pointer to the underlying mutable track object with index track_num and we return the pointer
		//TODO: Does it make sense that TrackFeatures is a repeated field, and we always return the index 0??
		return midi_track->mutable_internal_features(0);
	}

	// Get the number of bars in a piece
	int GetNumBars(midi::Piece* midi_piece) {
		if (midi_piece->tracks_size() == 0) {
			return 0;
		}
		std::set<int> track_num_bars;
		for (const auto track : midi_piece->tracks()) {
			track_num_bars.insert(track.bars_size());
		}
		if (track_num_bars.size() > 1) {
			throw std::runtime_error("Each track must have the same number of bars!");
		}
		//we dereference the pointer to the first element in the set (in this case the only element)
		return *track_num_bars.begin();
	}

	// ================================================================
	// Functions to update the note_polyphony field in the midi::Tracks of a midi::Piece
	// ================================================================

	midi::Note CreateNote(int start, int end, int pitch) {
		midi::Note note;
		note.set_start(start);
		note.set_end(end);
		note.set_pitch(pitch);
		return note;
	}

	// Go over all the bars and convert midi::events to midi::notes
	// TODO: Is there a way to decouple this function from its parent (obtain *duration_in_ticks some other way)
	std::vector<midi::Note> IterateAndConvert(midi::Piece* midi_piece, const midi::Track* current_track, bool bool_drum_track, int* duration_in_ticks) {
		midi::Event current_midi_event;
		std::vector<midi::Note> notes;
		std::map<int, int> onsets;
		int bar_start = 0;
		for (int bar_num = 0; bar_num<current_track->bars_size(); bar_num++) {
			const midi::Bar bar = current_track->bars(bar_num);
			for (auto event_id : bar.events()) {
				current_midi_event = midi_piece->events(event_id);
				if (current_midi_event.velocity() > 0) {
					// need to account for bar offset to get correct start time
					onsets[current_midi_event.pitch()] = bar_start + current_midi_event.time();
				}
				else {
					auto last_event_with_pitch = onsets.find(current_midi_event.pitch());
					// need to account for bar offset to get correct end time
					int end_time = bool_drum_track ? last_event_with_pitch->second + 1 : bar_start + current_midi_event.time();
					if (last_event_with_pitch != onsets.end()) {
						midi::Note note = CreateNote(last_event_with_pitch->second, end_time, last_event_with_pitch->first);
						notes.push_back(note);
						onsets.erase(last_event_with_pitch);
					}
				}
				*duration_in_ticks = std::max(*duration_in_ticks, bar_start + current_midi_event.time());
			}
			bar_start += midi_piece->resolution() * bar.internal_beat_length();
		}
		return notes;
	}

	// Get a specific track from a midi piece and convert its midi::events to midi::notes
	std::vector<midi::Note> TrackEventsToNotes(midi::Piece* midi_piece, int track_num, int* duration_in_ticks) {
		bool bool_drum_track = data_structures::is_drum_track(midi_piece->tracks(track_num).track_type()); //TODO: this should be renamed is_drum_track = check_if_drum_track()... refactor
		const midi::Track* current_track = &(midi_piece->tracks(track_num));
		std::vector<midi::Note> notes = IterateAndConvert(midi_piece, current_track, bool_drum_track, duration_in_ticks); //TODO: This is a mayor change, but maybe the .proto shouldn't keep the events int the Piece, and instead keep them in the track message type
		return notes;
	}

	// Get the notes playing simultaneously per tick and return the tick with most note count.
	int GetTrackMaxPolyphony(std::vector<midi::Note>& notes, int duration_in_ticks) {
		if (duration_in_ticks > 100000) {
			throw std::runtime_error("MAX TICK TO LARGE!");
		}
		int max_polyphony = 0;
		std::vector<int> flat_roll(duration_in_ticks, 0);
		for (const auto note : notes) {
			for (int tick = note.start(); tick < note.end(); tick++) {
				flat_roll[tick]++;
				// TODO: Check... why are we checking the whole roll every note... 
				// couldn't we just get the max from the roll once we did all the ++? 
				max_polyphony = std::max(flat_roll[tick], max_polyphony);
			}
		}
		return max_polyphony;
	}

	// Iterate over the tracks of a midi piece and update the max_polyphony field in each
	void UpdateMaxPolyphony(midi::Piece* midi_piece) {
		for (int track_num = 0; track_num < midi_piece->tracks_size(); track_num++) {
			int duration_in_ticks = 0;
			std::vector<midi::Note> notes = TrackEventsToNotes(midi_piece, track_num, &duration_in_ticks);
			GetTrackFeatures(midi_piece, track_num)->set_max_polyphony(GetTrackMaxPolyphony(notes, duration_in_ticks));
		}
	}

	// ================================================================
	// Functions to convert a polyphonic track to a monophonic one
	// ================================================================

	// We create an array of monophonic events
	// we iterate over events
	// if an event starts, we flag it.
	// if another event starts before the flag is down, we force the first event to end and 
	// be pushed in the array. We then flag the new event as being played
	// if the event ends before another starts, we just push it in the array

	void StopAndPushEvent(const auto& midi_event, std::vector<midi::Event>& mono_events, midi::Event& last_onset, bool& note_sounding) {
		mono_events.push_back(last_onset);
		midi::Event offset;
		offset.CopyFrom(last_onset);
		offset.set_time(midi_event.time());
		offset.set_velocity(0);
		mono_events.push_back(offset);
		note_sounding = false;
	}

	std::vector<midi::Event> ConvertToMonophonic(std::vector<midi::Event>& events) {
		bool note_sounding = false;
		midi::Event last_onset;
		std::vector<midi::Event> mono_events;
		for (const auto midi_event : events) {
			if (midi_event.velocity() > 0) {
				if ((note_sounding) && (midi_event.time() > last_onset.time())) {
					StopAndPushEvent(midi_event, mono_events, last_onset, note_sounding);
				}
				last_onset.CopyFrom(midi_event);
				note_sounding = true;
			}
			else if ((note_sounding) && (last_onset.pitch() == midi_event.pitch())) {
				mono_events.push_back(last_onset);
				mono_events.push_back(midi_event);
				note_sounding = false;
			}
		}

		return mono_events;
	}


	// TODO: CHECK FUNCTIONS BELOW THIS LINE. 
	// No idea what update valid segments does.

	// ========================================================================
	// EMPTY BARS

	void UpdateHasNotes(midi::Piece* midi_piece) {
		int track_num = 0;
		for (const auto track : midi_piece->tracks()) {
			int bar_num = 0;
			for (const auto bar : track.bars()) {
				bool has_notes = false;
				for (const auto event_index : bar.events()) {
					if (midi_piece->events(event_index).velocity() > 0) {
						has_notes = true;
						break;
					}
				}
				midi_piece->mutable_tracks(track_num)->mutable_bars(bar_num)->set_internal_has_notes(has_notes);
				bar_num++;
			}
			track_num++;
		}
	}

	// ========================================================================
	// RANDOM SEGMENT SELECTION FOR TRAINING
	// 
	// 1. we select an index of a random segment


	void UpdateValidSegments(midi::Piece* midi_piece, int seglen, int min_tracks, bool opz) {
		UpdateHasNotes(midi_piece);
		midi_piece->clear_internal_valid_segments();
		midi_piece->clear_internal_valid_tracks();

		if (midi_piece->tracks_size() < min_tracks) { return; } // no valid tracks

		int min_non_empty_bars = round(seglen * .75);
		int num_bars = GetNumBars(midi_piece);

		for (int start = 0; start < num_bars - seglen + 1; start++) {

			// check that all time sigs are supported
			bool supported_ts = true;
			bool is_four_four = true;
			// for now we ignore this as its better to just keep all the data and
			// prune at training time, as the encoding will fail.


			// check which tracks are valid
			midi::ValidTrack vtracks;
			std::map<int, int> used_track_types;
			for (int track_num = 0; track_num < midi_piece->tracks_size(); track_num++) {
				int non_empty_bars = 0;
				for (int k = 0; k < seglen; k++) {
					if (midi_piece->tracks(track_num).bars(start + k).internal_has_notes()) {
						non_empty_bars++;
					}
				}
				if (non_empty_bars >= min_non_empty_bars) {
					vtracks.add_tracks(track_num);
					if (opz) {
						// product of train types should be different
						int combined_train_type = 1;
						for (const auto train_type : midi_piece->tracks(track_num).internal_train_types()) {
							combined_train_type *= train_type;
						}
						used_track_types[combined_train_type]++;
					}
				}
			}

			// check if there are enough tracks
			bool enough_tracks = vtracks.tracks_size() >= min_tracks;
			if (opz) {
				// for OPZ we can't count repeated track types
				// as we train on only one track per track type
				bool opz_valid = used_track_types.size() >= min_tracks;

				// also valid if we have more than one multi possibility track
				auto it = used_track_types.find(
					midi::OPZ_ARP_TRACK * midi::OPZ_LEAD_TRACK);
				opz_valid |= ((it != used_track_types.end()) && (it->second > 1));

				it = used_track_types.find(
					midi::OPZ_ARP_TRACK * midi::OPZ_LEAD_TRACK * midi::OPZ_CHORD_TRACK);
				opz_valid |= ((it != used_track_types.end()) && (it->second > 1));

				enough_tracks &= opz_valid;
			}

			if (enough_tracks && is_four_four) {
				midi::ValidTrack* v = midi_piece->add_internal_valid_tracks_v2();
				v->CopyFrom(vtracks);
				midi_piece->add_internal_valid_segments(start);
			}
		}
	}

	// ================================================================
	// Non-factorized functions for inference
	// ================================================================

	inline double mmm_log2(const long double x) {
		return std::log(x) * M_LOG2E;
	}

	template <typename T>
	T clip(const T& n, const T& lower, const T& upper) {
		return std::max(lower, std::min(n, upper));
	}

	template<typename T>
	std::vector<T> quantile(std::vector<T>& x, std::vector<double> qs) {
		std::vector<T> vals;
		for (const auto q : qs) {
			if (x.size()) {
				int index = std::min((int)round((double)x.size() * q), (int)x.size() - 1);
				std::nth_element(x.begin(), x.begin() + index, x.end());
				vals.push_back(x[index]);
			}
			else {
				vals.push_back(0);
			}
		}
		return vals;
	}

	template<typename T>
	T min_value(std::vector<T>& x) {
		auto result = std::min_element(x.begin(), x.end());
		if (result == x.end()) {
			return 0;
		}
		return *result;
	}

	template<typename T>
	T max_value(std::vector<T>& x) {
		auto result = std::max_element(x.begin(), x.end());
		if (result == x.end()) {
			return 0;
		}
		return *result;
	}

	template <typename T>
	std::string protobuf_to_string(T* x) {
		std::string output;
		google::protobuf::util::JsonPrintOptions opt;
		opt.add_whitespace = true;
		google::protobuf::util::MessageToJsonString(*x, &output, opt);
		return output;
	}

	std::vector<int> get_note_durations(std::vector<midi::Note>& notes) {
		std::vector<int> durations;
		for (const auto note : notes) {
			double d = note.end() - note.start();
			durations.push_back((int)clip(mmm_log2(std::max(d / 3., 1e-6)) + 1, 0., 5.));
		}
		return durations;
	}

	std::tuple<double, double, double, double, double, double> av_polyphony_inner(std::vector<midi::Note>& notes, int max_tick, midi::TrackFeatures* f) {
		if (max_tick > 100000) {
			throw std::runtime_error("MAX TICK TO LARGE!");
		}
		int nonzero_count = 0;
		double count = 0;
		std::vector<int> flat_roll(max_tick, 0);
		for (const auto note : notes) {
			for (int t = note.start(); t < std::min(note.end(), max_tick - 1); t++) {
				if (flat_roll[t] == 0) {
					nonzero_count += 1;
				}
				flat_roll[t]++;
				count++;
			}
		}

		std::vector<int> nz;
		for (const auto x : flat_roll) {
			if (x > 0) {
				nz.push_back(x);
				if (f) {
					f->add_polyphony_distribution(x);
				}
			}
		}

		double silence = max_tick - nonzero_count;

		std::vector<int> poly_qs = quantile<int>(nz, { .15,.85 });

		double min_polyphony = min_value(nz);
		double max_polyphony = max_value(nz);

		double av_polyphony = count / std::max(nonzero_count, 1);
		double av_silence = silence / std::max(max_tick, 1);
		return std::make_tuple(av_polyphony, av_silence, poly_qs[0], poly_qs[1], min_polyphony, max_polyphony);
	}

	double note_duration_inner(std::vector<midi::Note>& notes) {
		double total_diff = 0;
		for (const auto note : notes) {
			total_diff += (note.end() - note.start());
		}
		return total_diff / std::max((int)notes.size(), 1);
	}

	// function to get note density value
	int get_note_density_target(midi::Track* track, int bin) {
		int qindex = track->instrument();
		int tt = track->track_type();
		if (data_structures::is_opz_track(tt)) {
			std::tuple<int, int> key = std::make_tuple(track->track_type(), qindex);
			if (enums::OPZ_DENSITY_QUANTILES.find(key) != enums::OPZ_DENSITY_QUANTILES.end()) {
				return enums::OPZ_DENSITY_QUANTILES[key][bin];
			}
			return 0;
		}
		if (data_structures::is_drum_track(tt)) {
			qindex = 128;
		}
		return enums::DENSITY_QUANTILES[qindex][bin];
	}

	void update_note_density(midi::Piece* x) {

		int track_num = 0;
		int num_notes, bar_num;
		for (const auto track : x->tracks()) {

			// calculate average notes per bar
			num_notes = 0;
			int bar_num = 0;
			std::set<int> valid_bars;
			for (const auto bar : track.bars()) {
				for (const auto event_index : bar.events()) {
					if (x->events(event_index).velocity()) {
						valid_bars.insert(bar_num);
						num_notes++;
					}
				}
				bar_num++;
			}
			int num_bars = std::max((int)valid_bars.size(), 1);
			double av_notes_fp = (double)num_notes / num_bars;
			int av_notes = round(av_notes_fp);

			// calculate the density bin
			int qindex = track.instrument();
			int bin = 0;

			if (data_structures::is_opz_track(track.track_type())) {
				std::tuple<int, int> key = std::make_tuple(track.track_type(), qindex);
				if (enums::OPZ_DENSITY_QUANTILES.find(key) != enums::OPZ_DENSITY_QUANTILES.end()) {
					while (av_notes > enums::OPZ_DENSITY_QUANTILES[key][bin]) {
						bin++;
					}

				}
			}
			else {
				if (data_structures::is_drum_track(track.track_type())) {
					qindex = 128;
				}
				while (av_notes > enums::DENSITY_QUANTILES[qindex][bin]) {
					bin++;
				}
			}

			// update protobuf
			midi::TrackFeatures* tf = GetTrackFeatures(x, track_num);
			tf->set_note_density_v2(bin);
			tf->set_note_density_value(av_notes_fp);
			track_num++;


		}
	}

	// adding note durations to events
	//encoder_el.h
	void calculate_note_durations(midi::Piece* p) {
		// to start set all durations == 0
		for (int i = 0; i < p->events_size(); i++) {
			p->mutable_events(i)->set_internal_duration(0);
		}

		for (const auto track : p->tracks()) {
			// pitches to (abs_time, event_index)
			std::map<int, std::tuple<int, int>> onsets;
			int bar_start = 0;
			for (const auto bar : track.bars()) {
				for (auto event_id : bar.events()) {
					midi::Event e = p->events(event_id);
					//std::cout << "PROC EVENT :: " << e.pitch() << " " << e.velocity() << " " << e.time() << std::endl;
					if (e.velocity() > 0) {
						if (data_structures::is_drum_track(track.track_type())) {
							// drums always have duration of 1 timestep
							p->mutable_events(event_id)->set_internal_duration(1);
						}
						else {
							onsets[e.pitch()] = std::make_tuple(bar_start + e.time(), event_id);
						}
					}
					else {
						auto it = onsets.find(e.pitch());
						if (it != onsets.end()) {
							int index = std::get<1>(it->second);
							int duration = (bar_start + e.time()) - std::get<0>(it->second);
							p->mutable_events(index)->set_internal_duration(duration);
							//std::cout << "SET DURATION :: " << bar_start << " " << duration << std::endl;
						}
					}
				}
				// move forward a bar
				bar_start += p->resolution() * bar.internal_beat_length();
			}
		}
	}

	//encoder_el.h
	void update_av_polyphony_and_note_duration(midi::Piece* p) {
		for (int track_num = 0; track_num < p->tracks_size(); track_num++) {
			int max_tick = 0;
			std::vector<midi::Note> notes = TrackEventsToNotes(
				p, track_num, &max_tick);
			std::vector<int> durations = get_note_durations(notes);
			midi::TrackFeatures* f = GetTrackFeatures(p, track_num);
			auto stat = av_polyphony_inner(notes, max_tick, f);
			f->set_note_duration(note_duration_inner(notes));
			f->set_av_polyphony(std::get<0>(stat));
			f->set_min_polyphony_q(
				std::max(std::min((int)std::get<2>(stat), 10), 1) - 1);
			f->set_max_polyphony_q(
				std::max(std::min((int)std::get<3>(stat), 10), 1) - 1);

			std::vector<int> dur_qs = quantile(durations, { .15,.85 });
			f->set_min_note_duration_q(dur_qs[0]);
			f->set_max_note_duration_q(dur_qs[1]);

			// new hard upper lower limits
			f->set_min_polyphony_hard(std::get<4>(stat));
			f->set_max_polyphony_hard(std::get<5>(stat));
			f->set_rest_percentage(std::get<1>(stat));

			f->set_min_note_duration_hard(min_value(durations));
			f->set_max_note_duration_hard(max_value(durations));

		}
	}

	// ======================================= jagged.h
	std::tuple<int, int> get_pitch_extents(midi::Piece* x) {
		int min_pitch = INT_MAX;
		int max_pitch = 0;
		for (const auto track : x->tracks()) {
			if (!data_structures::is_drum_track(track.track_type())) {
				for (const auto bar : track.bars()) {
					for (const auto event_index : bar.events()) {
						int pitch = x->events(event_index).pitch();
						min_pitch = std::min(pitch, min_pitch);
						max_pitch = std::max(pitch, max_pitch);
					}
				}
			}
		}
		return std::make_pair(min_pitch, max_pitch);
	}

	void select_random_segment_indices(midi::Piece* x, int num_bars, int min_tracks, int max_tracks, bool opz, std::mt19937* engine, std::vector<int>& valid_tracks, int* start) {
		UpdateValidSegments(x, num_bars, min_tracks, opz);

		if (x->internal_valid_segments_size() == 0) {
			throw std::runtime_error("NO VALID SEGMENTS");
		}

		//int index = rand() % x->internal_valid_segments_size();
		int index = random_on_range(x->internal_valid_segments_size(), engine);
		(*start) = x->internal_valid_segments(index);
		for (const auto track_num : x->internal_valid_tracks_v2(index).tracks()) {
			valid_tracks.push_back(track_num);
		}
		shuffle(valid_tracks.begin(), valid_tracks.end(), *engine);

		if (opz) {
			// filter out duplicate OPZ tracks
			// convert train_track_types to type
			// randomly pick a track type for each track

			std::vector<int> pruned_tracks;
			std::vector<int> used(midi::NUM_TRACK_TYPES, 0);
			for (const auto track_num : valid_tracks) {
				std::vector<midi::TRACK_TYPE> track_options;
				for (const auto track_type : x->tracks(track_num).internal_train_types()) {
					track_options.push_back((midi::TRACK_TYPE)track_type);
				}
				shuffle(track_options.begin(), track_options.end(), *engine);
				for (const auto track_type : track_options) {
					if ((track_type >= 0) && (track_type < midi::NUM_TRACK_TYPES)) {
						if ((used[track_type] == 0) && (track_type <= midi::OPZ_CHORD_TRACK)) {
							pruned_tracks.push_back(track_num);
							// set the track type to the one randomly selected
							x->mutable_tracks(track_num)->set_track_type(track_type);
							used[track_type] = 1;
							break;
						}
					}
				}
			}
			valid_tracks = pruned_tracks;

			// it is possible that we have less than min tracks here
			// throw an exception if this is the case
			if (valid_tracks.size() < min_tracks) {
				throw std::runtime_error("LESS THAN MIN TRACKS");
			}
		}
		else {
			// limit the tracks
			int ntracks = std::min((int)valid_tracks.size(), max_tracks);
			valid_tracks.resize(ntracks);
		}
	}

	void prune_tracks_dev2(midi::Piece* x, std::vector<int> tracks, std::vector<int> bars) {

		if (x->tracks_size() == 0) {
			return;
		}

		midi::Piece tmp(*x);

		int num_bars = GetNumBars(x);
		bool remove_bars = bars.size() > 0;
		x->clear_tracks();
		x->clear_events();

		std::vector<int> tracks_to_keep;
		for (const auto track_num : tracks) {
			if ((track_num >= 0) && (track_num < tmp.tracks_size())) {
				tracks_to_keep.push_back(track_num);
			}
		}

		std::vector<int> bars_to_keep;
		for (const auto bar_num : bars) {
			if ((bar_num >= 0) && (bar_num < num_bars)) {
				bars_to_keep.push_back(bar_num);
			}
		}

		for (const auto track_num : tracks_to_keep) {
			const midi::Track track = tmp.tracks(track_num);
			midi::Track* t = x->add_tracks();
			t->CopyFrom(track);
			if (remove_bars) {
				t->clear_bars();
				for (const auto bar_num : bars_to_keep) {
					const midi::Bar bar = track.bars(bar_num);
					midi::Bar* b = t->add_bars();
					b->CopyFrom(bar);
					b->clear_events();
					for (const auto event_index : bar.events()) {
						b->add_events(x->events_size());
						midi::Event* e = x->add_events();
						e->CopyFrom(tmp.events(event_index));
					}
				}
			}
		}
	}

	void select_random_segment(midi::Piece* x, int num_bars, int min_tracks, int max_tracks, bool opz, std::mt19937* engine) {
		int start;
		std::vector<int> valid_tracks;
		select_random_segment_indices(
			x, num_bars, min_tracks, max_tracks, opz, engine, valid_tracks, &start);
		std::vector<int> bars = arange(start, start + num_bars, 1);
		prune_tracks_dev2(x, valid_tracks, bars);
	}

	std::set<std::tuple<int, int>> make_bar_mask(midi::Piece* x, float proportion, std::mt19937* engine) {
		int num_tracks = x->tracks_size();
		int num_bars = GetNumBars(x);
		int max_filled_bars = (int)round(num_tracks * num_bars * proportion);
		int n_fill = random_on_range(max_filled_bars, engine);
		//int n_fill = rand() % (int)round(num_tracks * num_bars * proportion);
		std::vector<std::tuple<int, int>> choices;
		for (int track_num = 0; track_num < num_tracks; track_num++) {
			for (int bar_num = 0; bar_num < num_bars; bar_num++) {
				choices.push_back(std::make_pair(track_num, bar_num));
			}
		}
		std::set<std::tuple<int, int>> mask;
		shuffle(choices.begin(), choices.end(), *engine);
		for (int i = 0; i < n_fill; i++) {
			mask.insert(choices[i]);
		}
		return mask;
	}

	// ========================================================= control.h

	std::string get_piece_string(midi::Piece* x) {
		std::string output;
		google::protobuf::util::JsonPrintOptions opt;
		opt.add_whitespace = true;
		google::protobuf::util::MessageToJsonString(*x, &output, opt);
		return output;
	}

	void print_piece(midi::Piece* x) {
		std::cout << get_piece_string(x) << std::endl;
	}

	void print_piece_summary(midi::Piece* x) {
		midi::Piece c(*x);
		c.clear_events();
		for (int track_num = 0; track_num < c.tracks_size(); track_num++) {
			c.mutable_tracks(track_num)->clear_bars();
		}
		print_piece(&c);
	}

	void reorder_tracks(midi::Piece* x, std::vector<int> track_order) {
		int num_tracks = x->tracks_size();
		if (num_tracks != track_order.size()) {
			std::cout << num_tracks << " " << track_order.size() << std::endl;
			throw std::runtime_error("Track order does not match midi::Piece.");
		}
		for (int track_num = 0; track_num < num_tracks; track_num++) {
			GetTrackFeatures(x, track_num)->set_order(track_order[track_num]);
		}
		std::sort(
			x->mutable_tracks()->begin(),
			x->mutable_tracks()->end(),
			[](const midi::Track& a, const midi::Track& b) {
				return a.internal_features(0).order() < b.internal_features(0).order();
			}
		);
	}

	// ========================================================= sample_internal.h
	template <typename T>
	void string_to_protobuf(std::string& s, T* x) {
		google::protobuf::util::JsonStringToMessage(s, x);
	}

	// ========================================================= multi_step_sample.h
	template <typename T>
	void print_protobuf(T* x) {
		std::cout << protobuf_to_string(x) << std::endl;
	}

	void print_status(midi::Status* x) {
		std::string output;
		google::protobuf::util::JsonPrintOptions opt;
		opt.add_whitespace = true;
		google::protobuf::util::MessageToJsonString(*x, &output, opt);
		std::cout << output << std::endl;
	}

	void pad_piece_with_status(midi::Piece* p, midi::Status* s, int min_bars) {
		// add tracks when status references ones that do not exist
		for (const auto track : s->tracks()) {
			midi::Track* t = NULL;
			if (track.track_id() >= p->tracks_size()) {
				t = p->add_tracks();
				t->set_track_type(track.track_type());
				std::cout << "adding track " << track.track_id() << std::endl;
				midi::TrackFeatures* f = t->add_internal_features();
				//f->set_note_density_v2( 4 );
			}
			else {
				std::cout << "using track " << track.track_id() << std::endl;
				t = p->mutable_tracks(track.track_id());
			}
			for (int i = t->bars_size(); i < 5; i++) {}
			std::cout << "track " << track.track_id() << " has " << t->bars_size() << " bars" << std::endl;
			int num_bars = std::max(track.selected_bars_size(), min_bars);
			std::cout << "adding " << num_bars << " bars" << std::endl;
			for (int i = t->bars_size(); i < num_bars; i++) {
				std::cout << "adding bar " << i << std::endl;
				midi::Bar* b = t->add_bars();
				std::cout << "check " << i << std::endl;
				b->set_internal_beat_length(4);
			}
			std::cout << "end" << std::endl;
		}
	}


}
// END OF NAMESPACE