#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

//#include "midi_io.h"
#include "../common/midi_parsing/midi_io.h"
#include "dataset/jagged.h"
#include "encoder/encoder_el.h"
#include "enum/model_type.h"
#include "sampling/control.h"
#include "../common/encoder/util.h"
#include "version.h"

#include "piano_roll.h"

#ifndef NOTORCH
#include "sampling/sample_internal.h"
#include "sampling/multi_step_sample.h"

namespace mmm {
std::string generate_py(std::string &status_str, std::string &piece_str, std::string &param_str) {
  midi::Piece piece;
  google::protobuf::util::JsonStringToMessage(piece_str.c_str(), &piece);
  midi::Status status;
  google::protobuf::util::JsonStringToMessage(status_str.c_str(), &status);
  midi::SampleParam param;
  google::protobuf::util::JsonStringToMessage(param_str.c_str(), &param);


  sampling::sample(&piece, &status, &param);
  std::string output_str;
  google::protobuf::util::MessageToJsonString(piece, &output_str);
  
  return output_str;
}
}
#else 
namespace mmm {
void generate_py() { }
void sample_multi_step_py() { }
}
#endif

std::tuple<std::string,std::string,std::string> blank(std::string &piece_json, std::string &status_json, std::string &param_json) {
  midi::Piece p;
  midi::Status s;
  midi::SampleParam h;
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

PYBIND11_MODULE(mmm_api,m) {

  py::enum_<enums::MODEL_TYPE>(m, "MODEL_TYPE", py::arithmetic())
    .value("TRACK_MODEL", enums::MODEL_TYPE::TRACK_MODEL)
    .value("BAR_INFILL_MODEL", enums::MODEL_TYPE::BAR_INFILL_MODEL)
    .export_values();

  

  py::class_<encoder::REPRESENTATION>(m, "REPRESENTATION")
    .def("decode", &encoder::REPRESENTATION::decode)
    .def("is_token_type", &encoder::REPRESENTATION::is_token_type)
    .def("in_domain", &encoder::REPRESENTATION::in_domain)
    //.def("encode_timesig", &encoder::REPRESENTATION::encode_timesig)
    //.def("encode_continuous", &encoder::REPRESENTATION::encode_continuous)
    .def("encode", &encoder::REPRESENTATION::encode)
    .def("encode_partial", &encoder::REPRESENTATION::encode_partial_py_int)
    .def("encode_to_one_hot", &encoder::REPRESENTATION::encode_to_one_hot)
    .def("pretty", &encoder::REPRESENTATION::pretty)
    .def_readonly("vocab_size", &encoder::REPRESENTATION::vocab_size)
    .def("get_type_mask", &encoder::REPRESENTATION::get_type_mask)
    .def("max_token", &encoder::REPRESENTATION::max_token)
    .def_readonly("token_domains", &encoder::REPRESENTATION::token_domains);
  
  py::class_<encoder::TOKEN_DOMAIN>(m, "TOKEN_DOMAIN")
    .def(py::init<int>());
  
  py::class_<sampling::REP_NODE>(m, "REP_NODE")
