#include <catch2/catch_test_macros.hpp>

#include "../src/model.h"

using namespace model;


namespace TestData {

	Map::LootTypesProp prop1;
	Map map1{ Map::Id("1"), "Test_map1", Map::DogSpeed(0.0), Map::BagCapacity(0), Map::LootTypesNum(0), prop1 };
	model::Road road1(model::Road::HORIZONTAL, { 0, 0 }, 40);

	Map::LootTypesProp prop2 = {
		{
			{"name", std::string("key")},
			{"file", std::string("assets/key.obj")},
			{"type", std::string("obj")},
			{"scale", 0.03}
		},
		{
			{"name", std::string("wallet")},
			{"file", std::string("assets/wallet.obj")},
			{"type", std::string("obj")},
			{"scale", 0.01}
		}
	};

	Map map2{ Map::Id("2"), "Test_map2", Map::DogSpeed(0.0), Map::BagCapacity(0), Map::LootTypesNum(2), prop2 };
	model::Road road2(model::Road::VERTICAL, { 40, 0 }, 30);
}

SCENARIO("Model", "[Model]") {
	GIVEN("a game session with no loot types") {

		auto& map1 = TestData::map1;
		GameSession session{ &map1 };
		REQUIRE(*map1.GetLootTypesNum() == 0);

		WHEN("There are no roads to generate loot") {
			CHECK(map1.GetRoads().size() == 0);

			session.AddRandomLoot();
			THEN("No loot will be added to the session") {
				CHECK(session.GetAllLoot().size() == 0);
			}
		}
		WHEN("There is one road and no loot types") {
			map1.AddRoad(TestData::road1);
			REQUIRE(map1.GetRoads().size() == 1);

			THEN("No loot will be added to the session") {
				CHECK(session.GetAllLoot().size() == 0);
			}
		}
	}

	GIVEN("a game session with 2 loot types") {
		auto& map2 = TestData::map2;
		GameSession session{ &map2 };

		WHEN("There are no roads to generate loot") {
			CHECK(map2.GetRoads().size() == 0);

			THEN("No loot will be added to the session") {
				CHECK(session.GetAllLoot().size() == 0);
			}
		}

		WHEN("There is one road") {
			map2.AddRoad(TestData::road2);

			REQUIRE(map2.GetRoads().size() == 1);

			THEN("There will be two different loot objects") {
				for (int i = 0; i < *map2.GetLootTypesNum(); i++)
				{
					session.AddRandomLoot();
				}

				const auto& all_loot = session.GetAllLoot();
				REQUIRE(session.GetAllLoot().size() == 2);

				const auto& loot_1 = all_loot[0];
				const auto& loot_2 = all_loot[1];

				CHECK((loot_1.type_ >= 0 && loot_1.type_ < *map2.GetLootTypesNum()));
				CHECK((loot_2.type_ >= 0 && loot_2.type_ < *map2.GetLootTypesNum()));

				CHECK(((loot_1.pos_.x != loot_2.pos_.x) || (loot_1.pos_.y != loot_2.pos_.y)));

			}
		}

		WHEN("Loot object is generated") {
			THEN("It is not out of the road") {
				session.AddRandomLoot();

				const auto& loot_1 = session.GetAllLoot()[0];

				auto start = TestData::road2.GetStart();
				auto end = TestData::road2.GetEnd();

				CHECK((loot_1.pos_.x >= (start.x - 0.4) && loot_1.pos_.x <= (end.x + 0.4)));

				CHECK((loot_1.pos_.y >= start.y && loot_1.pos_.y <= end.y));
			}
		}
	}


}