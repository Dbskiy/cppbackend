#include "request_handler.h"

namespace http_handler {
	StringResponse RequestHandler::MakeStringResponse(http::status status, std::string_view body, unsigned http_version, std::string_view content_type)
	{
		StringResponse response(status, http_version);
		response.set(http::field::content_type, content_type);
		response.body() = body;
		response.content_length(body.size());
		return response;

	}

	std::string_view RequestHandler::MapsResponse(std::string& response_body)
	{
		json::array maps; 

		std::vector<model::Map> maps_vector = game_.GetMaps(); //все доступные карты 

		for (const auto& map : maps_vector) {
			json::object map_properties; //объект, содержащий свойства одной карты

			map_properties["id"] = *map.GetId();
			map_properties["name"] = map.GetName();

			maps.push_back(map_properties);
		}

		response_body = json::serialize(maps);

		return std::string_view(response_body.data(), response_body.size());
	}

	std::string_view RequestHandler::MapById(std::string& response_body, const std::string& map_id, model::Map::Id id)
	{
        const model::Map* this_map = game_.FindMap(id);

        json::object map;

        map["id"] = *this_map->GetId();
        map["name"] = this_map->GetName();

        json::array roads;
        json::array buildings;
        json::array offices;

        for (const auto& road : this_map->GetRoads()) {
            json::object coordinates;
            coordinates["x0"] = road.GetStart().x;
            coordinates["y0"] = road.GetStart().y;

            if (road.IsHorizontal()) {
                coordinates["x1"] = road.GetEnd().x;
            }
            else {
                coordinates["y1"] = road.GetEnd().y;
            }

            roads.push_back(coordinates);
        }
        map["roads"] = std::move(roads);

        if (!this_map->GetBuildings().empty()) {
            for (const auto& building : this_map->GetBuildings()) {
                json::object properties;

                properties["x"] = building.GetBounds().position.x;
                properties["y"] = building.GetBounds().position.y;

                properties["w"] = building.GetBounds().size.width;
                properties["h"] = building.GetBounds().size.height;

                buildings.push_back(properties);
            }
            map["buildings"] = std::move(buildings);
        }

        if (!this_map->GetOffices().empty()) {
            for (const auto& office : this_map->GetOffices()) {
                json::object properties;

                properties["id"] = *office.GetId();

                properties["x"] = office.GetPosition().x;
                properties["y"] = office.GetPosition().y;

                properties["offsetX"] = office.GetOffset().dx;
                properties["offsetY"] = office.GetOffset().dy;

                offices.push_back(properties);
            }
            map["offices"] = std::move(offices);
        }

        response_body = json::serialize(map);

        return std::string_view(response_body.data(), response_body.size());
	}

    std::string_view RequestHandler::MapNotFound(std::string& response_body)
    {
        json::object notFound;

        notFound["code"] = "mapNotFound";
        notFound["message"] = "Map not found";

        response_body = json::serialize(notFound);

        return std::string_view(response_body.data(), response_body.size());
    }

    std::string_view RequestHandler::BadRequest(std::string& response_body)
    {
        json::object badRequest;

        badRequest["code"] = "badRequest";
        badRequest["message"] = "Bad request";

        response_body = json::serialize(badRequest);

        return std::string_view(response_body.data(), response_body.size());
    }



}  // namespace http_handler
