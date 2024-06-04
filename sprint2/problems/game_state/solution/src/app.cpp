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
		//добавить сюда метод добавления игрока путем добавления собаки в сессию и убрать из Request_handler
		//также мб убрать из player gamesession, чтобы добавлять собаку непосредственно в этой функции

		//надо переписать метод добавления собаки в сессию мб. Надо написать метод в model, который будет искать сессию. Если кол-во человек <= 10, то надо вернуть сущ сессию
		//если > 10, то добавить новую и вернуть указатель на новую
	}



	ListPlayersResult ListPlayersUseCase::GetPlayersList(std::string_view authHeader)
	{
		std::string_view substr = "Bearer ";
		size_t pos = authHeader.find(substr);
		if (pos != std::string_view::npos && authHeader.size() - substr.size() == 32) {
			std::string token(authHeader.substr(pos + substr.size()));

			auto cur_player = players_.FindByToken(app::Token(token));

			if (cur_player == nullptr) {
				throw ListPlayersError{ ListPlayersErrorReason::TokenNotFound };
			}

			//тело ответа

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
		else {
			throw ListPlayersError{ ListPlayersErrorReason::InvalidHeader };
		}

	}

	GameStateResult GameStateUseCase::GameState(std::string_view authHeader)
	{
		std::string_view substr = "Bearer ";
		size_t pos = authHeader.find(substr);
		if (pos != std::string_view::npos && authHeader.size() - substr.size() == 32) {
			std::string token(authHeader.substr(pos + substr.size()));
			auto cur_player = players_.FindByToken(app::Token(token));

			if (cur_player == nullptr) {
				throw GameStateError{ GameStateErrorReason::TokenNotFound };
			}

			std::unordered_map<std::uint64_t, Player::PlayerState> players_state;

			auto& dogs = cur_player->session_->GetDogs();

			for (auto& dog : dogs) {
				Player::PlayerState temp_state;

				temp_state.position = dog->GetSpawnPoint();

				auto id = *dog->GetDogId();
				players_state[id] = std::move(temp_state);
			}

			return { players_state };
			
		}
		else {
			throw ListPlayersError{ ListPlayersErrorReason::InvalidHeader };
		}
	}


	JoinGameResult Application::JoinGame(const std::string& map_id, const std::string& name)
	{
		return join_game_.Join(map_id, name);
	}

	ListPlayersResult Application::GetPlayersList(std::string_view authHeader)
	{
		return list_players_.GetPlayersList(authHeader);
	}

	GameStateResult Application::GetGameState(std::string_view authHeader)
	{
		return game_state_.GameState(authHeader);
	}


}

