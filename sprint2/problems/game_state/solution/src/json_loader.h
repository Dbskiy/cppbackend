#pragma once

#include <filesystem>
#include <boost/json.hpp>
#include "model.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);

void InsertRoads(model::Map&, const auto&);

void InsertBuildings(model::Map&, const auto&);

void InsertOffices(model::Map&, const auto&);

}  // namespace json_loader
