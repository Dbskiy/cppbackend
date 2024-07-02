#include "app.h"

namespace app {
	Players::PairPlayerAndToken Players::AddPlayer(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session)
	{
		auto token = generateToken();
		auto player = std::make_shared<Player>(dog, session);

		players_[token] = player;

		return std::make_tuple(player, token);
	}

	std::shared_ptr<Player> Players::FindByToken(const Token& token)
	{

		if (players_.find(token) == players_.end()) {
			return nullptr;
		}
		return players_[token];
	}



	JoinGameResult JoinGameUseCase::Join(const std::string& map_id, const std::string& name, const bool randomize_spawn)
	{
		if (name.empty()) {
			throw JoinGameError{ JoinGameErrorReason::InvalidName };
		}

		Dog::DogName dog_name(name);

		if (auto session = game_.FindSession(Map::Id(map_id))) {
			auto [player, token] = players_.AddPlayer(session->AddDog(dog_name, randomize_spawn), session);

			return { token, player->GetId() };
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

				auto id = *dog->GetDogId();
				players_state[id] = state;
			}

			auto& all_loot = cur_player->session_->GetAllLoot();
			std::unordered_map<int, Loot> loot_state;

			for (int i = 0; i < all_loot.size(); i++)
			{
				loot_state[i] = all_loot[i];
			}

			return GameStateResult{ players_state, loot_state };
			
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

		return "{}"sv;
	}


	void GameTime::MoveAllPlayers(int tick)
	{
		auto& sessions = game_.GetSessions();

		for (const auto& session : sessions) {
			auto& dogs = session->GetDogs();
			const Map* map = session->GetMap();

			for (auto& dog : dogs) {
				Dog::DogState& state = dog->GetDogState();

				ChangeCoordinates(map, state, tick);
			}
		}
	}

	void GameTime::GenerateLoot(loot_gen::LootGenerator& loot_generator, int tick)
	{
		for (const auto& session : game_.GetSessions()) {
			auto new_loot_num = loot_generator.Generate(std::chrono::milliseconds(tick), session->GetAllLoot().size(), session->GetDogs().size());

			for (int i = 0; i < new_loot_num; i++)
			{
				session->AddRandomLoot();
			}
		}
	}

	void GameTime::ChangeCoordinates(const Map* map, Dog::DogState& state, int tick) const
	{
		const double ms_to_s = 1000.0;

		std::vector<Road> available_roads = map->FindRoadsAt(state.position.x, state.position.y);

		if (!available_roads.empty()) {
			state.position.x += state.speed.Vx * static_cast<double>(tick) / ms_to_s;
			state.position.y += state.speed.Vy * static_cast<double>(tick) / ms_to_s;
		}

		for (auto& road : available_roads) {

			Point start{ 0, 0 };
			Point end{ 0, 0 };

			if (road.IsHorizontal()) {
				start.x = std::min(road.GetStart().x, road.GetEnd().x);
				end.x = std::max(road.GetStart().x, road.GetEnd().x);
				start.y = end.y = road.GetStart().y;
			}
			else {
				start.y = std::min(road.GetStart().y, road.GetEnd().y);
				end.y = std::max(road.GetStart().y, road.GetEnd().y);
				start.x = end.x = road.GetStart().x;
			}

			if (available_roads.size() == 1) {

				PointXOutOfRoad(state, start.x, end.x);
				PointYOutOfRoad(state, start.y, end.y);

			}
			else {

				if (road.IsHorizontal() && (state.direction == "L" || state.direction == "R")) {
					PointXOutOfRoad(state, start.x, end.x);
				}
				else if (road.IsVertical() && (state.direction == "D" || state.direction == "U")) {
					PointYOutOfRoad(state, start.y, end.y);
				}
			}
		}


	}

	void GameTime::PointXOutOfRoad(Dog::DogState& state, int start_x, int end_x) const
	{
		auto half_of_width = Road::half_of_width;

		if (state.position.x < start_x - half_of_width) {
			state.position.x = start_x - half_of_width;
			state.SetDefaultSpeed();
		}
		else if (state.position.x > end_x + half_of_width) {
			state.position.x = end_x + half_of_width;
			state.SetDefaultSpeed();
		}
	}

	void GameTime::PointYOutOfRoad(Dog::DogState& state, int start_y, int end_y) const
	{
		auto half_of_width = Road::half_of_width;
		if (state.position.y < start_y - half_of_width) {
			state.position.y = start_y - half_of_width;
			state.SetDefaultSpeed();
		}
		else if (state.position.y > end_y + half_of_width) {
			state.position.y = end_y + half_of_width;
			state.SetDefaultSpeed();
		}
	}

	JoinGameResult Application::JoinGame(const std::string& map_id, const std::string& name, const bool randomize_spawn)
	{
		return join_game_.Join(map_id, name, randomize_spawn);
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
	void Application::Tick(int tick, loot_gen::LootGenerator& loot_generator)
	{
		game_time_.MoveAllPlayers(tick);
		game_time_.GenerateLoot(loot_generator, tick);
	}
}

