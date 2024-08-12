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

    std::optional<int> ExtractIntParameter(std::string_view url, std::string_view param)
    {
        auto pos = url.find(param);
        if (pos != std::string_view::npos) {
            pos += param.size();
            auto end_pos = url.find('&', pos);
            std::string_view value_str = url.substr(pos, end_pos - pos);
            try {
                return std::stoi(std::string(value_str));
            }
            catch (...) {
            }
        }
        return std::nullopt;
    }

    StringResponse ApiHandler::MakeApiResponse(const StringRequest& req)
    {

        std::string_view config_file_path = req.target();

        unsigned http_version = req.version();

        std::string_view content_type = DefineContentType(config_file_path);

        StringResponse response;

        if (config_file_path.find("/api/v1/maps") != std::string::npos) { 
            if (config_file_path.size() <= 13) {
                response = MapsResponse(req, content_type);
            }
            else {
                std::string map_id(config_file_path.substr(13));
                model::Map::Id id(map_id.c_str());
                response = MapById(req, content_type, id);
            }
        }
        else if (config_file_path.find("/api/v1/game") != std::string::npos) {
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
            else if (config_file_path.find("records") != std::string::npos) {
                response = GetRecords(req, content_type, config_file_path);
            }
            else if (config_file_path.find("tick") != std::string::npos) {
                if (tick_period_ == 0) {
                    response = TimeControl(req, content_type);
                }
                else {
                    std::cout << tick_period_ << std::endl;
                    response = MakeStringResponse(http::status::bad_request, ErrorResponse("badRequest", "Invalid endpoint"), http_version, content_type);
                }
            }
        }
        response.set(http::field::cache_control, "no-cache"sv);
        return response;
    }

    StringResponse ApiHandler::MapsResponse(const StringRequest& req, const std::string_view content_type)
	{
        if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::GET_HEAD, req.version(), content_type)) {
            return *invalid_method;
        }

		json::array maps; 

		std::vector<model::Map> maps_vector = App_.game_.GetMaps(); //все доступные карты 

		for (const auto& map : maps_vector) {
            json::object map_properties; //объект, содержащий свойства одной карты

			map_properties["id"] = *map.GetId();
			map_properties["name"] = map.GetName();

			maps.push_back(map_properties);
		}

        return MakeStringResponse(http::status::ok, json::serialize(maps) , req.version(), content_type);

	}

	StringResponse ApiHandler::MapById(const StringRequest& req, const std::string_view content_type, model::Map::Id id)
	{
        if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::GET_HEAD, req.version(), content_type)) {
            return *invalid_method;
        }

        if (App_.game_.FindMap(id) == nullptr) {
            return MakeStringResponse(http::status::not_found, ErrorResponse("mapNotFound", "Map not found"), req.version(), content_type);
        }

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

        if (!this_map->GetLootTypesProp().empty()) {
            auto& loot_types = this_map->GetLootTypesProp();

            json::array obj_array;
            for (const auto& loot_type : loot_types) {
                json::object type;

                for (const auto& pair : loot_type) {
                    std::visit([&type, &pair](auto&& arg) {
                        type[pair.first] = arg;
                    }, pair.second);
                }

                obj_array.emplace_back(type);
            }
            map["lootTypes"] = obj_array;
        }

        return MakeStringResponse(http::status::ok, json::serialize(map), req.version(), content_type);
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

    json::array ApiHandler::GameRecordsInfo(std::vector<db::RetiredPlayer>& records)
    {
        json::array records_json;
        for (auto& record : records) {
            json::object record_json;

            record_json["name"] = record.GetName();
            record_json["score"] = record.GetScore();
            record_json["playTime"] = record.GetPlayingTime();

            records_json.push_back(record_json);
        }

        return records_json;
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
            return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::TokenNotFound);
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
            return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::TokenNotFound);

        }, req, content_type);
    }

    StringResponse ApiHandler::GetRecords(const StringRequest& req, const std::string_view content_type, const std::string_view path)
    {
        if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::GET_HEAD, req.version(), content_type)) {
            return *invalid_method;
        }
        
        int start_idx = 0;
        int max_items = 100;

        if (auto start_param = ExtractIntParameter(path, "start="); start_param.has_value()) {
            start_idx = start_param.value();
        }

        if (auto max_items_param = ExtractIntParameter(path, "maxItems="); max_items_param.has_value()) {
            max_items = max_items_param.value();
            if (max_items < 0 || max_items > 100) {
                return MakeStringResponse(http::status::bad_request, ErrorResponse("invalidArgument", "Invalid max items element"),
                    req.version(), content_type);
            }
        }

        auto records = App_.GetDB().GetRecords(start_idx, max_items);
        auto response_body = GameRecordsInfo(records);


        return MakeStringResponse(http::status::ok, json::serialize(response_body), req.version(), content_type);
    }

    json::object ApiHandler::GetJsonPlayersState(const std::unordered_map<std::uint64_t, Dog::DogState>& players_state)
    {
        json::object players_json;

        //Состояние игроков
        for (const auto& pair : players_state) {

            auto& dog_state = pair.second;
            json::object stats;

            json::array position;
            position.push_back(dog_state.position.x);
            position.push_back(dog_state.position.y);

            json::array speed;
            speed.push_back(dog_state.speed.Vx);
            speed.push_back(dog_state.speed.Vy);

            stats["pos"] = position;
            stats["speed"] = speed;
            stats["dir"] = dog_state.direction;

            json::array bag;

            for (auto& loot : dog_state.bag.dog_loot_) {
                json::object loot_state;
                loot_state["id"] = loot.id;
                loot_state["type"] = loot.type_;

                bag.push_back(loot_state);
            }

            stats["bag"] = bag;

            stats["score"] = dog_state.score;

            players_json[std::to_string(pair.first)] = stats;
        }
        return players_json;
    }

    json::object ApiHandler::GetJsonLootState(const std::unordered_map<int, Loot>& loot_state)
    {
        json::object lost_objects;

        for (const auto& pair : loot_state) {
            json::object lost_object;
            json::array coord;

            lost_object["type"] = pair.second.type_;

            coord.push_back(pair.second.pos_.x);
            coord.push_back(pair.second.pos_.y);

            lost_object["pos"] = coord;

            lost_objects[std::to_string(pair.first)] = lost_object;
        }
        return lost_objects;
    }

    StringResponse ApiHandler::GetState(const StringRequest& req, const std::string_view content_type)
    {
        if (auto invalid_method = Allow::MethodNotAllowed(req.method(), Allow::GET_HEAD, req.version(), content_type)) {
            return *invalid_method;
        }

        return ExecuteAuthorized([&](const app::Token& token) {

            auto game_state = App_.GetGameState(token);

            if (game_state) {
                json::object res_body;

                res_body["players"] = GetJsonPlayersState(game_state->GetPlayersState());

                res_body["lostObjects"] = GetJsonLootState(game_state->GetLootState());

                return MakeStringResponse(http::status::ok, json::serialize(res_body), req.version(), content_type);
            }

            return MakeUnauthorizedError(req.version(), content_type, app::AuthorizationErrorReason::TokenNotFound);
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

        auto tick = it->value().as_int64();

        App_.Tick(tick);

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

            //иначе создаем новый путь, добавив в начало путь к корневой директории со статическими файлам и
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
