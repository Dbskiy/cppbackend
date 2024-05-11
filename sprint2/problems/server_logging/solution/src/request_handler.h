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

namespace http_handler {
namespace beast = boost::beast;
namespace sys = boost::system;
namespace http = beast::http;
namespace fs = std::filesystem;

using namespace std::literals;

// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using response_body = std::variant<FileResponse, StringResponse>;

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


class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, const fs::path& content_root_path) 
        : game_{game},
          content_root_path_{content_root_path} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status, std::string_view, unsigned, std::string_view);
    FileResponse StaticFileResponseBody(fs::path&, std::string_view, unsigned);
    
    std::string_view DefineContentType(const std::string_view);

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {

        if (req.target().substr(0, 5) == "/api/") {
            StringResponse ApiResponse = MakeApiResponse(req.target(), req.version());

            logger::res_code = ApiResponse.result_int();
            logger::res_content_type = ApiResponse.find(http::field::content_type)->value();
            send(ApiResponse);
        }
        else {
            response_body response = MakeStaticFileResponse(req.target(), content_root_path_, req.version());
            if (std::holds_alternative<FileResponse>(response)) {
                FileResponse& f_res = std::get<FileResponse>(response);
                logger::res_code = f_res.result_int();
                logger::res_content_type = f_res.find(http::field::content_type)->value();
                send(f_res);
            } 
            else if (std::holds_alternative<StringResponse>(response)) {
                StringResponse& s_res = std::get<StringResponse>(response);
                logger::res_code = s_res.result_int();
                logger::res_content_type = s_res.find(http::field::content_type)->value();
                send(s_res);
            }
        }
    }

private:

    std::string_view MapsResponse(std::string&);

    std::string_view MapById(std::string&, const std::string&, model::Map::Id);

    std::string_view MapNotFound(std::string&);

    std::string_view BadRequest(std::string&);

    std::string_view InvalidMethod(std::string&);

    json::array RoadsProperties(const model::Map*);

    json::array BuildingsProperties(const model::Map*);

    json::array OfficesProperties(const model::Map*);

    StringResponse MakeApiResponse(const std::string_view, const unsigned);

    response_body MakeStaticFileResponse(const std::string_view, const fs::path&, const unsigned);

    model::Game& game_;

    fs::path content_root_path_;
};

}  // namespace http_handler
