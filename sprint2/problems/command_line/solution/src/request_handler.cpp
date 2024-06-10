#include "request_handler.h"


namespace http_handler {
	StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version, std::string_view content_type)
	{
		StringResponse response(status, http_version);
		response.set(http::field::content_type, content_type);
		response.body() = body;
		response.content_length(body.size());
		return response;

	}

    FileResponse StaticFileResponseBody(fs::path& static_file_path, std::string_view content_type, unsigned http_version)
    {
        using namespace http;
        file_body::value_type file;
        if (sys::error_code ec; file.open(static_file_path.string().c_str(), beast::file_mode::read, ec), ec) {
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

    std::string ErrorResponse(const std::string& code, const std::string& message)
    {
        json::object notFound;

        notFound["code"] = code;
        notFound["message"] = message;

        std::string response_body = json::serialize(notFound);

        return response_body;
    }

    std::string_view DefineContentType(const std::string_view file_path)
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

    StringResponse ApiHandler::MakeApiResponse(const StringRequest& req)
    {

        std::string_view config_file_path = req.target();

        unsigned http_version = req.version();

        std::string_view content_type = DefineContentType(config_file_path);

        StringResponse response;

        if (config_file_path.find("/api/v1/maps"sv) != std::string::npos) {
            if (config_file_path.size() <= 13) {
                response = MakeStringResponse(http::status::ok, MapsResponse(), req.version(), content_type);
            }
            else {
                std::string map_id(config_file_path.substr(13));
                model::Map::Id id(map_id.c_str());

                if (App_.game_.FindMap(id) != nullptr) {
                    response = MakeStringResponse(http::status::ok, MapById(map_id, id), req.version(), content_type);
                }
                else {
                    response = MakeStringResponse(http::status::not_found, ErrorResponse("mapNotFound", "Map not found"), req.version(), content_type);
                }
            }
        }
        else if (config_file_path.find("/api/v1/game"sv) != std::string::npos) {

            std::string_view content_type = req.find(http::field::content_type)->value();

            if (config_file_path.find("join") != std::string::npos) {
                response = JoinGame(req, content_type);
            }
            else if (config_file_path.find("players") != std::string::npos) {
                response = GetPlayers(req, content_type);
            }
            else if (config_file_path.find("state") != std::string::npos) {
                response = GetState(req, content_type);
            }
            else if (config_file_path.find("player") != std::string::npos) {
                if (config_file_path.find("action") != std::string::npos) {
                    response = PlayerAction(req, content_type);
                }
            }
            else if (config_file_path.find("tick") != std::string::npos && tick_period_) {
                if (tick_period_ == 0) {
                    response = TimeControl(req, content_type);
                }
                else {
                    response = MakeStringResponse(http::status::bad_request, ErrorResponse("badRequest", "Invalid endpoint"), http_version, content_type);
                }
            }

        }

        response.set(http::field::cache_control, "no-cache"sv);
        return response;
    }

    std::string ApiHandler::MapsResponse()
	{
		json::array maps; 

		std::vector<model::Map> maps_vector = App_.game_.GetMaps(); //все доступные карты 

		for (const auto& map : maps_vector) {
			json::object map_properties; //объект, содержащий свойства одной карты

			map_properties["id"] = *map.GetId();
			map_properties["name"] = map.GetName();

			maps.push_back(map_properties);
		}

		std::string response_body = json::serialize(maps);

		return response_body;
	}

	std::string ApiHandler::MapById(const std::string& map_id, model::Map::Id id)
	{
        const model::Map* this_map = App_.game_.FindMap(id);

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

        std::string response_body = json::serialize(map);

        return response_body;
	}

    json::array ApiHandler::RoadsProperties(const model::Map* this_map)
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

    json::array ApiHandler::BuildingsProperties(const model::Map* this_map)
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

    json::array ApiHandler::OfficesProperties(const model::Map* this_map)
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

    StringResponse ApiHandler::MakeUnauthorizedError(const unsigned http_version, const std::string_view content_type, app::AuthorizationErrorReason reason)
    {
        std::string code;
        std::string message;
        if (reason == app::AuthorizationErrorReason::InvalidHeader) {
            code = "invalidToken";
            message = "Authorization header is missing";
        }
        else if (reason == app::AuthorizationErrorReason::TokenNotFound) {
            code = "unknownToken";
            message = "Player token has not been found";
        }
        return MakeStringResponse(http::status::unauthorized, ErrorResponse(code, message), http_version, content_type);
    }

    std::optional<app::Token> ApiHandler::TryExtractToken(const StringRequest& req)
    {
        auto it = req.find(http::field::authorization);

        if (it == req.end()) return std::nullopt;

        auto authHeader = it->value();

        std::string_view substr = "Bearer ";
        size_t pos = authHeader.find(substr);

        if (pos != std::string_view::npos && authHeader.size() - substr.size() == 32) {
            std::string token(authHeader.substr(pos + substr.size()));
            return app::Token(token);
        }
        return std::nullopt;
    }

    StringResponse ApiHandler::JoinGame(const StringRequest& req, const std::string_view content_type)
    {
        if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::POST, req.version(), content_type)) {
            return *invalid_method;
        }
        else {
            json::error_code ec;

            auto json_body = json::parse(req.body(), ec);

            //ошибка при парсинге json или при получении его свойств
            if (ec || !json_body.as_object().contains("userName") || !json_body.as_object().contains("mapId")) {
                return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Join game request parse error"),
                    req.version(), content_type);
            }

            try {
                std::string user_name = json_body.at("userName").as_string().c_str();
                std::string map_id = json_body.at("mapId").as_string().c_str();

                json::object join_res_body;

                auto [token, playerId] = App_.JoinGame(map_id, user_name, randomize_spawn_);

                join_res_body["authToken"] = *token;

                join_res_body["playerId"] = *playerId;

                return MakeStringResponse(http::status::ok, json::serialize(join_res_body), req.version(), content_type);

            }
            catch (const app::JoinGameError& e) {
                if (e.GetReason() == app::JoinGameErrorReason::InvalidName) {
                    return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Invalid name"), req.version(), content_type);
                }
                else if (e.GetReason() == app::JoinGameErrorReason::InvalidMap) {
                    return MakeStringResponse(http::status::not_found, ErrorResponse("mapNotFound", "Map Not Found"), req.version(), content_type);
                }
            }
        }
    }

    StringResponse ApiHandler::GetPlayers(const StringRequest& req, const std::string_view content_type)
    {
        return ExecuteAuthorized([&](const app::Token& token) {
            if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::GET_HEAD, req.version(), content_type)) {
                return *invalid_method;
            }

            auto list_players = App_.GetPlayersList(token);

            if (list_players) {
                json::object res_body;

                for (const auto& pair : list_players->list_players_) {
                    json::object name_body;

                    name_body["name"] = pair.second;

                    res_body[std::to_string(pair.first)] = name_body;
                }
                return MakeStringResponse(http::status::ok, json::serialize(res_body), req.version(), content_type);
            }
            else {
                return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::TokenNotFound);
            }
            }, req, content_type);
    }

    StringResponse ApiHandler::PlayerAction(const StringRequest& req, const std::string_view content_type)
    {
        return ExecuteAuthorized([&](const app::Token& token) {
            if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::POST, req.version(), content_type)) {
                return *invalid_method;
            }

            if (content_type != ContentType::APP_JSON) {
                return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Invalid content type"), 
                    req.version(), content_type);
            }

            json::error_code ec;

            auto json_body = json::parse(req.body(), ec);

            //ошибка при парсинге json или при получении его свойств
            if (ec || !json_body.as_object().contains("move")) {
                return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Failed to parse action"),
                    req.version(), content_type);
            }

            std::string move = json_body.as_object().at("move").as_string().c_str();

            if (move != "L" && move != "R" && move != "U" && move != "D" && move != "") {
                return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Failed to parse action"),
                    req.version(), content_type);
            }

            if (auto player_action_res = App_.MovePlayer(token, move)) {
                return MakeStringResponse(http::status::ok, *player_action_res, req.version(), content_type);
            }
            else {
                return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::TokenNotFound);
            }



        }, req, content_type);
    }

    StringResponse ApiHandler::GetState(const StringRequest& req, const std::string_view content_type)
    {
        return ExecuteAuthorized([&](const app::Token& token) {
            if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::GET_HEAD, req.version(), content_type)) {
                return *invalid_method;
            }

            auto players_state = App_.GetGameState(token);

            if (players_state) {
                json::object res_body;
                json::object players_json;

                for (const auto& pair : players_state->GetPlayersState()) {

                    json::object stats;
                    json::array position;
                    position.push_back(pair.second.position.x);
                    position.push_back(pair.second.position.y);

                    json::array speed;
                    speed.push_back(pair.second.speed.Vx);
                    speed.push_back(pair.second.speed.Vy);

                    stats["pos"] = position;
                    stats["speed"] = speed;
                    stats["dir"] = pair.second.direction;

                    players_json[std::to_string(pair.first)] = stats;
                }
                res_body["players"] = players_json;
                return MakeStringResponse(http::status::ok, json::serialize(res_body), req.version(), content_type);
            }
            else {
                return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::TokenNotFound);
            }
            }, req, content_type);
    }

    StringResponse ApiHandler::TimeControl(const StringRequest& req, const std::string_view content_type)
    {
        if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::POST, req.version(), content_type)) {
            return *invalid_method;
        }
        json::error_code ec;

        auto json_body = json::parse(req.body(), ec);

        if (ec || !json_body.as_object().contains("timeDelta")) {
            return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Failed to parse tick request JSON"),
                req.version(), content_type);
        }
        
        auto it = json_body.as_object().find("timeDelta");
        if (it == json_body.as_object().end() || it->value().is_bool() || it->value().is_double() || it->value().is_string() || it->value().is_null()) {
            return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Failed to parse tick request JSON"),
                req.version(), content_type);
        }
        tick_period_ = it->value().as_int64();

        App_.Tick(tick_period_);

        return MakeStringResponse(http::status::ok, "{}", req.version(), content_type);
    }

    response_body RequestHandler::MakeStaticFileResponse(const std::string_view static_file_path, const fs::path& root_path, const unsigned http_version)
    {
        using namespace http;
        using namespace static_content;

        std::string_view static_f_path = ToValidStaticFilePath(static_file_path);

        fs::path rel_fs_path = URL_decode(static_f_path);

        fs::path abs_fs_path = fs::weakly_canonical(rel_fs_path);

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

            //если файл существует
            if (fs::exists(new_fs_path)) {
                return StaticFileResponseBody(new_fs_path, static_f_path, http_version);
            }

            //файл не существует
            return MakeStringResponse(status::not_found, "File Not Found"sv, http_version, ContentType::TEXT_PLAIN);

        }
    }

    void RequestHandler::UpdateLogger(int res_code, std::string_view res_content_type)
    {
        // Логируем ответ внутри защищенного контекста strand
        logger::res_code = res_code;
        logger::res_content_type = res_content_type;
    }
    

}  // namespace http_handler
