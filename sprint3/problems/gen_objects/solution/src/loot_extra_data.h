#pragma once
#include <string>
#include <variant>
#include <boost/json.hpp>
#include <unordered_map>
#include <vector>

namespace json = boost::json;
//using namespace std::literals;

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
		void SetLootProperties(const json::array& loot_types) {
			for (const auto& item : loot_types) {
				if (item.is_object()) {
					const json::object& obj = item.as_object();

					std::unordered_map<std::string, loot_variant> loot_properties;
					for (const auto& pair : obj) {
						loot_variant loot_var;	
						const std::string& key = pair.key().data();
						const json::value& value = pair.value();

						if (value.is_string()) {
							loot_var = value.as_string().c_str();
						}
						else if (value.is_int64()) {
							loot_var = value.as_int64();
						}
						else if (value.is_double()) {
							loot_var = value.as_double();
						}
						loot_properties[key] = loot_var;
					}

					loot_types_prop_.emplace_back(std::move(loot_properties));
				}
			}
		}

		std::vector<std::unordered_map<std::string, loot_variant>> loot_types_prop_;
	};
}