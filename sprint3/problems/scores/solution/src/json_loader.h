#pragma once

#include <filesystem>
#include <boost/json.hpp>
#include "model.h"
#include "loot_extra_data.h"

namespace json = boost::json;
using namespace std::literals;

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);

model::Map::DogSpeed DefaultDogSpeed(const json::object&);

model::Map::BagCapacity DefaultBagCapacity(const json::object&);

void LootConfiguration(model::Game& game, const json::object& json_object);

model::Map::LootTypesProp LootTypesProperties(json::array loot_types);

model::Map::DogSpeed DogSpeedOnTheMap(const json::object& map_json, const auto def_speed);

model::Map::BagCapacity BagCapacityOnTheMap(const json::object& map_json, model::Map::BagCapacity def_bag_capacity);

void InsertRoads(model::Map&, const auto&);

void InsertBuildings(model::Map&, const auto&);

void InsertOffices(model::Map&, const auto&);

}  // namespace json_loader
