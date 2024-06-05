#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include "http_server.h"
#include "model.h"
#include <filesystem>
#include <iostream>
#include "static_content.h"
#include <unordered_map>
#include <variant>
//#include <chrono>
#include "logger.h"
#include "app.h"

namespace http_handler {
namespace beast = boost::beast;
namespace sys = boost::system;
namespace http = beast::http;
namespace fs = std::filesystem;
using tcp = net::ip::tcp;

using namespace std::literals;

// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using response_body = std::variant<FileResponse, StringResponse>;

struct Allow {
    Allow() = delete;
    constexpr static std::string_view ALLOW_POST = "POST"sv;
    constexpr static std::string_view ALLOW_GET_HEAD = "GET, HEAD"sv;

};

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view APP_JSON = "application/json"sv;
    constexpr static std::string_view APP_XML = "application/xml"sv;
    constexpr static std::string_view APP_OCTET_STREAM = "application/octet-stream"sv;

    constexpr static std::string_view TEXT_HTML = "text/html"sv;
    constexpr static std::string_view TEXT_CSS = "text/css"sv;
    constexpr static std::string_view TEXT_PLAIN = "text/plain"sv;
    constexpr static std::string_view TEXT_JS = "text/javascript"sv;
    
    constexpr static std::string_view IMAGE_PNG = "image/png"sv;
    constexpr static std::string_view IMAGE_JPEG = "image/jpeg"sv;
    constexpr static std::string_view IMAGE_GIF = "image/gif"sv;
    constexpr static std::string_view IMAGE_BMP = "image/bmp"sv;
    constexpr static std::string_view IMAGE_ICO = "image/vnd.microsoft.icon"sv;
    constexpr static std::string_view IMAGE_TIFF = "image/tiff"sv;
    constexpr static std::string_view IMAGE_SVG = "image/svg+xml"sv;


    constexpr static std::string_view AUDIO_MP3 = "audio/mpeg"sv;
};

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status, std::string_view, unsigned, std::string_view);
    FileResponse StaticFileResponseBody(fs::path&, std::string_view, unsigned);
    
    std::string_view DefineContentType(const std::string_view);

class ApiHandler
{
public:
    explicit ApiHandler(model::Game& game, app::Players& players) : game_{ game }, players_{players} {}

    //StringResponse MakeApiResponse(const std::string_view, const unsigned);
    template <typename Body, typename Allocator>
    StringResponse MakeApiResponse(const http::request<Body, http::basic_fields<Allocator>>& req) {

        std::string_view config_file_path = req.target();

        unsigned http_version = req.version();

        std::string_view content_type = DefineContentType(config_file_path);

        std::string_view sub_path = config_file_path.substr(0, 12);

        StringResponse response;

        if (sub_path == "/api/v1/maps"sv) {
            if (config_file_path.size() <= 13) {
                response = MakeStringResponse(http::status::ok, MapsResponse(), http_version, content_type);
            }
            else {
                std::string map_id(config_file_path.substr(13));
                model::Map::Id id(map_id.c_str());

                if (game_.FindMap(id) != nullptr) {
                    response = MakeStringResponse(http::status::ok, MapById(map_id, id), http_version, content_type);
                }
                else {
                    response = MakeStringResponse(http::status::not_found, ErrorResponse("mapNotFound", "Map not found"), http_version, content_type);
                }
            }
        }
        else if (sub_path == "/api/v1/game"sv) {

            std::string_view content_type = req.find(http::field::content_type)->value();

            if (config_file_path.find("join") != std::string::npos) {
                response = JoinGame(req, content_type, http_version);
            }
            else if (config_file_path.find("players") != std::string::npos) {
                response = GetPlayers(req, content_type, http_version);
            }
            else if (config_file_path.find("state") != std::string::npos) {
                response = GetState(req, content_type, http_version);
            }

            response.set(http::field::cache_control, "no-cache"sv);
        }
        else {
            response = MakeStringResponse(http::status::bad_request, ErrorResponse("badRequest", "Bad request"), http_version, content_type);
        }
        return response;
    }

private:
    model::Game& game_;

    app::Players& players_;

