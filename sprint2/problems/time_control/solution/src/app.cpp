#include "app.h"

namespace app {
	Players::PairPlayerAndToken Players::AddPlayer(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session)
	{
		auto token = generateToken();
		auto player = std::make_shared<Player>(dog, session);

		// Добавить player в контейнер игроков по необходимости
		players_[token] = player;

		return std::make_tuple(player, token);
	}
	std::shared_ptr<Player> Players::FindByDogIdAndMapId(const Dog::DogId& dog_id, const Map::Id& map_id)
	{
		//будет искать id собаки в векторе dogs
		//будет искать id карты в векторе maps в 


		return std::shared_ptr<Player>();
	}

	std::shared_ptr<Player> Players::FindByToken(const Token& token)
	{
		//здесь будет искать игрока по токену через hash-map

		if (players_.find(token) == players_.end()) {
			return nullptr;
		}
		return players_[token];
	}



	JoinGameResult JoinGameUseCase::Join(const std::string& map_id, const std::string& name)
	{
		if (name.empty()) {
			throw JoinGameError{ JoinGameErrorReason::InvalidName };
		}

		//auto session = game_.FindSession(Map::Id(map_id));

		//ДОБАВИТЬ СПАВН!!!! - НОВОЕ ЗАДАНИЕ КОРОЧЕ

		Dog::DogName dog_name(name);

		if (auto session = game_.FindSession(Map::Id(map_id))) {
			auto [player, token] = players_.AddPlayer(session->AddDog(dog_name), session);

			return { token, player->GetId() }; //JoinGameResult
		} 
		else {
			throw JoinGameError{ JoinGameErrorReason::InvalidMap };
		}
	}



	std::optional<ListPlayersResult> ListPlayersUseCase::GetPlayersList(const Token& token)
	{
			auto cur_player = players_.FindByToken(token);

			if (cur_player == nullptr) {
				return std::nullopt;
			}

			//получение списка персонажей в игровой сессии
			auto& dogs = cur_player->session_->GetDogs();
			std::unordered_map<std::uint64_t, std::string> list_players;

			//итерация по каждому персонажу
			for (auto& dog : dogs) {
				std::string name = *dog->GetDogName();

				std::uint64_t id = *dog->GetDogId();
				list_players[id] = name;
			}

			return { list_players };

	}

	std::optional<GameStateResult> GameStateUseCase::GameState(const Token& token)
	{
			auto cur_player = players_.FindByToken(token);

			if (cur_player == nullptr) {
				return std::nullopt;
			}

			std::unordered_map<std::uint64_t, Dog::DogState> players_state;

			auto& dogs = cur_player->session_->GetDogs();

			for (auto& dog : dogs) {

				auto& state = dog->GetDogState();

				//state.position = dog->GetSpawnPoint();

				auto id = *dog->GetDogId();
				players_state[id] = state;
			}

			return { players_state };
			
	}

	int GameTime::game_time = 0;

	void PlayerActionUseCase::GetCorrectCoordinates(const Map* map, Dog::DogState& state)
	{
		//int time = GameTime{}.game_time;
		double pos_x = state.position.x + state.speed.Vx * static_cast<double>(GameTime::game_time) / 1000;

		double pos_y = state.position.y + state.speed.Vy * static_cast<double>(GameTime::game_time) / 1000;

		auto available_roads = map->FindRoadsAt(state.position.x, state.position.y);

		for (const auto& road : available_roads) {
			if (road.IsHorizontal()) {
				double road_min_x = std::min(road.GetStart().x, road.GetEnd().x);
				double road_max_x = std::max(road.GetStart().x, road.GetEnd().x);
				double road_y = road.GetStart().y;

				if (pos_y < road_y - 0.4 || pos_y > road_y + 0.4) {
					pos_y = std::clamp(pos_y, road_y - 0.4, road_y + 0.4);
				}
				if (pos_x < road_min_x) {
					pos_x = road_min_x;
				}
				else if (pos_x > road_max_x) {
					pos_x = road_max_x;
				}
			}
			else {
				double road_min_y = std::min(road.GetStart().y, road.GetEnd().y);
				double road_max_y = std::max(road.GetStart().y, road.GetEnd().y);
				double road_x = road.GetStart().x;

				if (pos_x < road_x - 0.4 || pos_x > road_x + 0.4) {
					pos_x = std::clamp(pos_x, road_x - 0.4, road_x + 0.4);
				}
				if (pos_y < road_min_y) {
					pos_y = road_min_y;
				}
				else if (pos_y > road_max_y) {
					pos_y = road_max_y;
				}
			}
		}

		state.position.x = pos_x;
		state.position.y = pos_y;
	}

	std::optional<std::string_view> PlayerActionUseCase::MovePlayer(const Token& token, const std::string& move)
	{
		auto cur_player = players_.FindByToken(token);

		if (cur_player == nullptr) {
			return std::nullopt;
		}

		auto player_movement = PlayerMovement(cur_player->session_->GetMap()->GetDogSpeed()); 

		auto it = player_movement.change_of_dir.find(move);

		auto& [x, y] = it->second;

		auto& state = cur_player->dog_->GetDogState();	

		state.direction = move;

		state.speed.Vx = x;
		state.speed.Vy = y;

		GetCorrectCoordinates(cur_player->session_->GetMap(), state);

		return "{}"sv;
	}

	JoinGameResult Application::JoinGame(const std::string& map_id, const std::string& name)
	{
		return join_game_.Join(map_id, name);
	}

	std::optional<ListPlayersResult> Application::GetPlayersList(const Token& token)
	{
		return list_players_.GetPlayersList(token);
	}

	std::optional<GameStateResult> Application::GetGameState(const Token& token)
	{
		return game_state_.GameState(token);
	}

	std::optional<std::string_view> Application::MovePlayer(const Token& token, const std::string& move) {
		return player_action.MovePlayer(token, move);
	}
}

