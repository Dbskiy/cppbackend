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

        auto json_data = json::parse(json_str);

        model::Game game;



        const int maps_size = json_data.as_object().at("maps").as_array().size();

        for (int i = 0; i < maps_size; i++) {
            //добавление id и имени в карту
            model::Map::Id map_id(json_data.as_object().at("maps").as_array().at(i).at("id").as_string().c_str());
            std::string name = json_data.as_object().at("maps").as_array().at(i).at("name").as_string().c_str();

            model::Map map(map_id, name);

            //новый код
            //добавление дороги в карту
            const json::array& roads_array = json_data.as_object().at("maps").as_array().at(i).at("roads").as_array();
            for (const auto& road : roads_array) {
                int x0 = road.as_object().at("x0").as_int64();
                int y0 = road.as_object().at("y0").as_int64();

                if (road.as_object().contains("x1")) {
                    int x1 = road.as_object().at("x1").as_int64();
                    map.AddRoad(model::Road(model::Road::HORIZONTAL, { x0, y0 }, x1));
                }
                else {
                    int y1 = road.as_object().at("y1").as_int64();
                    map.AddRoad(model::Road(model::Road::VERTICAL, { x0, y0 }, y1));
                }
            }


            const json::array& buildings = json_data.as_object().at("maps").as_array().at(i).at("buildings").as_array();
            if (!buildings.empty()) {
                for (const auto& building : buildings) {
                    int x = building.as_object().at("x").as_int64();
                    int y = building.as_object().at("y").as_int64();

                    int w = building.as_object().at("w").as_int64();
                    int h = building.as_object().at("h").as_int64();

                    map.AddBuilding(model::Building({ {x, y}, {w, h} }));
                }
            }

            const json::array& offices = json_data.as_object().at("maps").as_array().at(i).at("offices").as_array();
            if (!offices.empty()) {
                for (const auto& office : offices) {
                    model::Office::Id office_id(office.as_object().at("id").as_string().c_str());
                    int x = office.as_object().at("x").as_int64();
                    int y = office.as_object().at("y").as_int64();

                    int offsetX = office.as_object().at("offsetX").as_int64();
                    int offsetY = office.as_object().at("offsetY").as_int64();

                    map.AddOffice(model::Office(office_id, { x, y }, { offsetX, offsetY }));
                }
            }

            game.AddMap(map);

        }
        return game;
    }
}  // namespace json_loader
