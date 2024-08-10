#include "loot_extra_data.h"

namespace loot_extra_data {
	void LootExtraData::SetLootProperties(const json::array& loot_types)
	{
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

}
