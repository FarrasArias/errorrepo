#pragma once

#include "../../common/encoder/encoder_all.h"
#include <string>

// START OF NAMESPACE
namespace enums {

enum ENCODER_TYPE {
  EL_VELOCITY_DURATION_POLYPHONY_YELLOW_ENCODER,
  NO_ENCODER
};

std::unique_ptr<encoder::ENCODER> getEncoder(ENCODER_TYPE et) {
	encoder::ENCODER *e;
  switch (et) {
    case EL_VELOCITY_DURATION_POLYPHONY_YELLOW_ENCODER:
			e = new encoder::ElVelocityDurationPolyphonyYellowEncoder();
			return std::unique_ptr<encoder::ENCODER>(e);
			break;
    case NO_ENCODER: return NULL;
	default: return NULL;
  }
}

ENCODER_TYPE getEncoderType(const std::string &s) {
  if (s == "EL_VELOCITY_DURATION_POLYPHONY_YELLOW_ENCODER") return EL_VELOCITY_DURATION_POLYPHONY_YELLOW_ENCODER;
  return NO_ENCODER;
}

int getEncoderSize(ENCODER_TYPE et) {
  std::unique_ptr<encoder::ENCODER> encoder = getEncoder(et);
  if (!encoder) {
    return 0;
  }
  int size = encoder->rep->max_token();
  return size;
}
}
// END OF NAMESPACE
