#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

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

Game::PairDogAndSession Game::AddDogToSession(const Map::Id& map_id, const Dog::DogName user_name)
{
    for (auto& session : sessions_) {
        if (session->GetMap()->GetId() == map_id && session->GetDogs().size() < 10) {
            session->AddDog(user_name);
            return {std::make_shared<Dog>(session->GetDogs().back()), session};
        }
    }
    const Map* map = FindMap(map_id);
    if (map) {
        auto new_session = std::make_shared<GameSession>(const_cast<Map*>(map));
        new_session->AddDog(user_name);
        AddGameSession(new_session);
        return { std::make_shared<Dog>(new_session->GetDogs().back()), new_session };
    }
    else {
        throw std::invalid_argument("Map with id "s + *map_id + " does not exist"s);
    }
}

void Game::AddGameSession(std::shared_ptr<GameSession> game_session)
{
    //здесь просто добавление
    sessions_.emplace_back(std::move(game_session));
}
}  // namespace model
