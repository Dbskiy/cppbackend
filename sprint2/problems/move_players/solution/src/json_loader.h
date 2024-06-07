#pragma once

#include <filesystem>
#include <boost/json.hpp>
#include "model.h"

namespace json = boost::json;
using namespace std::literals;

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);

model::Map::DogSpeed DefaultDogSpeed(const json::object&);
model::Map::DogSpeed DogSpeedOnTheMap(const json::object& map_json, const auto def_speed);

void InsertRoads(model::Map&, const auto&);

void InsertBuildings(model::Map&, const auto&);

void InsertOffices(model::Map&, const auto&);

}  // namespace json_loader
