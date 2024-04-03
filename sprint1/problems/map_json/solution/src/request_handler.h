#pragma once
#include "http_server.h"
#include "model.h"


namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;

using namespace std::literals;

// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
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
        //const auto text_response = [&req](http::status status, std::string_view text) {
        //    MakeStringResponse(status, text, req.version()); //написать функцию?
        //    };


        if (req.method() == http::verb::get) {
            if (req.target() == "/api/v1/maps"s) {
                std::vector<model::Map> maps = game_.GetMaps();
                std::string str_maps = "";
                for (int i = 0; i < maps.size() ; i++) {
                    str_maps += R"([{"id": )" + *maps[i].GetId() + R"("name": )" + maps[i].GetName() + R"(}])" + '\n';
                }
                send(MakeStringResponse(http::status::ok, std::string_view(str_maps.data(), str_maps.size()), req.version())); //функция для отправки ответа
            }
            else if (req.target().substr(0, 13) == "/api/v1/maps/") {
                std::string map_id_str(req.target().substr(13));
                model::Map::Id map_id(map_id_str.c_str());
                if (game_.FindMap(map_id) != nullptr) {
                    //send(std::move(text_response(http::status::ok, "{MAKAKA}"sv)));
                    send(MakeStringResponse(http::status::ok, "{MAKAKA}"sv, req.version()));
                }
                else {
                    std::string_view mapNotFound = R"({
"code": "mapNotFound",
"message": "Map not found"
})";
                    //send(std::move(text_response(http::status::not_found, mapNotFound)));
                    send(MakeStringResponse(http::status::not_found, mapNotFound, req.version()));
                }
            }
            else {
                std::string_view bad_request = R"({
"code": "badRequest",
"message": "Bad request"
})";
                //send(std::move(text_response(http::status::bad_request, bad_request)));
                send(MakeStringResponse(http::status::bad_request, bad_request, req.version()));
            }
        }
        else {
            //send(std::move(text_response(http::status::method_not_allowed, "Invalid method"sv)));
            send(MakeStringResponse(http::status::method_not_allowed, "Invalid method"sv, req.version()));
        }
        //send(response);
    }

private:
    model::Game& game_;
};

}  // namespace http_handler
