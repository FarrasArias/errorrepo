#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include "common/encoder/encoder_all.h"


#include "common/midi_parsing/midi_io.h"
#include "./inference/dataset/jagged.h"
//#include "./inference/encoder/encoder_el.h"
#include "./inference/enum/model_type.h"
//#include "./inference/sampling/control.h"
#include "./inference/version.h"
/*#include "dataset/jagged.h"
#include "encoder/encoder_el.h"
#include "enum/model_type.h"
#include "sampling/control.h"
#include "version.h"*/

#include "./inference/piano_roll.h"

#ifndef NOTORCH
//#include "./inference/sampling/sample_internal.h"
//#include "./inference/sampling/multi_step_sample.h"

// DATASET CREATION LIBRARIES (check if they are needed)
// ======================
#include <iostream>
#include <string>
#include "../include/dataset_creation/dataset_manipulation/bytes_to_file.h"
#include "../libraries/protobuf/include/proto_library.h"
#include "../libraries/torch/include/torch_library.h"
#include "../libraries/protobuf/build/midi.pb.h"
#include "MidiFile.h"
//#include <pybind11/pybind11.h>
//#include <pybind11/stl.h>
#include "./common/data_structures/train_config.h"
//#include "./dataset_creation/encoder/encoder_v2.h"

// ======================

namespace mmm { // you can probably remove this namespace
std::string generate_py(std::string &status_str, std::string &piece_str, std::string &param_str) {
  midi::Piece piece;
  google::protobuf::util::JsonStringToMessage(piece_str.c_str(), &piece);
  midi::Status status;
  google::protobuf::util::JsonStringToMessage(status_str.c_str(), &status);
  midi::HyperParam param;
  google::protobuf::util::JsonStringToMessage(param_str.c_str(), &param);

  //sampling::sample(&piece, &status, &param);
  std::string output_str;
  google::protobuf::util::MessageToJsonString(piece, &output_str);
  return output_str;
}
}
#else 
namespace mmm { // you can probably remove this namespace
void generate_py() { }
//void sample_multi_step_py() { }
}

/*namespace sampling {
    void sample_multi_step_py() {}
}*/
#endif

std::tuple<std::string,std::string,std::string> blank(std::string &piece_json, std::string &status_json, std::string &param_json) {
  midi::Piece p;
  midi::Status s;
  midi::HyperParam h;
  google::protobuf::util::JsonStringToMessage(piece_json.c_str(), &p);
  google::protobuf::util::JsonStringToMessage(status_json.c_str(), &s);
  google::protobuf::util::JsonStringToMessage(param_json.c_str(), &h);
  std::string po;
  std::string so;
  std::string ho;
  google::protobuf::util::MessageToJsonString(p, &po);
  google::protobuf::util::MessageToJsonString(s, &so);
  google::protobuf::util::MessageToJsonString(h, &ho);
  return std::make_tuple(po, so, ho);
}

