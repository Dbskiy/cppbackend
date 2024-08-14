#pragma once
#include "collision_detector.h"

using namespace collision_detector;

namespace providers {

	class GathererProvider : public ItemGathererProvider {
	public:
		void SetItem(Item& item) {
			items_.push_back(item);
		}

		void SetGatherer(Gatherer& gatherer) {
			gatherers_.push_back(gatherer);
		}

		size_t ItemsCount() const override {
			return items_.size();
		}

		Item GetItem(size_t idx) const override {
			try {
				return items_.at(idx);
			} catch(const std::out_of_range&) {
				return Item{};
			}
		}

		size_t GatherersCount() const override {
			return gatherers_.size();
		}

		Gatherer GetGatherer(size_t idx) const override {
			try {
				return gatherers_.at(idx);
			}
			catch (const std::out_of_range&) {
				return Gatherer{};
			}
		}


	private:
		std::vector<Item> items_;
		std::vector<Gatherer> gatherers_;
	};

	class DelivererProvider : public ItemsDelivererProvider {
	public:
		void SetBase(Base& base) {
			bases_.emplace_back(base);
		}

		void SetDeliverer(Gatherer& deliverer) {
			deliverers_.emplace_back(deliverer);
		}

		size_t BasesCount() const override {
			return bases_.size();
		}
		Base GetBase(size_t idx) const override {
			try {
				return bases_.at(idx);
			}
			catch (const std::out_of_range&) {
				return Base{};
			}
		}
		
		size_t DeliverersCount() const override {
			return deliverers_.size();
		}

		Gatherer GetDeliverer(size_t idx) const override {
			try {
				return deliverers_.at(idx);
			}
			catch (const std::out_of_range&) {
				return Gatherer{};
			}
		}
		
	private:
		std::vector<Base> bases_;
		std::vector<Gatherer> deliverers_;
	};
}
