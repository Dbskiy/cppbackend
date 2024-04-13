#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>


namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace json = boost::json;

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
    StringResponse MakeStringResponse(http::status, std::string_view, unsigned,
        std::string_view content_type = ContentType::APP_JSON);

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        std::string response_body = "";
        if (req.method() == http::verb::get) {


            if (req.target() == "/api/v1/maps"s || req.target() == "/api/v1/maps/"s) {
                send(MakeStringResponse(http::status::ok, MapsResponse(response_body), req.version()));
            }
            else if (req.target().substr(0, 13) == "/api/v1/maps/" && req.target().size() > 13) {
                std::string map_id(req.target().substr(13));
                model::Map::Id id(map_id.c_str());

                if (game_.FindMap(id) != nullptr) {
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
            send(MakeStringResponse(http::status::method_not_allowed, InvalidMethod(response_body), req.version()));
        }
    }

private:
    std::string_view MapsResponse(std::string&);

    std::string_view MapById(std::string&, const std::string&, model::Map::Id);

    std::string_view MapNotFound(std::string&);

    std::string_view BadRequest(std::string&);

    std::string_view InvalidMethod(std::string&);

    model::Game& game_;
};

}  // namespace http_handler
