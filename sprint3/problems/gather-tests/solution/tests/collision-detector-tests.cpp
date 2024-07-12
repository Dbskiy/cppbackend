#pragma once
#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_contains.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>

#include <iostream> 
#include <sstream>   
#include <string>
#include <algorithm>

#include "../src/collision_detector.h"
#include "../src/test_class.h"


using namespace collision_detector;
using namespace test;
using Catch::Matchers::Contains;
using Catch::Matchers::Predicate;

namespace Catch {

	using namespace geom;

	const double dog_width = 0.6;

	const double item_width = 0.3;
	template<>
	struct StringMaker<collision_detector::GatheringEvent> {
		static std::string convert(collision_detector::GatheringEvent const& value) {
			std::ostringstream tmp;
			tmp << "(" << value.gatherer_id << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

			return tmp.str();
		}
	};
}  // namespace Catch

namespace my_matcher {
	using namespace std::literals;
	template <typename Range>
	struct IsPermutationMatcher : Catch::Matchers::MatcherGenericBase {
		IsPermutationMatcher(Range range)
			: range_{ std::move(range) } {
			std::sort(std::begin(range_), std::end(range_));
		}
		IsPermutationMatcher(IsPermutationMatcher&&) = default;

		template <typename OtherRange>
		bool match(OtherRange other) const {
			using std::begin;
			using std::end;

			std::sort(begin(other), end(other));
			return std::equal(begin(range_), end(range_), begin(other), end(other));
		}

		std::string describe() const override {
			// Описание свойства, проверяемого матчером:
			return "Is permutation of: "s + Catch::rangeToString(range_);
		}

	private:
		Range range_;
	};

	template<typename Range>
	IsPermutationMatcher<Range> IsPermutation(Range&& range) {
		return IsPermutationMatcher<Range>{std::forward<Range>(range)};
	}
}

namespace test_data {
	using namespace geom;

	const double dog_width = 0.6;

	const double item_width = 0.3;

	Point2D RandomItemPos(Point2D start, Point2D end) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> dis(start.x, end.x);
		auto point = dis(gen);

		return { point, point};
	}

	TestClass GetRandomItemTestProvider() {
		TestClass test_provider;

		Point2D last_pos = { 0, 0 };

		for (int i = 0; i < 3; i++)
		{
			Gatherer gatherer = { .start_pos = last_pos, .end_pos = {last_pos.x + 2.0, last_pos.y + 2.0}, .width = dog_width };
			Item item = { .position = RandomItemPos(gatherer.start_pos, gatherer.end_pos), .width = item_width };

			last_pos.x += 2.0;
			last_pos.y += 2.0;

			test_provider.SetGatherer(gatherer);
			test_provider.SetItem(item);
		}

		Gatherer cross_gatherer = { .start_pos = {1.0, 1.0}, .end_pos = {3.0, 3.0}, .width = dog_width };

		test_provider.SetGatherer(cross_gatherer);

		Item item = { .position = {5.3534, 5.3534}, .width = item_width };

		return test_provider;
	}

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
		gatherer.end_pos = { last_x, last_y + 2.0 };

		Item item;
		item.width = item_width;
		item.position = { last_x, (gatherer.start_pos.y + gatherer.end_pos.y) / 2.0 + 0.25};


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

	TestClass GetBadTestProvider1() {
		TestClass provider;

		AddZeroGatherer(provider);

		Gatherer gatherer1 = { .start_pos = { 10.0, 10.0 }, .end_pos = { 12.0, 12.0 }, .width = dog_width };

		Item item1 = { .position = {12.5, 12.5}, .width = item_width };

		Gatherer gatherer2 = { .start_pos = {5.5, 5.5}, .end_pos = {7.5, 5.5}, .width = dog_width };
		Item item2 = { .position = {2.2, 2.2}, .width = item_width };

		Gatherer gatherer3 = { .start_pos = {1.0, 1.0}, .end_pos = {1.0, 3.0}, .width = dog_width };


		provider.SetGatherer(gatherer1);
		provider.SetGatherer(gatherer2);
		provider.SetGatherer(gatherer3);

		provider.SetItem(item1);
		provider.SetItem(item2);

		return provider;
	}

	TestClass GetTestProvider() {
		TestClass provider;

		AddGatherer_xy(provider); //координаты: {0.0, 0.0} {2.0, 2.0}, item {1.0, 1.0}
		AddGatherer_x(provider); //координаты: {2.0, 2.0} {4.0, 2.0}, item {2.5, 2.0} 
		AddGatherer_y(provider); //координаты: {2.0, 2.0}, {2.0, 4.0}, item {2.0, 3.25}
		AddZeroGatherer(provider); //координаты: {0.0} {0.0}, no item
		// {0.25, 0.3125, 0.5, 0.625 } //2,5,1,3
		Gatherer gatherer;
		gatherer.width = dog_width;
		gatherer.start_pos = { 1.875, 2.0 }; 
		gatherer.end_pos = { 3.875, 2.0 }; //item {2.5, 2.0} same as the second one

		provider.SetGatherer(gatherer);

		return provider;
	}
}



