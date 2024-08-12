#include "json_loader.h"
#include <iostream>
#include <fstream>
#include <string>

namespace json_loader {

    model::Game LoadGame(const std::filesystem::path& json_path) {
        using namespace loot_extra_data;

        std::ifstream f_map(json_path);

        if (!f_map.is_open()) {
            throw std::runtime_error("Failed to open JSON file");
        }

        std::string json_str;
        std::string str;
        while (std::getline(f_map, str)) {
            json_str += str;
        }

        auto json_data = json::parse(json_str);

        model::Game game;
        const auto& json_object = json_data.as_object();

        const auto default_speed = DefaultDogSpeed(json_object);
        
        model::Map::BagCapacity def_bag_capacity = DefaultBagCapacity(json_object);

        LootConfiguration(game, json_object);

        DogRetirementTime(game, json_object);

        const json::array& all_maps = json_object.at("maps").as_array();

        for (const auto& map_json : all_maps) {
            model::Map::Id map_id(map_json.at("id").as_string().c_str());
            std::string name = map_json.at("name").as_string().c_str();
            
            const auto dog_speed = DogSpeedOnTheMap(map_json.as_object(), default_speed);

            const auto bag_capacity = BagCapacityOnTheMap(map_json.as_object(), def_bag_capacity);

            json::array loot_types = map_json.at("lootTypes").as_array();

            model::Map::LootTypesNum loot_types_num(loot_types.size());

            model::Map::LootTypesProp loot_types_prop = LootTypesProperties(std::move(loot_types));

            model::Map map(map_id, name, dog_speed, bag_capacity, loot_types_num, loot_types_prop);

            InsertRoads(map, map_json);

            InsertBuildings(map, map_json);
            
            InsertOffices(map, map_json);
            
            game.AddMap(map);

        }
        return game;
    }

    model::Map::DogSpeed DefaultDogSpeed(const json::object& json_object)
    {
        if (json_object.contains("defaultDogSpeed")) {
            return model::Map::DogSpeed(json_object.at("defaultDogSpeed").as_double());
        }
        return model::Map::DogSpeed(1);
    }

    model::Map::BagCapacity DefaultBagCapacity(const json::object& json_object)
    {
        if (json_object.contains("defaultBagCapacity")) {
            return model::Map::BagCapacity(json_object.at("defaultBagCapacity").as_int64());
        }
        return model::Map::BagCapacity(3);
    }

    void LootConfiguration(model::Game& game, const json::object& json_object)
    {
        if (json_object.contains("lootGeneratorConfig")) {
            double period = json_object.at("lootGeneratorConfig").as_object().at("period").as_double();
            double probability = json_object.at("lootGeneratorConfig").as_object().at("probability").as_double();

            game.SetLootGeneratorConfig(period, probability);
        }
    }

    void DogRetirementTime(model::Game& game, const json::object& json_object)
    {
        if (json_object.contains("dogRetirementTime")) {
            double retirement_time = json_object.at("dogRetirementTime").as_double();

            game.SetDogRetirementTime(retirement_time);
        }
    }

    model::Map::LootTypesProp LootTypesProperties(json::array loot_types)
    {
        loot_extra_data::LootExtraData extra_data(std::move(loot_types));

        return extra_data.GetLootTypesProp();
    }

    model::Map::DogSpeed DogSpeedOnTheMap(const json::object& map_json, const auto def_speed)
    {
        if (map_json.contains("dogSpeed")) {
            return model::Map::DogSpeed(map_json.at("dogSpeed").as_double());
        }
        return def_speed;
    }

    model::Map::BagCapacity BagCapacityOnTheMap(const json::object& map_json, model::Map::BagCapacity def_bag_capacity)
    {
        if (map_json.contains("bagCapacity")) {
            return model::Map::BagCapacity(map_json.at("bagCapacity").as_uint64());
        }
        return def_bag_capacity;
    }

    void InsertRoads(model::Map& map, const auto& map_json) {
        const json::array& roads_array = map_json.at("roads").as_array();
        for (const auto& road : roads_array) {
            int x0 = road.at("x0").as_int64();
            int y0 = road.at("y0").as_int64();

            if (road.as_object().contains("x1")) {
                int x1 = road.at("x1").as_int64();
                map.AddRoad(model::Road(model::Road::HORIZONTAL, { x0, y0 }, x1));
            }
            else {
                int y1 = road.at("y1").as_int64();
                map.AddRoad(model::Road(model::Road::VERTICAL, { x0, y0 }, y1));
            }
        }
    }

    void InsertBuildings(model::Map& map, const auto& map_json) {
        const json::array& buildings = map_json.at("buildings").as_array();
        if (!buildings.empty()) {
            for (const auto& building : buildings) {
                int x = building.at("x").as_int64();
                int y = building.at("y").as_int64();

                int w = building.at("w").as_int64();
                int h = building.at("h").as_int64();

                map.AddBuilding(model::Building({ {x, y}, {w, h} }));
            }
        }
    }

    void InsertOffices(model::Map& map, const auto& map_json) {
        const json::array& offices = map_json.at("offices").as_array();
        if (!offices.empty()) {
            for (const auto& office : offices) {
                model::Office::Id office_id(office.at("id").as_string().c_str());
                int x = office.at("x").as_int64();
                int y = office.at("y").as_int64();

                int offsetX = office.at("offsetX").as_int64();
                int offsetY = office.at("offsetY").as_int64();

                map.AddOffice(model::Office(office_id, { x, y }, { offsetX, offsetY }));
            }
        }
    }

}  // namespace json_loader
