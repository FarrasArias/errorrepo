#pragma once

#include "encoder_base.h"
#include "util.h"
#include "../data_structures/track_type.h"
#include "../../inference/enum/velocity.h"
#include "../../inference/enum/timesigs.h"
#include "../../inference/enum/pretrain_group.h"
#include "../midi_parsing/util_protobuf.h"
#include "../../inference/protobuf/validate.h"

// START OF NAMESPACE
namespace encoder {
class ElVelocityDurationPolyphonyYellowEncoder : public ENCODER {
public:
  ElVelocityDurationPolyphonyYellowEncoder() {
    config = get_encoder_config();
    rep = get_encoder_rep();
  }
  ~ElVelocityDurationPolyphonyYellowEncoder() {
    delete rep;
    delete config;
  }

  REPRESENTATION *get_encoder_rep() {
    REPRESENTATION *r = new REPRESENTATION({
      {token::PIECE_START, TOKEN_DOMAIN(2)},
      {token::NUM_BARS, TOKEN_DOMAIN({4,8}, INT_VALUES_DOMAIN)},
      {token::BAR, TOKEN_DOMAIN(1)},
      {token::BAR_END, TOKEN_DOMAIN(1)},
      {token::TIME_SIGNATURE, TOKEN_DOMAIN(
        enums::YELLOW_TS_MAP,TIMESIG_MAP_DOMAIN)},
      {token::TRACK, TOKEN_DOMAIN({
        midi::STANDARD_TRACK,
        midi::STANDARD_DRUM_TRACK  
      },INT_VALUES_DOMAIN)},
      {token::TRACK_END, TOKEN_DOMAIN(1)},
      {token::INSTRUMENT, TOKEN_DOMAIN(enums::PRETRAIN_GROUPING,INT_MAP_DOMAIN)},
      {token::NOTE_ONSET, TOKEN_DOMAIN(128)},
      {token::NOTE_DURATION, TOKEN_DOMAIN(96)},
      {token::TIME_ABSOLUTE_POS, TOKEN_DOMAIN(192)},
      {token::FILL_IN_PLACEHOLDER, TOKEN_DOMAIN(1)},
      {token::FILL_IN_START, TOKEN_DOMAIN(1)},
      {token::FILL_IN_END, TOKEN_DOMAIN(1)},
      {token::MIN_NOTE_DURATION, TOKEN_DOMAIN(6)},
      {token::MAX_NOTE_DURATION, TOKEN_DOMAIN(6)},
      {token::MIN_POLYPHONY, TOKEN_DOMAIN(10)},
      {token::MAX_POLYPHONY, TOKEN_DOMAIN(10)},
      {token::DENSITY_LEVEL, TOKEN_DOMAIN(10)},
      {token::VELOCITY_LEVEL, TOKEN_DOMAIN(enums::DEFAULT_VELOCITY_MAP,INT_MAP_DOMAIN)}
    });
    return r;
  }

  data_structures::EncoderConfig *get_encoder_config() {
    data_structures::EncoderConfig *e = new data_structures::EncoderConfig();
    e->both_in_one = true;
    e->force_instrument = true;
    e->mark_note_duration_quantile = true;
    e->mark_polyphony_quantile = true;
    e->use_note_duration_encoding = true;
    e->use_absolute_time_encoding = true;
    e->mark_time_sigs = true;
    e->mark_drum_density = true;
    e->use_drum_offsets = false;
    e->use_velocity_levels = true;
    e->min_tracks = 1; // not sure this is used anywhere
    e->resolution = 12;
    return e;
  }

  void preprocess_piece(midi::Piece *p) {
    util_protobuf::calculate_note_durations(p);
    util_protobuf::update_av_polyphony_and_note_duration(p);
    util_protobuf::update_note_density(p);
  }
};

}
// END OF NAMESPACE