    template <typename Body, typename Allocator>
    StringResponse JoinGame(const http::request<Body, http::basic_fields<Allocator>>& req, const std::string_view content_type, const unsigned http_version) {
        if (req.method() != http::verb::post) {
            StringResponse invalidMethod = MakeStringResponse(http::status::method_not_allowed, ErrorResponse("invalidMethod", "Invalid method"),
                http_version, content_type);
            invalidMethod.set(http::field::allow, Allow::ALLOW_POST);
            return invalidMethod;
        }
        else {
            json::error_code ec;

            auto json_body = json::parse(req.body(), ec);

            //ошибка при парсинге json или при получении его свойств
            if (ec || !json_body.as_object().contains("userName") || !json_body.as_object().contains("mapId")) {
                return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Join game request parse error"), 
                    http_version, content_type);
            }

            try {
                std::string user_name = json_body.at("userName").as_string().c_str();
                std::string map_id = json_body.at("mapId").as_string().c_str();

                json::object join_res_body;

                auto [token, playerId] = app::Application{ game_, players_ }.JoinGame(map_id, user_name);

                join_res_body["authToken"] = *token;

                join_res_body["playerId"] = *playerId;

                return MakeStringResponse(http::status::ok, json::serialize(join_res_body), http_version, content_type);

            }
            catch (const app::JoinGameError& e) {
                if (e.GetReason() == app::JoinGameErrorReason::InvalidName) {
                    return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Invalid name"), http_version, content_type);
                }
                else if (e.GetReason() == app::JoinGameErrorReason::InvalidMap) {
                    return MakeStringResponse(http::status::not_found, ErrorResponse("mapNotFound", "Map Not Found"), http_version, content_type);
                }
            }
        }
    }

    template <typename Body, typename Allocator>
    StringResponse GetPlayers(const http::request<Body, http::basic_fields<Allocator>>& req, const std::string_view content_type, const unsigned http_version) {
        //Если http-метод не соответствует корректному
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            StringResponse invalidMethod = MakeStringResponse(http::status::method_not_allowed, ErrorResponse("invalidMethod", "Invalid method"),
                http_version, content_type);
            invalidMethod.set(http::field::allow, Allow::ALLOW_GET_HEAD);
            return invalidMethod;
        }
        
        try {
            auto authHeader = req.find(http::field::authorization)->value();
            auto list_players = app::Application{ game_, players_ }.GetPlayersList(authHeader);

            json::object res_body;

            for (const auto& pair : list_players.list_players_) {
                json::object name_body;

                name_body["name"] = pair.second;

                res_body[std::to_string(pair.first)] = name_body;
            }

            return MakeStringResponse(http::status::ok, json::serialize(res_body), http_version, content_type);

        }
        catch (const app::ListPlayersError& e) {
            if (e.GetReason() == app::ListPlayersErrorReason::InvalidHeader) {
                return MakeStringResponse(http::status::unauthorized, ErrorResponse("invalidToken", "Authorization header is missing"),
                    http_version, content_type);
            }
            else if (e.GetReason() == app::ListPlayersErrorReason::TokenNotFound) {
                return MakeStringResponse(http::status::unauthorized, ErrorResponse("unknownToken", "Player token has not been found"), 
                    http_version, content_type);
            }
        }
       
    }

    template <typename Body, typename Allocator>
    StringResponse GetState(const http::request<Body, http::basic_fields<Allocator>>& req, const std::string_view content_type, const unsigned http_version) {
        if (req.method() != http::verb::get && req.method() != http::verb::head) {
            StringResponse invalidMethod = MakeStringResponse(http::status::method_not_allowed, ErrorResponse("invalidMethod", "Invalid method"),
                http_version, content_type);
            invalidMethod.set(http::field::allow, Allow::ALLOW_GET_HEAD);
            return invalidMethod;
        }

        try {
            auto authHeader = req.find(http::field::authorization)->value();
            auto players_state = app::Application{ game_, players_ }.GetGameState(authHeader);

            json::object res_body;
            json::object players_json;

            for (const auto& pair : players_state.GetPlayersState()) {

                json::object stats;
                json::array position;
                position.push_back(pair.second.position[0]);
                position.push_back(pair.second.position[1]);

                json::array speed;
                speed.push_back(pair.second.speed[0]);
                speed.push_back(pair.second.speed[1]);

                stats["pos"] = position;
                stats["speed"] = speed;
                stats["dir"] = pair.second.direction;

                players_json[std::to_string(pair.first)] = stats;
            }
            res_body["players"] = players_json;

            return MakeStringResponse(http::status::ok, json::serialize(res_body), http_version, content_type);
        }
        catch (const app::GameStateError& e) {
            if (e.GetReason() == app::GameStateErrorReason::InvalidHeader) {
                return MakeStringResponse(http::status::unauthorized, ErrorResponse("invalidToken", "Authorization header is missing"),
                    http_version, content_type);
            }
            else if (e.GetReason() == app::GameStateErrorReason::TokenNotFound) {
                return MakeStringResponse(http::status::unauthorized, ErrorResponse("unknownToken", "Player token has not been found"),
                    http_version, content_type);
            }
        }
    }



    std::string MapsResponse();

    std::string MapById(const std::string&, model::Map::Id);

    std::string ErrorResponse(const std::string&, const std::string&);

    json::array RoadsProperties(const model::Map*);

    json::array BuildingsProperties(const model::Map*);

    json::array OfficesProperties(const model::Map*);

};