SCENARIO("Collision detector") {
	using namespace test_data;
	using namespace my_matcher;

	auto between_zero_and_one = [](double x) {
		return (x > 0 && x < 1);
		};

	auto less_or_equal_to_width_sum = [](double x) {
		return (sqrt(x) <= dog_width + item_width);
		};


	GIVEN("Bad Item Gatherer Provider object with all non-movable gatherers") {
		TestClass bad_provider = GetBadTestProvider1();
		auto events = FindGatherEvents(bad_provider);

		WHEN("There are 4 different gatherers and 2 items that can't be collected") {
			THEN("No gathering events") {
				REQUIRE(events.size() == 0);
			}
		}
	}

	GIVEN("Item Gatherer Provider with random items") {
		TestClass provider = GetRandomItemTestProvider();
		auto events = FindGatherEvents(provider);

		WHEN("There are 4 different gatherers and 4 different items") {
			THEN("at least gathering events") {
				REQUIRE(events.size() >= 3);
			}
		}

		WHEN("At least 3 gathering events") {
			THEN("Every event has the correct item id and gatherer id") {
				for (size_t i = 0; i < events.size(); i++)
				{
					auto& event = events[i];
					CHECK(event.gatherer_id < provider.GatherersCount());
					CHECK(event.item_id < provider.ItemsCount());

				}
			}
		}
		std::vector<double> sq_distances;
		sq_distances.reserve(events.size());

		std::transform(events.begin(), events.end(), std::back_inserter(sq_distances),
			[](const GatheringEvent& event) { return event.sq_distance; });

		WHEN("At least 3 gathering events") {
			THEN("Square of distance is less or equal to sum of dog and item width") {
				CHECK_THAT(sq_distances, Contains(Predicate<double>(less_or_equal_to_width_sum)));
			}
		}

		std::vector<double> times;
		times.reserve(events.size());
		std::transform(events.begin(), events.end(), std::back_inserter(times),
			[](const GatheringEvent& event) { return event.time; });

		WHEN("At least 3 gathering events") {
			THEN("Time is bigger than 0 and less than 1 and events are in chronological order") {
				CHECK_THAT(times, Contains(Predicate<double>(between_zero_and_one)));
				CHECK(std::is_sorted(times.begin(), times.end()));
			}
		}

	}

	GIVEN("Correct Item Gatherer Provider object") {
		TestClass test_provider = GetTestProvider();
		auto events = FindGatherEvents(test_provider);


		WHEN("There are 3 items and 5 different gatherers") {
			THEN("Only 4 of gatherers are able to collect loot and there are 4 events") {
				REQUIRE(events.size() == 4);
			}
		}

		WHEN("There are 4 gathering events") {
			THEN("Every event has the correct item id and gatherer id") {
				for (size_t i = 0; i < events.size(); i++)
				{
					auto& event = events[i];
					CHECK(event.gatherer_id < test_provider.GatherersCount());
					CHECK(event.item_id < test_provider.ItemsCount());
				}
			}
		}

		std::vector<double> sq_distances;
		sq_distances.reserve(events.size());

		std::transform(events.begin(), events.end(), std::back_inserter(sq_distances),
			[](const GatheringEvent& event) { return event.sq_distance; });

		WHEN("There are 4 events") {
			THEN("Square of distance is less or equal to sum of dog and item width") {
				CHECK_THAT(sq_distances, Contains(Predicate<double>(less_or_equal_to_width_sum)));
			}
		}

		std::vector<double> times;
		times.reserve(events.size());
		std::transform(events.begin(), events.end(), std::back_inserter(times),
			[](const GatheringEvent& event) { return event.time; });

		WHEN("There are 4 events") {
			THEN("Time is bigger than 0 and less than 1 and events are in chronological order") {
				CHECK_THAT(times, Contains(Predicate<double>(between_zero_and_one)));
				CHECK(std::is_sorted(times.begin(), times.end()));
				CHECK_THAT(times, IsPermutation(std::vector{ 0.25, 0.3125, 0.5, 0.625 }));
			}
		}

	}
}
