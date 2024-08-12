#pragma once
#include <string>
#include <variant>
#include <boost/json.hpp>
#include <unordered_map>
#include <vector>

namespace json = boost::json;

namespace loot_extra_data 
{
	using loot_variant = std::variant<std::string, int64_t, double>;
	class LootExtraData {
	public:
		LootExtraData(const json::array& loot_types) {
			SetLootProperties(loot_types);
		};

		const std::vector<std::unordered_map<std::string, loot_variant>>& GetLootTypesProp() const {
			return loot_types_prop_;
		}

	private:
		void SetLootProperties(const json::array& loot_types);

		std::vector<std::unordered_map<std::string, loot_variant>> loot_types_prop_;
	};
}