template<class SomeRequestHandler>
class LoggingRequestHandler {
public:
    LoggingRequestHandler(SomeRequestHandler& decorated) : decorated_(decorated) {}

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        //логируем запрос
        LogRequest(req);

        system_clock::time_point start_ts = system_clock::now();

        decorated_(std::move(req), std::forward<Send>(send));

        system_clock::time_point end_ts = system_clock::now();

        auto duration = duration_cast<milliseconds>(end_ts - start_ts);

        LogResponse(duration);
    }

private:
    template <typename Body, typename Allocator>
    static void LogRequest(const http::request<Body, http::basic_fields<Allocator>>& req) {
        //обработка запроса
        logger::LOG(logger::GetReqData(logger::client_ip, req.target(), req.method()), "request received"sv);

    }
    
    static void LogResponse(const milliseconds& duration) {
        logger::LOG(logger::GetResData(duration, logger::res_code, logger::res_content_type), "response sent"sv);
    }

    SomeRequestHandler& decorated_;
};


class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;

    explicit RequestHandler(Strand api_strand, model::Game& game, const fs::path& content_root_path, app::Players& players) 
        : api_strand_{api_strand},
          game_{game},
          content_root_path_{content_root_path},
          api_handler_{game_, players} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

        auto keep_alive = req.keep_alive();
        auto version = req.version();

        try {
            if (req.target().substr(0, 5) == "/api/") {
                auto handle = [self = shared_from_this(), send,
                    req = std::forward<decltype(req)>(req), version, keep_alive] {
                    try {
                        assert(self->api_strand_.running_in_this_thread());

                        StringResponse ApiResponse = self->api_handler_.MakeApiResponse(req);

                        self->UpdateLogger(ApiResponse.result_int(), ApiResponse.find(http::field::content_type)->value());

                        //logger::res_code = ApiResponse.result_int(); //мб гонка
                        //logger::res_content_type = ApiResponse.find(http::field::content_type)->value(); //мб гонка
                        return send(ApiResponse);
                    }
                    catch (...) {
                        //do smth
                    }

                    };
                return net::dispatch(api_strand_, handle);

            }
            else {
                response_body response = MakeStaticFileResponse(req.target(), content_root_path_, version);

                std::visit(
                    [self = shared_from_this(), &send](auto&& res) {
                        self->UpdateLogger(res.result_int(), res.find(http::field::content_type)->value());
                        send(std::forward<decltype(res)>(res));
                    },
                    response
                );
            }
        }
        catch (...) {
            //do smth
        }
    }

private:

    response_body MakeStaticFileResponse(const std::string_view, const fs::path&, const unsigned);

    void UpdateLogger(int, std::string_view);

    Strand api_strand_;

    model::Game& game_;

    ApiHandler api_handler_;

    fs::path content_root_path_;
};

}  // namespace http_handler
