#pragma once
#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
#include <iostream> 
#include <sstream>   
#include <string>
#include "../src/collision_detector.h"
#include "../src/test_class.h"

using namespace collision_detector;
using namespace test;

namespace Catch {
	template<>
	struct StringMaker<collision_detector::GatheringEvent> {
		static std::string convert(collision_detector::GatheringEvent const& value) {
			std::ostringstream tmp;
			tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

			return tmp.str();
		}
	};
}  // namespace Catch

namespace test_data {
	using namespace geom;

	const double dog_width = 0.6;

	const double item_width = 0.3;

	void AddZeroGatherer(TestClass& provider) {
		Gatherer gatherer;
		gatherer.width = dog_width;

		gatherer.start_pos = { 0.0, 0.0 };
		gatherer.end_pos = { 0.0, 0.0 };

		provider.SetGatherer(gatherer);
	}
	void AddGatherer_x(TestClass& provider) {
		double last_x = 0.0;
		double last_y = 0.0;
		if (provider.GatherersCount() != 0) {
			last_x = provider.GetGatherer(0).end_pos.x;
			last_y = provider.GetGatherer(0).end_pos.y;
		}
		Gatherer gatherer;
		gatherer.width = dog_width;

		gatherer.start_pos = { last_x, last_y };
		gatherer.end_pos = { last_x + 2.0, last_y };

		Item item;
		item.width = item_width;
		item.position = {(gatherer.start_pos.x + gatherer.end_pos.x) / 2.0 - 0.5, last_y};

		provider.SetGatherer(gatherer);
		provider.SetItem(item);
	}
	void AddGatherer_y(TestClass& provider) {
		double last_x = 0.0;
		double last_y = 0.0;
		if (provider.GatherersCount() != 0) {
			last_x = provider.GetGatherer(0).start_pos.x;
			last_y = provider.GetGatherer(0).end_pos.y;
		}
		Gatherer gatherer;
		gatherer.width = dog_width;

		gatherer.start_pos = { last_x, last_y};
		gatherer.end_pos = { last_x, last_y + 4.0 };

		Item item;
		item.width = item_width;
		item.position = { last_x, (gatherer.start_pos.y + gatherer.end_pos.y) / 2.0 + 0.5};


		provider.SetGatherer(gatherer);
		provider.SetItem(item);
	}
	void AddGatherer_xy(TestClass& provider) {
		double last_x = 0.0;
		double last_y = 0.0;
		if (provider.GatherersCount() != 0) {
			last_x = provider.GetGatherer(0).end_pos.x;
			last_y = provider.GetGatherer(0).end_pos.y;
		}
		Gatherer gatherer;
		gatherer.width = dog_width;

		gatherer.start_pos = { last_x, last_y };
		gatherer.end_pos = { last_x + 2.0, last_y + 2.0 };

		Item item;
		item.width = item_width;
		item.position = {(gatherer.start_pos.x + gatherer.end_pos.x) / 2.0, (gatherer.start_pos.y + gatherer.end_pos.y) / 2.0};


		provider.SetGatherer(gatherer);
		provider.SetItem(item);
	}

	TestClass GetTestProvider() {
		TestClass provider;

		AddGatherer_xy(provider); //координаты: {0.0, 0.0} {2.0, 2.0}, item {1.0, 1.0}
		AddGatherer_x(provider); //координаты: {2.0, 2.0} {4.0, 2.0}, item {2.5, 2.0} 
		AddGatherer_y(provider); //координаты: {2.0, 2.0} {2.0, 6.0}, item {2.0, 4.5}
		AddZeroGatherer(provider); //координаты: {0.0} {0.0}, no item

		Gatherer gatherer;
		gatherer.width = dog_width;
		gatherer.start_pos = { 0.0, 2.0 }; 
		gatherer.end_pos = { 8.0, 2.0 }; //item {3.0, 2.0} same as the second one

		provider.SetGatherer(gatherer);

		return provider;
	}
}


SCENARIO("Collision detector") {
	GIVEN("Item Gatherer Provider object") {
	TestClass test_provider = test_data::GetTestProvider();
	auto events = FindGatherEvents(test_provider);


		WHEN("There are three items and 5 different gatherers") {
			THEN("Only 4 of gatherers are able to collect loot and there are 4 events") {
				CHECK(events.size() == 4);
			}
		}

		GIVEN("Vector of time in chronological order") {
			std::vector<double> time_ = { 0.25, 0.3125, 0.5, 0.625 };


			WHEN("There are 4 events") {
				THEN("Events in chronological order") {
					for (size_t i = 0; i < events.size(); i++)
					{

					}
				}
			}

		}

		//проверка, что текущее значение таймера <= следующему значению


	}
}

// Напишите здесь тесты для функции collision_detector::FindGatherEvents
