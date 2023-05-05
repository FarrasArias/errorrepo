#pragma once

#include <map>
#include <string>

// START OF NAMESPACE
namespace data_structures {

class TrainConfig {
public:
    int num_bars;
    int min_tracks;
    int max_tracks;
    float max_mask_percentage;
    bool opz;
    bool no_max_length;

    TrainConfig();

    std::map<std::string, std::string> ToJson();
    void FromJson(std::map<std::string, std::string>& json_config);
};

//TrainConfig::TrainConfig() {
//    num_bars = 4;
//    min_tracks = 1;
//    max_tracks = 12;
//    max_mask_percentage = 0.75;
//    opz = false;
//    no_max_length = false;
//}
//
//std::map<std::string, std::string> TrainConfig::ToJson() {
//    std::map<std::string, std::string> json_config;
//    json_config["num_bars"] = std::to_string(num_bars);
//    json_config["min_tracks"] = std::to_string(min_tracks);
//    json_config["max_tracks"] = std::to_string(max_tracks);
//    json_config["max_mask_percentage"] = std::to_string(max_mask_percentage);
//    json_config["opz"] = std::to_string((int)opz);
//    json_config["no_max_length"] = std::to_string((int)no_max_length);
//    return json_config;
//}
//
//void TrainConfig::FromJson(std::map<std::string, std::string> &json_config) {
//    num_bars = stoi(json_config["num_bars"]);
//    min_tracks = stoi(json_config["min_tracks"]);
//    max_tracks = stoi(json_config["max_tracks"]);
//    max_mask_percentage = stof(json_config["max_mask_percentage"]);
//    opz = (bool)stoi(json_config["opz"]);
//    no_max_length = (bool)stoi(json_config["no_max_length"]);
//}

//class TrainConfig { //IDGOOD
//public:
//  TrainConfig() {
//    num_bars = 4;
//    min_tracks = 1;
//    max_tracks = 12;
//    max_mask_percentage = 0.75;
//    opz = false;
//    no_max_length = false;
//  }
//  std::map<std::string,std::string> ToJson() {
//    std::map<std::string,std::string> json_config;
//    json_config["num_bars"] = std::to_string(num_bars);
//    json_config["min_tracks"] = std::to_string(min_tracks);
//    json_config["max_tracks"] = std::to_string(max_tracks);
//    json_config["max_mask_percentage"] = std::to_string(max_mask_percentage);
//    json_config["opz"] = std::to_string((int)opz);
//    json_config["no_max_length"] = std::to_string((int)no_max_length);
//    return json_config;
//  }
//  void FromJson(std::map<std::string,std::string> &json_config) {
//    num_bars = stoi(json_config["num_bars"]);
//    min_tracks = stoi(json_config["min_tracks"]);
//    max_tracks = stoi(json_config["max_tracks"]);
//    max_mask_percentage = stof(json_config["max_mask_percentage"]);
//    opz = (bool)stoi(json_config["opz"]);
//    no_max_length = (bool)stoi(json_config["no_max_length"]);
//  }
//  int num_bars;
//  int min_tracks;
//  int max_tracks;
//  float max_mask_percentage;
//  bool opz;
//  bool no_max_length;
//};
//
}
// END OF NAMESPACE