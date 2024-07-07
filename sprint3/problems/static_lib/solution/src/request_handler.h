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
#include "logger.h"
#include "loot_extra_data.h"
#include "loot_generator.h"
#include "app.h"

namespace http_handler {
namespace beast = boost::beast;
namespace sys = boost::system;
namespace http = beast::http;
namespace fs = std::filesystem;
using tcp = net::ip::tcp;

using namespace std::literals;

// Ответ, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using response_body = std::variant<FileResponse, StringResponse>;


// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status, std::string_view, unsigned, std::string_view);
FileResponse StaticFileResponseBody(fs::path&, std::string_view, unsigned);

std::string ErrorResponse(const std::string&, const std::string&);
    
std::string_view DefineContentType(const std::string_view);

struct Allow {
    Allow() = delete;
    constexpr static std::string_view POST = "POST"sv;
    constexpr static std::string_view GET_HEAD = "GET, HEAD"sv;

    static std::optional<StringResponse> MethodNotAllowed(const http::verb method, auto expected, const unsigned http_version, const std::string_view content_type) {
        if (expected == POST) {
            if (method != http::verb::post) {
                StringResponse invalidMethod = MakeStringResponse(http::status::method_not_allowed, ErrorResponse("invalidMethod", "Invalid method"),
                    http_version, content_type);
                invalidMethod.set(http::field::allow, Allow::POST);
                return invalidMethod;
            }
        }
        else if (expected == GET_HEAD) {
            if (method != http::verb::get && method != http::verb::head) {
                StringResponse invalidMethod = MakeStringResponse(http::status::method_not_allowed, ErrorResponse("invalidMethod", "Invalid method"),
                    http_version, content_type);
                invalidMethod.set(http::field::allow, Allow::GET_HEAD);
                return invalidMethod;
            }
        }
        return std::nullopt;
    }

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


class ApiHandler
{
public:
    explicit ApiHandler(app::Application& App, int tick_period, const bool randomize_spawn) 
        : tick_period_(tick_period),
          randomize_spawn_(randomize_spawn),
          App_{App} {}

    StringResponse MakeApiResponse(const StringRequest& req);

private:
    int tick_period_;
    bool randomize_spawn_;

    app::Application App_;

    StringResponse MapsResponse(const StringRequest& req, const std::string_view content_type);

    StringResponse MapById(const StringRequest& req, const std::string_view content_type, model::Map::Id);

    json::array RoadsProperties(const model::Map*); 

    json::array BuildingsProperties(const model::Map*);

    json::array OfficesProperties(const model::Map*);

    StringResponse MakeUnauthorizedError(const unsigned, const std::string_view, app::AuthorizationErrorReason);

    std::optional<app::Token> TryExtractToken(const StringRequest& req);

    template <typename Fn>
    StringResponse ExecuteAuthorized(Fn&& action, const StringRequest& req, const std::string_view content_type) {
        if (auto token = TryExtractToken(req)) {
            return action(*token);
        }
        else {
            return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::InvalidHeader);
        }
    }

    StringResponse JoinGame(const StringRequest& req, const std::string_view content_type);
    
    StringResponse GetPlayers(const StringRequest&, const std::string_view);
    
    StringResponse PlayerAction(const StringRequest&, const std::string_view);

    StringResponse GetState(const StringRequest&, const std::string_view);

    StringResponse TimeControl(const StringRequest&, const std::string_view);

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

    explicit RequestHandler(Strand api_strand, app::Application& App, const fs::path& content_root_path, int tick_period, const bool randomize_spawn) 
        : api_strand_{api_strand},
          content_root_path_{content_root_path},
          api_handler_{App, tick_period, randomize_spawn} {}

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

                        return send(ApiResponse);
                    }
                    catch (...) {
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
        }
    }

private:

    response_body MakeStaticFileResponse(const std::string_view, const fs::path&, const unsigned);

    void UpdateLogger(int, std::string_view);

    Strand api_strand_;

    

    ApiHandler api_handler_;

    fs::path content_root_path_;
};

}  // namespace http_handler