PYBIND11_MODULE(mmm_refactored,handle) {

  /*handle.def("get_genres", &mmm::get_genres);*/

  // functions from piano roll.h
  //handle.def("fast_bit_roll64", &mmm::fast_bit_roll64);
  //handle.def("bit_to_bool", &mmm::bit_to_bool);
  //handle.def("bool_to_bit", &mmm::bool_to_bit);

  handle.def("blank", &blank);

  //handle.def("random_perturb", &protobuf::random_perturb_py);
  //handle.def("append_piece", &protobuf::append_piece_py);
  //handle.def("update_note_density", &protobuf::update_note_density_py);
  //handle.def("update_valid_segments", &protobuf::update_valid_segments_py);
  //handle.def("select_random_segment", &protobuf::select_random_segment_py);
  //handle.def("reorder_tracks", &protobuf::reorder_tracks_py);
  //handle.def("prune_tracks", &protobuf::prune_tracks_py);
  //handle.def("prune_empty_tracks", &protobuf::prune_empty_tracks_py);
  //handle.def("prune_notes_wo_offset", &protobuf::prune_notes_wo_offset_py);

  handle.def("version", &version);
  handle.def("getEncoderSize", &enums::getEncoderSize);
  handle.def("getEncoderType", &enums::getEncoderType);
  handle.def("getEncoder", &enums::getEncoder);

  //handle.def("gm_inst_to_string", &protobuf::gm_inst_to_string);

  //handle.def("generate", &mmm::generate_py);

  //handle.def("sample_multi_step", &sampling::sample_multi_step_py); //FApproved
  //handle.def("piece_to_status", &protobuf::piece_to_status_py);
  //handle.def("default_sample_param", &protobuf::default_sample_param_py);
  //handle.def("print_piece_summary", &protobuf::print_piece_summary_py);
  //handle.def("flatten_velocity", &protobuf::flatten_velocity_py);
  //handle.def("update_av_polyphony_and_note_duration", &protobuf::update_av_polyphony_and_note_duration_py);

  //handle.def("piece_to_onset_distribution", &protobuf::piece_to_onset_distribution_py);

  py::enum_<enums::MODEL_TYPE>(handle, "MODEL_TYPE", py::arithmetic())
    .value("TRACK_MODEL", enums::MODEL_TYPE::TRACK_MODEL)
    .value("BAR_INFILL_MODEL", enums::MODEL_TYPE::BAR_INFILL_MODEL)
    .export_values();

  py::class_<compression::Jagged>(handle, "Jagged")
    .def(py::init<std::string &>())
    .def("set_seed", &compression::Jagged::set_seed)
    .def("set_num_bars", &compression::Jagged::set_num_bars)
    .def("set_min_tracks", &compression::Jagged::set_min_tracks)
    .def("set_max_tracks", &compression::Jagged::set_max_tracks)
    .def("set_max_seq_len", &compression::Jagged::set_max_seq_len)
    .def("enable_write", &compression::Jagged::enable_write)
    .def("enable_read", &compression::Jagged::enable_read)
    .def("append", &compression::Jagged::append)
    .def("read", &compression::Jagged::read)
    .def("read_bytes", &compression::Jagged::read_bytes)
    .def("read_json", &compression::Jagged::read_json)
    .def("read_batch", &compression::Jagged::read_batch)
    .def("read_batch_v2", &compression::Jagged::read_batch_v2)
    .def("read_batch_w_feature", &compression::Jagged::read_batch_w_feature)
    .def("load_piece", &compression::Jagged::load_piece)
    .def("load_piece_pair", &compression::Jagged::load_piece_pair)
    .def("load_piece_pair_batch", &compression::Jagged::load_piece_pair_batch)
    .def("close", &compression::Jagged::close)
    .def("get_size", &compression::Jagged::get_size)
    .def("get_split_size", &compression::Jagged::get_split_size);

  py::class_<data_structures::TrainConfig>(handle, "TrainConfig")
    .def(py::init<>())
    .def_readwrite("num_bars", &data_structures::TrainConfig::num_bars)
    .def_readwrite("min_tracks", &data_structures::TrainConfig::min_tracks)
    .def_readwrite("max_tracks", &data_structures::TrainConfig::max_tracks)
    .def_readwrite("max_mask_percentage", &data_structures::TrainConfig::max_mask_percentage)
    .def_readwrite("opz", &data_structures::TrainConfig::opz)
    .def_readwrite("no_max_length", &data_structures::TrainConfig::no_max_length)
    .def("to_json", &data_structures::TrainConfig::ToJson)
    .def("from_json", &data_structures::TrainConfig::FromJson);

  py::class_<encoder::REPRESENTATION>(handle, "REPRESENTATION")
    .def(py::init<std::vector<std::pair<token::TOKEN_TYPE,encoder::TOKEN_DOMAIN>>>())
    .def("decode", &encoder::REPRESENTATION::decode)
    .def("is_token_type", &encoder::REPRESENTATION::is_token_type)
    .def("in_domain", &encoder::REPRESENTATION::in_domain)
    .def("encode", &encoder::REPRESENTATION::encode)
    .def("encode_partial", &encoder::REPRESENTATION::encode_partial_py_int)
    .def("encode_to_one_hot", &encoder::REPRESENTATION::encode_to_one_hot)
    .def("pretty", &encoder::REPRESENTATION::pretty)
    .def_readonly("vocab_size", &encoder::REPRESENTATION::vocab_size)
    .def("get_type_mask", &encoder::REPRESENTATION::get_type_mask)
    .def("max_token", &encoder::REPRESENTATION::max_token)
    .def_readonly("token_domains", &encoder::REPRESENTATION::token_domains);
  
  py::class_<encoder::TOKEN_DOMAIN>(handle, "TOKEN_DOMAIN")
    .def(py::init<int>());
  
  /*py::class_<sampling::REP_NODE>(handle, "REP_NODE")
    .def(py::init<token::TOKEN_TYPE>());*/

py::class_<data_structures::EncoderConfig>(handle, "EncoderConfig")
  .def(py::init<>())
  .def_readwrite("both_in_one", &data_structures::EncoderConfig::both_in_one)
  .def_readwrite("unquantized", &data_structures::EncoderConfig::unquantized)
  .def_readwrite("interleaved", &data_structures::EncoderConfig::interleaved)
  .def_readwrite("multi_segment", &data_structures::EncoderConfig::multi_segment)
  .def_readwrite("te", &data_structures::EncoderConfig::te)
  .def_readwrite("do_fill", &data_structures::EncoderConfig::do_fill)
  .def_readwrite("do_multi_fill", &data_structures::EncoderConfig::do_multi_fill)
  .def_readwrite("do_track_shuffle", &data_structures::EncoderConfig::do_track_shuffle)
  .def_readwrite("do_pretrain_map", &data_structures::EncoderConfig::do_pretrain_map)
  .def_readwrite("force_instrument", &data_structures::EncoderConfig::force_instrument)
  .def_readwrite("mark_note_duration", &data_structures::EncoderConfig::mark_note_duration)
  .def_readwrite("mark_polyphony", &data_structures::EncoderConfig::mark_polyphony)
  .def_readwrite("density_continuous", &data_structures::EncoderConfig::density_continuous)
  .def_readwrite("mark_genre", &data_structures::EncoderConfig::mark_genre)
  .def_readwrite("mark_density", &data_structures::EncoderConfig::mark_density)
  .def_readwrite("mark_instrument", &data_structures::EncoderConfig::mark_instrument)
  .def_readwrite("mark_polyphony_quantile", &data_structures::EncoderConfig::mark_polyphony_quantile)
  .def_readwrite("mark_note_duration_quantile", &data_structures::EncoderConfig::mark_note_duration_quantile)
  .def_readwrite("mark_time_sigs", &data_structures::EncoderConfig::mark_time_sigs)
  .def_readwrite("allow_beatlength_matches", &data_structures::EncoderConfig::allow_beatlength_matches)
  .def_readwrite("instrument_header", &data_structures::EncoderConfig::instrument_header)
  .def_readwrite("use_velocity_levels", &data_structures::EncoderConfig::use_velocity_levels)
  .def_readwrite("genre_header", &data_structures::EncoderConfig::genre_header)
  .def_readwrite("piece_header", &data_structures::EncoderConfig::piece_header)
  .def_readwrite("bar_major", &data_structures::EncoderConfig::bar_major)
  .def_readwrite("force_four_four", &data_structures::EncoderConfig::force_four_four)
  .def_readwrite("segment_mode", &data_structures::EncoderConfig::segment_mode)
  .def_readwrite("force_valid", &data_structures::EncoderConfig::force_valid)
  .def_readwrite("use_drum_offsets", &data_structures::EncoderConfig::use_drum_offsets)
  .def_readwrite("use_note_duration_encoding", &data_structures::EncoderConfig::use_note_duration_encoding)
  .def_readwrite("use_absolute_time_encoding", &data_structures::EncoderConfig::use_absolute_time_encoding)
  .def_readwrite("mark_num_bars", &data_structures::EncoderConfig::mark_num_bars)
  .def_readwrite("mark_drum_density", &data_structures::EncoderConfig::mark_drum_density)
  .def_readwrite("mark_pitch_limits", &data_structures::EncoderConfig::mark_pitch_limits)
  .def_readwrite("embed_dim", &data_structures::EncoderConfig::embed_dim)
  .def_readwrite("transpose", &data_structures::EncoderConfig::transpose)
  .def_readwrite("seed", &data_structures::EncoderConfig::seed)
  .def_readwrite("segment_idx", &data_structures::EncoderConfig::segment_idx)
  .def_readwrite("fill_track", &data_structures::EncoderConfig::fill_track)
  .def_readwrite("fill_bar", &data_structures::EncoderConfig::fill_bar)
  .def_readwrite("max_tracks", &data_structures::EncoderConfig::max_tracks)
  .def_readwrite("resolution", &data_structures::EncoderConfig::resolution)
  .def_readwrite("default_tempo", &data_structures::EncoderConfig::default_tempo)
  .def_readwrite("num_bars", &data_structures::EncoderConfig::num_bars)
  .def_readwrite("min_tracks", &data_structures::EncoderConfig::min_tracks)
  .def_readwrite("fill_percentage", &data_structures::EncoderConfig::fill_percentage)
  .def_readwrite("multi_fill", &data_structures::EncoderConfig::multi_fill)
  .def_readwrite("genre_tags", &data_structures::EncoderConfig::genre_tags);

py::enum_<token::TOKEN_TYPE>(handle, "TOKEN_TYPE", py::arithmetic())
  .value("PIECE_START", token::TOKEN_TYPE::PIECE_START)
  .value("NOTE_ONSET", token::TOKEN_TYPE::NOTE_ONSET)
  .value("NOTE_OFFSET", token::TOKEN_TYPE::NOTE_OFFSET)
  .value("PITCH", token::TOKEN_TYPE::PITCH)
  .value("NON_PITCH", token::TOKEN_TYPE::NON_PITCH)
  .value("VELOCITY", token::TOKEN_TYPE::VELOCITY)
  .value("TIME_DELTA", token::TOKEN_TYPE::TIME_DELTA)
  .value("TIME_ABSOLUTE_POS", token::TOKEN_TYPE::TIME_ABSOLUTE_POS)
  .value("INSTRUMENT", token::TOKEN_TYPE::INSTRUMENT)
  .value("BAR", token::TOKEN_TYPE::BAR)
  .value("BAR_END", token::TOKEN_TYPE::BAR_END)
  .value("TRACK", token::TOKEN_TYPE::TRACK)
  .value("TRACK_END", token::TOKEN_TYPE::TRACK_END)
  .value("DRUM_TRACK", token::TOKEN_TYPE::DRUM_TRACK)
  .value("FILL_IN", token::TOKEN_TYPE::FILL_IN)
  .value("FILL_IN_PLACEHOLDER", token::TOKEN_TYPE::FILL_IN_PLACEHOLDER)
  .value("FILL_IN_START", token::TOKEN_TYPE::FILL_IN_START)
  .value("FILL_IN_END", token::TOKEN_TYPE::FILL_IN_END)
  .value("HEADER", token::TOKEN_TYPE::HEADER)
  .value("VELOCITY_LEVEL", token::TOKEN_TYPE::VELOCITY_LEVEL)
  .value("GENRE", token::TOKEN_TYPE::GENRE)
  .value("DENSITY_LEVEL", token::TOKEN_TYPE::DENSITY_LEVEL)
  .value("TIME_SIGNATURE", token::TOKEN_TYPE::TIME_SIGNATURE)
  .value("SEGMENT", token::TOKEN_TYPE::SEGMENT)
  .value("SEGMENT_END", token::TOKEN_TYPE::SEGMENT_END)
  .value("SEGMENT_FILL_IN", token::TOKEN_TYPE::SEGMENT_FILL_IN)
  .value("NOTE_DURATION", token::TOKEN_TYPE::NOTE_DURATION)
  .value("AV_POLYPHONY", token::TOKEN_TYPE::AV_POLYPHONY)
  .value("MIN_POLYPHONY", token::TOKEN_TYPE::MIN_POLYPHONY)
  .value("MAX_POLYPHONY", token::TOKEN_TYPE::MAX_POLYPHONY)
  .value("MIN_NOTE_DURATION", token::TOKEN_TYPE::MIN_NOTE_DURATION)
  .value("MAX_NOTE_DURATION", token::TOKEN_TYPE::MAX_NOTE_DURATION)
  .value("NUM_BARS", token::TOKEN_TYPE::NUM_BARS)
  .value("MIN_POLYPHONY_HARD", token::TOKEN_TYPE::MIN_POLYPHONY_HARD)
  .value("MAX_POLYPHONY_HARD", token::TOKEN_TYPE::MAX_POLYPHONY_HARD)
  .value("MIN_NOTE_DURATION_HARD", token::TOKEN_TYPE::MIN_NOTE_DURATION_HARD)
  .value("MAX_NOTE_DURATION_HARD", token::TOKEN_TYPE::MAX_NOTE_DURATION_HARD)
  .value("REST_PERCENTAGE", token::TOKEN_TYPE::REST_PERCENTAGE)
  .value("PITCH_CLASS", token::TOKEN_TYPE::PITCH_CLASS)
  .value("NONE", token::TOKEN_TYPE::NONE)
  .export_values();

py::enum_<enums::ENCODER_TYPE>(handle, "ENCODER_TYPE", py::arithmetic())
  .value("EL_VELOCITY_DURATION_POLYPHONY_YELLOW_ENCODER", enums::ENCODER_TYPE::EL_VELOCITY_DURATION_POLYPHONY_YELLOW_ENCODER)
  .value("NO_ENCODER", enums::ENCODER_TYPE::NO_ENCODER)
  .export_values();

 

// =========================================================
// =========================================================
// ENCODERS
// =========================================================
// =========================================================

py::class_<encoder::ElVelocityDurationPolyphonyYellowEncoder>(handle, "ElVelocityDurationPolyphonyYellowEncoder")
  .def(py::init<>())
  .def("encode", &encoder::ElVelocityDurationPolyphonyYellowEncoder::encode)
  .def("decode", &encoder::ElVelocityDurationPolyphonyYellowEncoder::decode)
  .def("midi_to_json", &encoder::ElVelocityDurationPolyphonyYellowEncoder::midi_to_json)
  .def("midi_to_json_bytes", &encoder::ElVelocityDurationPolyphonyYellowEncoder::midi_to_json_bytes)
  .def("midi_to_tokens", &encoder::ElVelocityDurationPolyphonyYellowEncoder::midi_to_tokens)
  .def("json_to_midi", &encoder::ElVelocityDurationPolyphonyYellowEncoder::json_to_midi)
  .def("json_track_to_midi", &encoder::ElVelocityDurationPolyphonyYellowEncoder::json_track_to_midi)
  .def("json_to_tokens", &encoder::ElVelocityDurationPolyphonyYellowEncoder::json_to_tokens)
  .def("tokens_to_json", &encoder::ElVelocityDurationPolyphonyYellowEncoder::tokens_to_json)
  .def("tokens_to_midi", &encoder::ElVelocityDurationPolyphonyYellowEncoder::tokens_to_midi)
  .def_readwrite("config", &encoder::ElVelocityDurationPolyphonyYellowEncoder::config)
  .def_readwrite("rep", &encoder::ElVelocityDurationPolyphonyYellowEncoder::rep);

//
// =========================================================
// =========================================================
// DATASET CREATION
// =========================================================
// =========================================================

/*py::class_<data_structures::TrainConfig>(handle, "TrainConfigDC")
.def(py::init<>())
.def_readwrite("num_bars", &data_structures::TrainConfig::num_bars)
.def_readwrite("min_tracks", &data_structures::TrainConfig::min_tracks)
.def_readwrite("max_tracks", &data_structures::TrainConfig::max_tracks)
.def_readwrite("max_mask_percentage", &data_structures::TrainConfig::max_mask_percentage)
.def_readwrite("opz", &data_structures::TrainConfig::opz)
.def_readwrite("no_max_length", &data_structures::TrainConfig::no_max_length)
.def("to_json", &data_structures::TrainConfig::ToJson)
.def("from_json", &data_structures::TrainConfig::FromJson);*/

//dataset_manipulation folder definitions
py::class_<dataset_manipulation::BytesToFile>(handle, "BytesToFile")
.def(py::init<std::string&>())
.def("append_bytes_to_file_stream", &dataset_manipulation::BytesToFile::appendBytesToFileStream)
.def("write_file", &dataset_manipulation::BytesToFile::writeFile)
.def("close", &dataset_manipulation::BytesToFile::close);

py::class_<encoder::TrackEncoder>(handle, "TrackEncoder")
.def(py::init<>())
.def("midi_to_json_bytes", &encoder::TrackEncoder::midi_to_json_bytes)
.def_readwrite("config", &encoder::TrackEncoder::config)
.def_readwrite("rep", &encoder::TrackEncoder::rep);

}