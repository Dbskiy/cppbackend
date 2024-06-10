#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

std::vector<Road> Map::FindRoadsAt(double x, double y) const
{
    std::vector<Road> result;

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

        if ((x >= start.x - 0.4) && (x <= end.x + 0.4) && (y >= start.y - 0.4) && (y <= end.y + 0.4)) {
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
    //здесь просто добавление
    sessions_.emplace_back(std::move(game_session));
}
}  // namespace model
