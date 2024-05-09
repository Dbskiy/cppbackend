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

    FileResponse RequestHandler::StaticFileResponseBody(fs::path& static_file_path, std::string_view content_type, unsigned http_version)
    {
        using namespace http;
        file_body::value_type file;
        if (sys::error_code ec; file.open(static_file_path.string().c_str(), beast::file_mode::read, ec), ec) {
            std::cout << "Failed to open file "sv << static_file_path.string() << std::endl;

            throw ec.what();
        }

        FileResponse res;
        res.version(http_version);
        res.result(status::ok);
        res.insert(field::content_type, DefineContentType(content_type));

        res.body() = std::move(file);
        res.prepare_payload();
        return res;
    }

    std::string_view RequestHandler::DefineContentType(const std::string_view file_path)
    {
        std::unordered_map<std::string_view, std::string_view> content_types{
            {"json", ContentType::APP_JSON},
            {"xml", ContentType::APP_XML},
            {"htm", ContentType::TEXT_HTML},
            {"html", ContentType::TEXT_HTML},
            {"css", ContentType::TEXT_CSS},
            {"txt", ContentType::TEXT_PLAIN},
            {"js", ContentType::TEXT_JS},
            {"png", ContentType::IMAGE_PNG},
            {"jpg", ContentType::IMAGE_JPEG},
            {"jpe", ContentType::IMAGE_JPEG},
            {"jpeg", ContentType::IMAGE_JPEG},
            {"gif", ContentType::IMAGE_GIF},
            {"bmp", ContentType::IMAGE_BMP},
            {"ico", ContentType::IMAGE_ICO},
            {"tiff", ContentType::IMAGE_TIFF},
            {"tif", ContentType::IMAGE_TIFF},
            {"svg", ContentType::IMAGE_SVG},
            {"svgz", ContentType::IMAGE_SVG},
            {"mpeg", ContentType::AUDIO_MP3},
        };

        if (file_path.find("api") != std::string::npos) return ContentType::APP_JSON;

        size_t index = file_path.find_last_of(".");
        if (index == std::string::npos) {
            // Если расширение файла не найдено, возвращаем текстовый тип по умолчанию
            return ContentType::APP_OCTET_STREAM;
        }

        std::string_view extension = file_path.substr(index + 1);

        return content_types[extension];

    }

    std::string_view RequestHandler::getResponseContentType() {
        return res_content_type_;
    }

    int RequestHandler::getResponseCode() {
        return res_code_;
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

        map["roads"] = RoadsProperties(this_map);

        if (!this_map->GetBuildings().empty()) {
            map["buildings"] = BuildingsProperties(this_map);
        }

        if (!this_map->GetOffices().empty()) {
            map["offices"] = OfficesProperties(this_map);
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

    std::string_view RequestHandler::InvalidMethod(std::string& response_body)
    {
        json::string invalidMethod("Invalid Method");

        response_body = json::serialize(invalidMethod);

        return std::string_view(response_body.data(), response_body.size());
    }

    json::array RequestHandler::RoadsProperties(const model::Map* this_map)
    {
        json::array roads;
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
        return roads;
    }

    json::array RequestHandler::BuildingsProperties(const model::Map* this_map)
    {
        json::array buildings;
        for (const auto& building : this_map->GetBuildings()) {
            json::object properties;

            properties["x"] = building.GetBounds().position.x;
            properties["y"] = building.GetBounds().position.y;

            properties["w"] = building.GetBounds().size.width;
            properties["h"] = building.GetBounds().size.height;

            buildings.push_back(properties);
        }
        return buildings;
    }

    json::array RequestHandler::OfficesProperties(const model::Map* this_map)
    {
        json::array offices;

        for (const auto& office : this_map->GetOffices()) {
            json::object properties;

            properties["id"] = *office.GetId();

            properties["x"] = office.GetPosition().x;
            properties["y"] = office.GetPosition().y;

            properties["offsetX"] = office.GetOffset().dx;
            properties["offsetY"] = office.GetOffset().dy;

            offices.push_back(properties);
        }
        return offices;
    }

    StringResponse RequestHandler::MakeApiResponse(const std::string_view config_file_path, const unsigned http_version)
    {
        std::string response_body = "";

        std::string_view content_type = DefineContentType(config_file_path);

        if (config_file_path == "/api/v1/maps"sv || config_file_path == "/api/v1/maps/"sv) {
            return MakeStringResponse(http::status::ok, MapsResponse(response_body), http_version, content_type);
        }
        else if (config_file_path.substr(0, 13) == "/api/v1/maps/"sv && config_file_path.size() > 13) {
            std::string map_id(config_file_path.substr(13));
            model::Map::Id id(map_id.c_str());

            if (game_.FindMap(id) != nullptr) {
                return MakeStringResponse(http::status::ok, MapById(response_body, map_id, id), http_version, content_type);
            }
            else {
                return MakeStringResponse(http::status::not_found, MapNotFound(response_body), http_version, content_type);
            }
        }
        else {
            return MakeStringResponse(http::status::bad_request, BadRequest(response_body), http_version, content_type);
        }
    }

    response_body RequestHandler::MakeStaticFileResponse(const std::string_view static_file_path, const fs::path& root_path, const unsigned http_version)
    {
        using namespace http;
        using namespace static_content;

        std::string_view static_f_path = ToValidStaticFilePath(static_file_path);

        fs::path rel_fs_path = URL_decode(static_f_path);

        fs::path abs_fs_path = fs::weakly_canonical(rel_fs_path);

        //std::cout << "abs_fs_path "sv << abs_fs_path.string() << std::endl; ////для отладки

        if (fs::exists(abs_fs_path)) {
            if (IsSubPath(abs_fs_path, root_path)) {
                return StaticFileResponseBody(abs_fs_path, static_f_path, http_version);
            }
            else {
                //файл вне корневого каталога со статическими файлами
                return MakeStringResponse(status::bad_request, "Bad Request: trying to get inaccessible file!"sv, http_version, ContentType::TEXT_PLAIN);
            }
        }
        else {
            if (IsSubPath(abs_fs_path, root_path)) {
                //файл не существует
                return MakeStringResponse(status::not_found, "File Not Found"sv, http_version, ContentType::TEXT_PLAIN);
            }

            //иначе создаем новый путь, добавив в начало путь к корневой директории со статическими файлами
            fs::path new_fs_path = fs::weakly_canonical(root_path / rel_fs_path);
            //std::cout << "new_fs_path "sv << new_fs_path.string() << std::endl; ////для отладки

            //если файл существует
            if (fs::exists(new_fs_path)) {
                return StaticFileResponseBody(new_fs_path, static_f_path, http_version);
            }

            //файл не существует
            return MakeStringResponse(status::not_found, "File Not Found"sv, http_version, ContentType::TEXT_PLAIN);

        }
    }
    

}  // namespace http_handler
