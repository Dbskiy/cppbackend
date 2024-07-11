#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
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

	Point2D RandomXEndPos() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> dis_x(2, 5);

		return { dis_x(gen), 0 };
	}

	Point2D RandomYEndPos() {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> dis_y(2, 5);

		return { 0 , dis_y(gen) };
	}

	Point2D RandomItemPos(double start_x, double end_x) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<double> dis_x(start_x + 0.5, end_x - 0.5);

		return { dis_x(gen), 0 };
	}

	TestClass GetTestProvider() {
		TestClass test_provider;

		for (int i = 0; i < 5; i++)
		{
			Gatherer gatherer;
			gatherer.width = dog_width;

			//три item'а, которые будут подобраны минимум тремя собаками
			if (i % 2 == 0) {
				Item item;

				item.width = 0.3;

				gatherer.start_pos = { 0, 0 };
				gatherer.end_pos = RandomXEndPos();

				item.position = RandomItemPos(gatherer.start_pos.x, gatherer.end_pos.x);
				test_provider.SetItem(item);
			}
			else { //две собаки, которые не полдберут никаких объектов item
				gatherer.start_pos = { 0, 1.0 };

				gatherer.end_pos = RandomYEndPos();
			}

			test_provider.SetGatherer(gatherer);
		}

		//собака с нулевым путем
		Gatherer zero_gatherer;
		zero_gatherer.width = dog_width;
		zero_gatherer.start_pos = { 0.0, 0.0 };
		zero_gatherer.end_pos = { 0.0, 0.0 };

		test_provider.SetGatherer(zero_gatherer);

		return test_provider;
	}
	/*TestClass GetTestProvider1() {
		TestClass test_provider;

		double last_pos_x = 0.0;
		double last_pos_y = 0.0;

		for (int i = 1; i < 6; i++)
		{
			Gatherer gatherer;
			gatherer.width = dog_width;

			Item item;
			item.width = item_width;
			
			if (i % 2 == 0) {
				gatherer.start_pos = { last_pos_x, 0.0 };
				gatherer.end_pos = { last_pos_x + static_cast<double>(i), 0.0 };

				item.position = {last_pos_x + }
			}

			gatherer.end_pos()

				Item item;
		}
	}*/

}


SCENARIO("Collision detector") {

	GIVEN("Item Gatherer Provider object") {
		TestClass test_provider = test_data::GetTestProvider();
		auto events = FindGatherEvents(test_provider);

		WHEN("tralala") {
			THEN("tralala") {
				CHECK((events.size() >= 3 && events.size() <= 6));
			}
		}

		//проверка, что текущее значение таймера <= следующему значению


	}
}

// Напишите здесь тесты для функции collision_detector::FindGatherEvents