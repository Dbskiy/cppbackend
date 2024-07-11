#pragma once
#include "collision_detector.h"
#include <random>

using namespace collision_detector;

namespace test {
	class TestClass : public ItemGathererProvider {
	public:
		void SetItem(Item& item) {
			items_.emplace_back(item);
		}

		void SetGatherer(Gatherer& gatherer) {
			gatherers_.emplace_back(gatherer);
		}

		size_t ItemsCount() const override {
			return items_.size();
		}

		Item GetItem(size_t idx) const override {
			if (idx < items_.size()) return items_[idx];
		}

		virtual size_t GatherersCount() const override {
			return gatherers_.size();
		}

		Gatherer GetGatherer(size_t idx) const override {
			if (idx < gatherers_.size()) return gatherers_[idx];
		}


	private:
		std::vector<Item> items_;
		std::vector<Gatherer> gatherers_;
	};

}