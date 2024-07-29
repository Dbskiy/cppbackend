#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <random>
#include <optional>
#include <variant>

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

struct Speed {
    double Vx{0.0};
    double Vy{0.0};
};

struct Position {
    double x{};
    double y{};
};

struct Loot {
    int id{};
    int type_{};
    Position pos_;
    bool is_collected_ = false;
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

    static const double half_of_width;

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

    std::tuple<double, double> GetRoadStartPoint() const;

    std::tuple<double, double> GetRandomPoint() const;

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
        using DogSpeed = util::Tagged<double, Map>;
        using BagCapacity = util::Tagged<size_t, Map>;
        using Roads = std::vector<Road>;
        using Buildings = std::vector<Building>;
        using Offices = std::vector<Office>;
        using LootTypesNum = util::Tagged<int, Map>;
        using loot_variant = std::variant<std::string, int64_t, double>;
        using loot_type = std::unordered_map<std::string, loot_variant>;
        using LootTypesProp = std::vector<loot_type>;

        Map(Id id, std::string name, DogSpeed speed, BagCapacity bag_capacity,
            LootTypesNum loot_types_num, const LootTypesProp& loot_types_prop) noexcept
            : id_(std::move(id))
            , name_(std::move(name))
            , speed_(std::move(speed))
            , bag_capacity_(std::move(bag_capacity))
            , loot_types_num_(std::move(loot_types_num))
            , loot_types_prop_(loot_types_prop)
        {
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

        const DogSpeed& GetDogSpeed() const noexcept {
            return speed_;
        }

        const LootTypesNum& GetLootTypesNum() const noexcept {
            return loot_types_num_;
        }

        const LootTypesProp& GetLootTypesProp() const noexcept {
            return loot_types_prop_;
        }

        const loot_type& GetCertainLootType(int type) const noexcept {
            return loot_types_prop_[type];
        }

        const BagCapacity& GetBagCapacity() const noexcept {
            return bag_capacity_;
        }

        void AddRoad(const Road& road) {
            roads_.emplace_back(road);

        }

        const Road& GetRandomRoad() const {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis_road(0, roads_.size() - 1);

            int road_index = dis_road(gen);

            return roads_[road_index];
        }

        std::vector<Road> FindRoadsAt(double x, double y) const;

        void AddBuilding(const Building& building) {
            buildings_.emplace_back(building);
        }

        void AddOffice(Office office);

    private:
        using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

        Id id_;
        std::string name_;
        DogSpeed speed_;
        BagCapacity bag_capacity_;
        Roads roads_;
        Buildings buildings_;
        LootTypesNum loot_types_num_;
        LootTypesProp loot_types_prop_;

        OfficeIdToIndex warehouse_id_to_index_;
        Offices offices_;
    };

class Dog {
public:
    using DogId = util::Tagged<std::uint64_t, Dog>;
    using DogName = util::Tagged<std::string, Dog>;

    explicit Dog(DogId dog_id, DogName name, Position spawn_point)
        : dog_id_(Dog::DogId(dog_id)), 
        name_(name),
        spawn_point_(spawn_point),
        state_{} {
        state_.position = spawn_point;
    };

    struct Bag {

        bool isBagFull(const Map::BagCapacity& bag_capacity) const noexcept {
            return dog_loot_.size() == *bag_capacity;
        }

        void FreeBag() {
            dog_loot_.clear();
            dog_loot_.shrink_to_fit();
        }
        std::vector<Loot> dog_loot_;
    };

    struct DogState {
        Position prev_position;
        Position position;
        Speed speed;
        std::string direction = "U";
        Bag bag;
        int score = 0;

        void SetDefaultSpeed() {
            speed.Vx = 0.0;
            speed.Vy = 0.0;
        }
    };

    DogId GetDogId() const noexcept {
        return dog_id_;
    }

    DogName GetDogName() const noexcept {
        return name_;
    }

    Position GetSpawnPoint() const noexcept{
        return spawn_point_;
    }

    DogState& GetDogState() noexcept {
        return state_;
    }

private:
    DogId dog_id_;
    DogName name_;
    Position spawn_point_;
    DogState state_;
};

class GameSession {
public:
    using SessionId = util::Tagged<unsigned, GameSession>;
    using Dogs = std::vector<std::shared_ptr<Dog>>;
    using AllLoot = std::vector<Loot>;

    explicit GameSession(SessionId id, const Map* map) : id_(id), map_(map) {};

    const SessionId GetId() const noexcept {
        return id_;
    }

    Dogs& GetDogs() {
        return dogs_;
    }

    void SetDogs(Dogs& dogs) {
        dogs_ = dogs;
    }

    const std::shared_ptr<Dog> GetDogById(Dog::DogId id) const noexcept {
        if (*id < dogs_.size()) {
            return dogs_[*id];
        }
        return nullptr;
    }

    std::shared_ptr<Dog> AddDog(Dog::DogName user_name, const bool randomize_spawn) {
        auto spawn_point = GenerateSpawn(randomize_spawn);

        auto new_dog = std::make_shared<Dog>(Dog::DogId(dogs_.size()), user_name, spawn_point);
        dogs_.emplace_back(new_dog);
        return new_dog;

    }

    void AddRandomLoot();

    void RemoveCollectedLoot();

    AllLoot& GetAllLoot() {
        return all_loot_;
    }

    void SetAllLoot(const AllLoot& all_loot) {
        all_loot_ = all_loot;
    }

    const Map* GetMap() const noexcept {
        return map_;
    }

private:
    SessionId id_;
    Dogs dogs_;
    const Map* map_;

    AllLoot all_loot_;

    Position GenerateSpawn(const bool randomize_spawn);
    Position GenerateRandomPointOnMap();
    int GenerateRandomLootType();
};

class Game {
public:
    using Maps = std::vector<Map>;
    using Sessions = std::vector<std::shared_ptr<GameSession>>;
    using PairDogAndSession = std::pair<std::shared_ptr<Dog>, std::shared_ptr<GameSession>>;

    struct LootGeneratorConfig {
        double period_;
        double probability_;
    };

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Sessions& GetSessions() const noexcept {
        return sessions_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    const std::shared_ptr<GameSession> FindGameSession(GameSession::SessionId id) const noexcept {
        if (*id < sessions_.size()) {
            return sessions_[*id];
        }

        return nullptr;
    }

    std::shared_ptr<GameSession> ConnectToSession(Map::Id map_id);

    void SetSessions(Sessions& sessions) {
        sessions_ = sessions;
    }

    void SetLootGeneratorConfig(double period, double probability) {
        loot_generator_config_ = LootGeneratorConfig{ period, probability };
    }

    std::optional<LootGeneratorConfig> GetLootGeneratorConfig() const noexcept {
        return loot_generator_config_;
    }



private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;

    Sessions sessions_;
    std::optional<LootGeneratorConfig> loot_generator_config_;
};

}  // namespace model
