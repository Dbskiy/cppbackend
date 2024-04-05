#pragma once
#include "http_server.h"
#include "model.h"


namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

using namespace std::literals;

// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APP_JSON = "application/json"sv;
};

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
        std::string_view content_type = ContentType::APP_JSON) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        return response;

    }

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        // Обработать запрос request и отправить ответ, используя send

        if (req.method() == http::verb::get) {

            std::string response_body = "";

            if (req.target() == "/api/v1/maps/"s) {

                
                send(MakeStringResponse(http::status::ok, MapsResponse(response_body), req.version()));
            }
            else if (req.target().substr(0, 13) == "/api/v1/maps/" && req.target().size() > 13) {

                std::string map_id(req.target().substr(13));
                model::Map::Id id(map_id.c_str());

                if (game_.FindMap(id) != nullptr) {
                    //MapById(response_body, map_id, id);
                    send(MakeStringResponse(http::status::ok, MapById(response_body, map_id, id), req.version()));
                }
                else {
                    send(MakeStringResponse(http::status::not_found, MapNotFound(response_body), req.version()));
                }
            }
            else {
                send(MakeStringResponse(http::status::bad_request, BadRequest(response_body), req.version()));
            }
        }
        else {
            send(MakeStringResponse(http::status::method_not_allowed, "{ Invalid Method }"sv, req.version()));
        }
    }

private:
    std::string_view MapsResponse(std::string& response_body) {
        std::vector<model::Map> maps = game_.GetMaps();
        for (const auto& map : maps) {
            response_body += R"([{"id": ")" + *map.GetId() + R"(", "name": ")" + map.GetName() + R"("})"; //мб надо будет добавить \n
        }

        response_body += "]";

        return std::string_view(response_body.data(), response_body.size());
    }

    std::string_view MapById(std::string& response_body, const std::string& map_id, model::Map::Id id) {
        const model::Map* this_map = game_.FindMap(id);

        response_body += "{\n  ";
        response_body += R"("id": ")" + *this_map->GetId() + R"(",)";
        response_body += +"\n  ";
        response_body += R"("name": ")" + this_map->GetName() + R"(",)";
        response_body += +"\n  ";
        response_body += R"("roads": [)";
        for (const auto& road : this_map->GetRoads()) {
            response_body += "\n    ";
            response_body += R"({ "x0": )" + std::to_string(road.GetStart().x) + R"(, )";
            response_body += R"("y0": )" + std::to_string(road.GetStart().y) + R"(, )";

            if (road.IsHorizontal()) {
                response_body += R"("x1": )" + std::to_string(road.GetEnd().x) + R"( },)";
            }
            else {
                response_body += R"("y1": )" + std::to_string(road.GetEnd().y) + R"( },)";
            }
        }

        response_body.pop_back();
        response_body += "\n  ],\n  ";

        response_body += R"("buildings": [)";

        if (!this_map->GetBuildings().empty()) {
            for (const auto& building : this_map->GetBuildings()) {
                response_body += "\n    ";
                response_body += R"({ "x": )" + std::to_string(building.GetBounds().position.x) + R"(, )";
                response_body += R"("y": )" + std::to_string(building.GetBounds().position.y) + R"(, )";
                response_body += R"("w": )" + std::to_string(building.GetBounds().size.width) + R"(, )";
                response_body += R"("h": )" + std::to_string(building.GetBounds().size.height) + R"( },)";
            }
            response_body.pop_back();
            response_body += "\n  ";
        }
        response_body += "],\n  ";

        response_body += R"("offices": [)";

        if (!this_map->GetOffices().empty()) {
            for (const auto& office : this_map->GetOffices()) {
                response_body += "\n    ";
                response_body += R"({ "id": ")" + *office.GetId() + R"(", )";
                response_body += R"("x": )" + std::to_string(office.GetPosition().x) + R"(, )";
                response_body += R"("y": )" + std::to_string(office.GetPosition().y) + R"(, )";
                response_body += R"("offsetX": )" + std::to_string(office.GetOffset().dx) + R"(, )";
                response_body += R"("offsetY": )" + std::to_string(office.GetOffset().dy) + R"( },)";
            }
            response_body.pop_back();
            response_body += "\n";
        }

        response_body += "  ]\n";
        response_body += R"(})";
        return std::string_view(response_body.data(), response_body.size());
    }

    std::string_view MapNotFound(std::string& response_body) {
        response_body += "{\n";
        response_body += R"("code": "mapNotFound",)";
        response_body += "\n";
        response_body += R"("message": "Map not found")";
        response_body += "\n}";

        return std::string_view(response_body.data(), response_body.size());
    }

    std::string_view BadRequest(std::string& response_body) {
        response_body += "{\n";
        response_body += R"("code": "badRequest",)";
        response_body += "\n";
        response_body += R"("message": "Bad request")";
        response_body += "\n}";

        return std::string_view(response_body.data(), response_body.size());
    }

    model::Game& game_;
};

}  // namespace http_handler
