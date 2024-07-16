#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

const double Road::half_of_width = 0.4;

std::tuple<double, double> Road::GetRoadStartPoint() const
{
    Coord min_x, min_y, max_x, max_y;

    if (IsHorizontal()) {
        min_x = std::min(start_.x, end_.x);
        max_x = std::max(start_.x, end_.x);
        min_y = max_y = start_.y;
    }
    else {
        min_y = std::min(start_.y, end_.y);
        max_y = std::max(start_.y, end_.y);
        min_x = max_x = start_.x;
    }

    return std::make_tuple(min_x, min_y);
}

std::tuple<double, double> Road::GetRandomPoint() const
{
    std::random_device rd;
    std::mt19937 gen(rd());

    double min_x, min_y, max_x, max_y;

    if (IsHorizontal()) {
        min_x = std::min(start_.x, end_.x);
        max_x = std::max(start_.x, end_.x);
        min_y = start_.y - 0.4;
        max_y = end_.y + 0.4;
    }
    else {
        min_y = std::min(start_.y, end_.y);
        max_y = std::max(start_.y, end_.y);
        min_x = start_.x - 0.4;
        max_x = end_.x + 0.4;
    }

    std::uniform_real_distribution<double> dis_x(min_x, max_x);
    std::uniform_real_distribution<double> dis_y(min_y, max_y);

    return std::make_tuple(dis_x(gen), dis_y(gen));
}

std::vector<Road> Map::FindRoadsAt(double x, double y) const
{
    std::vector<Road> result;
    auto half_of_width = Road::half_of_width;

    for (auto& road : GetRoads()) {

        Point start{ 0, 0 };
        Point end{ 0, 0 };
        if (road.IsHorizontal()) {
            start.x = std::min(road.GetStart().x, road.GetEnd().x);
            end.x = std::max(road.GetStart().x, road.GetEnd().x);
            start.y = end.y = road.GetStart().y;
        }
        else {
            start.y = std::min(road.GetStart().y, road.GetEnd().y);
            end.y = std::max(road.GetStart().y, road.GetEnd().y);
            start.x = end.x = road.GetStart().x;
        }

        if ((x >= start.x - half_of_width) && (x <= end.x + half_of_width) && (y >= start.y - half_of_width) && (y <= end.y + half_of_width)) {
            result.emplace_back(road);
        }
    }

    return result;
}


void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

void GameSession::AddRandomLoot()
{
    {
        if (*map_->GetLootTypesNum() > 0) {
            Loot loot;

            loot.pos_ = GenerateRandomPointOnMap();

            loot.type_ = GenerateRandomLootType();

            loot.id = all_loot_.size();

            all_loot_.emplace_back(loot);
        }
    }
}

void GameSession::RemoveCollectedLoot()
{
    for (auto it = all_loot_.begin(); it != all_loot_.end(); ) {
        if (it->is_collected_) {
            it = all_loot_.erase(it);
        }
        else ++it;
    }
}

Position GameSession::GenerateSpawn(const bool randomize_spawn)
{
    auto& roads = map_->GetRoads();
    Position pos;

    if (randomize_spawn) {
        auto [x, y] = map_->GetRandomRoad().GetRandomPoint();

        pos.x = x;
        pos.y = y;
    }
    else {
        auto& first_road = roads.front();
        auto [x, y] = first_road.GetRoadStartPoint();
        pos.x = x;
        pos.y = y;
    }

    return pos;
}

Position GameSession::GenerateRandomPointOnMap()
{
    auto [x, y] = map_->GetRandomRoad().GetRandomPoint();

    return { x, y };
}

int GameSession::GenerateRandomLootType()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis_loot_type(0, *map_->GetLootTypesNum() - 1);

    return dis_loot_type(gen);
}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

//нахождение нужной сессии без добавления собаки
std::shared_ptr<GameSession> Game::FindSession(Map::Id map_id)
{
    for (auto& session : sessions_) {
        if (session->GetMap()->GetId() == map_id && session->GetDogs().size() - 1 < 5) {
            return session;
        }
    }

    const Map* map = FindMap(map_id);
    if (map) {
        auto new_session = std::make_shared<GameSession>(const_cast<Map*>(map));
        AddGameSession(new_session);
        return new_session;
    }

    return nullptr;

}

void Game::AddGameSession(std::shared_ptr<GameSession> game_session)
{
    sessions_.emplace_back(std::move(game_session));
}


}  // namespace model
