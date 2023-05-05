#pragma once

#include <limits>

#include "encoder_base.h"
#include "../data_structures/track_type.h"
#include "../midi_parsing/util_protobuf.h"
#include "representation.h"

// START OF NAMESPACE
namespace encoder {

class TrackEncoder : public ENCODER { //IDGOOD
public:
  TrackEncoder() {    
    rep = new encoder::REPRESENTATION({
      {token::PIECE_START, encoder::TOKEN_DOMAIN(1)},
      {token::BAR, encoder::TOKEN_DOMAIN(1)},
      {token::BAR_END, encoder::TOKEN_DOMAIN(1)},
      {token::TRACK, encoder::TOKEN_DOMAIN({
        midi::STANDARD_TRACK,
        midi::STANDARD_DRUM_TRACK,   
      },encoder::INT_VALUES_DOMAIN)},
      {token::TRACK_END, encoder::TOKEN_DOMAIN(1)},
      {token::INSTRUMENT, encoder::TOKEN_DOMAIN(128)},
      {token::NOTE_OFFSET, encoder::TOKEN_DOMAIN(128)},
      {token::NOTE_ONSET, encoder::TOKEN_DOMAIN(128)},
      {token::TIME_DELTA, encoder::TOKEN_DOMAIN(48)},
      });
    config = get_encoder_config();
  }
  ~TrackEncoder() {
    delete rep;
    delete config;
  }
  data_structures::EncoderConfig *get_encoder_config() {
    data_structures::EncoderConfig *encoder_config = new data_structures::EncoderConfig();
    encoder_config->force_instrument = true;
    encoder_config->min_tracks = 1; // not sure this is used anywhere
    encoder_config->resolution = 12;
    return encoder_config;
  }
};
}
// END OF NAMESPACE






