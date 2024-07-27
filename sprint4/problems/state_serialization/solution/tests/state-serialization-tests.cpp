#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>

#include "../src/model.h"
#include "../src/state_serialization.h"

using namespace model;
using namespace std::literals;
namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

}  // namespace

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        auto dog = [] {
            Dog dog{ Dog::DogId{42}, Dog::DogName{"Pluto"s}, {0.0, 0.0} };

            auto& dog_state = dog.GetDogState();
            dog_state.score = 100;
            Loot loot{ .id = 0, .type_ = 1, .pos_ = {1.0, 1.0}, .is_collected_ = true };
            dog_state.bag.dog_loot_.push_back(loot);
            dog_state.direction = "U";

            dog_state.speed = {2.3, -1.2};
            return std::make_shared<Dog>(dog);
        }();

        WHEN("dog is serialized") {
            {
                serialization::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::DogRepr repr;
                input_archive >> repr;
                auto restored = repr.Restore();
                auto& state = restored->GetDogState();

                CHECK(dog->GetDogId() == restored->GetDogId());
                CHECK(dog->GetDogName() == restored->GetDogName());
                CHECK(((dog->GetSpawnPoint().x == restored->GetSpawnPoint().x) && (dog->GetSpawnPoint().y == restored->GetSpawnPoint().y)));
                CHECK(dog->GetDogState().direction == state.direction);
                CHECK(((dog->GetDogState().position.x == state.position.x) && (dog->GetDogState().position.y == state.position.y)));
                CHECK(((dog->GetDogState().prev_position.x == state.prev_position.x)
                    && (dog->GetDogState().prev_position.y == state.prev_position.y)));
                CHECK(dog->GetDogState().score == state.score);
                CHECK(((dog->GetDogState().speed.Vx == state.speed.Vx) && (dog->GetDogState().speed.Vy == state.speed.Vy)));
            }
        }
    }
}

