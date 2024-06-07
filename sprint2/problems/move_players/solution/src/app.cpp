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
				Dog::DogState temp_state;

				state.position = dog->GetSpawnPoint();

				auto id = *dog->GetDogId();
				players_state[id] = state;
			}

			return { players_state };
			
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

		cur_player->dog_->GetDogState().direction = move;
		cur_player->dog_->GetDogState().speed.Vx = x;
		cur_player->dog_->GetDogState().speed.Vy = y;

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

