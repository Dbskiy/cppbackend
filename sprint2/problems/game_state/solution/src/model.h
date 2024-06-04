#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <ctime>
#include <random>

#include "tagged.h"

namespace model {

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Point position;
    Size size;
};

struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        HorizontalTag() = default;
    };

    struct VerticalTag {
        VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

private:
    Point start_;
    Point end_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Dog {
public:
    ////информация о собаке: имя и id
    using DogId = util::Tagged<std::uint64_t, Dog>;
    using DogName = util::Tagged<std::string, Dog>;
    using SpawnPoint = util::Tagged<std::vector<double>, Dog>;
    ////id - ключ 

    explicit Dog(std::uint64_t dog_id, DogName name, std::vector<double>& spawn_point)
        : dog_id_(Dog::DogId(dog_id)), 
        name_(name),
        spawn_point_(spawn_point) {};

    DogId GetDogId() const noexcept {
        return dog_id_;
    }

    DogName GetDogName() const noexcept {
        return name_;
    }

    std::vector<double> GetSpawnPoint() const noexcept{
        return spawn_point_;
    }

private:
    DogId dog_id_;
    DogName name_;
    std::vector<double> spawn_point_;
};

class GameSession {
public:
    using Dogs = std::vector<std::shared_ptr<Dog>>;

    explicit GameSession(Map* map) : map_(map) {};

    const Dogs& GetDogs() const noexcept {
        return dogs_;
    }

    std::shared_ptr<Dog> AddDog(Dog::DogName user_name) {
        auto spawn_point = GenerateSpawn();

        auto new_dog = std::make_shared<Dog>(dogs_.size(), user_name, spawn_point);
        dogs_.emplace_back(new_dog);
        return new_dog;

    }

    const Map* GetMap() const noexcept {
        return map_;
    }

private:
    Dogs dogs_;
    Map* map_;

    std::vector<double> GenerateSpawn() {

        //auto& roads = map_->GetRoads();
        //
        //auto road_num = rand() % (roads.size());

        //auto& road = roads[road_num];

/*
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> distrib_x(road.GetStart().x, road.GetEnd().x);
        std::uniform_real_distribution<float> distrib_y(road.GetStart().y, road.GetEnd().y)*/;
        double x = 0.0;
        double  y = 0.0;


        std::vector<double> points;
        points.push_back(x);
        points.push_back(y);
        return points;
    }

};

class Game {
public:
    using Maps = std::vector<Map>;
    using PairDogAndSession = std::pair<std::shared_ptr<Dog>, std::shared_ptr<GameSession>>;

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    std::shared_ptr<GameSession> FindSession(Map::Id map_id);

private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    void AddGameSession(std::shared_ptr<GameSession> game_session);
    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;

    std::vector<std::shared_ptr<GameSession>> sessions_;
};

}  // namespace model
