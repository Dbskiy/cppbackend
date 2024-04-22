#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>
#include <filesystem>
#include <iostream>
#include "static_content.h"
#include <unordered_map>
#include <variant>

namespace http_handler {
namespace beast = boost::beast;
namespace sys = boost::system;
namespace http = beast::http;
namespace json = boost::json;
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

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game, fs::path content_root_path) //мб передать rvalue
        : game_{game},
          content_root_path_{std::move(content_root_path)} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    // Создаёт StringResponse с заданными параметрами
    StringResponse MakeStringResponse(http::status, std::string_view, unsigned,
        std::string_view);

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        //если начинается с api то send(MakeApiResponse)
        //иначе send(MakeStaticFileResponse) 
        //InvalidMethod?
        //if (req.target().substr(1) == "favicon.ico") return;

        if (req.target().substr(0, 5) == "/api/") {
            send(MakeApiResponse(req.target(), req.version()));
        }
        else {
            response_body response = MakeStaticFileResponse(req.target(), content_root_path_, req.version());
            if (std::holds_alternative<FileResponse>(response)) {
                FileResponse& f_res = std::get<FileResponse>(response);
                send(f_res);
            } 
            else if (std::holds_alternative<StringResponse>(response)) {
                StringResponse& s_res = std::get<StringResponse>(response);
                send(s_res);
            }

            //send(MakeStaticFileResponse(req.target(), content_root_path_, req.version()));
        }
    }

private:
    std::string_view DefineContentType(std::string_view);

    std::string_view MapsResponse(std::string&);

    std::string_view MapById(std::string&, const std::string&, model::Map::Id);

    std::string_view MapNotFound(std::string&);

    std::string_view BadRequest(std::string&);

    std::string_view InvalidMethod(std::string&);

    json::array RoadsProperties(const model::Map*);

    json::array BuildingsProperties(const model::Map*);

    json::array OfficesProperties(const model::Map*);

    StringResponse MakeApiResponse(std::string_view, unsigned);

    response_body MakeStaticFileResponse(std::string_view, fs::path&, unsigned);

    model::Game& game_;

    fs::path content_root_path_;
};

}  // namespace http_handler
