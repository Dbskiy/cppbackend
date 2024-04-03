#include "json_loader.h"
#include <boost/json.hpp>

#include <iostream>
#include <fstream>
#include <string>

namespace json = boost::json;
using namespace std::literals;

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    std::ifstream f_map(json_path);

    if (!f_map.is_open()) {
        throw std::runtime_error("Failed to open JSON file");
    }

    std::string json_str;
    std::string str;
    while (std::getline(f_map, str)) {

        json_str += str;

        //std::cout << str << std::endl;
    }

    auto json_maps = json::parse(json_str);

    model::Game game;

    int maps_size = json_maps.as_object().at("maps").as_array().size();

    for (int i = 0; i < maps_size; i++) {
        model::Map::Id id(json_maps.as_object().at("maps").as_array().at(i).at("id").as_string().c_str());
        std::string name = json_maps.as_object().at("maps").as_array().at(i).at("name").as_string().c_str();
        
        model::Map map(id, name);
        

        game.AddMap(map);
    }

    return game;
}   

}  // namespace json_loader
