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

	void GatheringAndDelivering::GatherAndDeliver()
	{

		GathererProvider gatherer_provider = GetGathererProvider();
		DelivererProvider deliverier_provider = GetDelivererProvider();

		std::vector<GatheringEvent> g_events = FindGatherEvents(gatherer_provider);
		std::vector<DeliveringEvent> d_events = FindDeliveringEvents(deliverier_provider);

		SetCorrectChronology(g_events, d_events, dogs_);

		size_t i = 0, j = 0;

		//алгоритм добавления лута:
		//Проверка, что события еще не было. Затем, что в сумке есть место, потом добавляем лут в сумку
		//после цикла произвести отчистку вектора предметов

		while (true) {
			if (i < g_events.size() && j < d_events.size()) {
				if (g_events[i].time == d_events[j].time) {
					//очищаем сумку, а затем добавляем лут (необязательно тому же игроку)

					BringLootToTheBase(d_events[j].deliverer_id);
					PutInTheBag(g_events[i].gatherer_id, g_events[i].item_id);

					++i;
					++j;
				}
				else if (g_events[i].time < d_events[j].time) {
					PutInTheBag(g_events[i].gatherer_id, g_events[i].item_id);
					++i;
				}
				else {
					BringLootToTheBase(d_events[j].deliverer_id);
					++j;
				}
			}
			else if (i < g_events.size()) {
				PutInTheBag(g_events[i].gatherer_id, g_events[i].item_id);
				++i;
			}
			else if (j < d_events.size()) {
				BringLootToTheBase(d_events[j].deliverer_id);
				++j;
			}
			else {
				break;
			}
		}

		//функция по очистке вектора с лутом
		session_->RemoveCollectedLoot();

	}

	GathererProvider GatheringAndDelivering::GetGathererProvider()
	{
		GathererProvider provider;

		for (const auto& loot : all_loot_) {
			Item item;
			item.position.x = loot.pos_.x;
			item.position.y = loot.pos_.y;

			provider.SetItem(item);
		}

		for (const auto& dog : dogs_) {
			const auto& state = dog->GetDogState();
			Gatherer gatherer;

			gatherer.start_pos.x = state.prev_position.x;
			gatherer.start_pos.y = state.prev_position.y;

			gatherer.end_pos.x = state.position.x;
			gatherer.end_pos.y = state.position.y;

			provider.SetGatherer(gatherer);
		}

		return provider;
		
	}

	DelivererProvider GatheringAndDelivering::GetDelivererProvider()
	{
		DelivererProvider provider;

		for (const auto& office : offices_) {
			Base base;

			base.position.x = office.GetPosition().x;
			base.position.y = office.GetPosition().y;

			provider.SetBase(base);
		}

		for (const auto& dog : dogs_) {
			const auto& state = dog->GetDogState();
			Gatherer deliverer;

			deliverer.start_pos.x = state.prev_position.x;
			deliverer.start_pos.y = state.prev_position.y;

			deliverer.end_pos.x = state.position.x;
			deliverer.end_pos.y = state.position.y;

			provider.SetDeliverer(deliverer);
		}

		return provider;
	}

	void GatheringAndDelivering::SetCorrectChronology(std::vector<GatheringEvent>& g_events, std::vector<DeliveringEvent>& d_events, const GameSession::Dogs& dogs) const
	{

		for (auto& g_event : g_events) {
			auto& state = dogs[g_event.gatherer_id]->GetDogState();

			double distance_x = state.position.x - state.prev_position.x;
			double distance_y = state.position.y - state.prev_position.y;

			double cur_distance = std::sqrt(distance_x * distance_x + distance_y * distance_y);

			g_event.time = cur_distance / max_distance_;
		}

		std::sort(g_events.begin(), g_events.end(), [](const GatheringEvent& l, const GatheringEvent& r) {
			return l.time < r.time;
			});

		for (auto& d_event : d_events) {
			auto& state = dogs[d_event.deliverer_id]->GetDogState();

			double distance_x = state.position.x - state.prev_position.x;
			double distance_y = state.position.y - state.prev_position.y;

			double cur_distance = std::sqrt(distance_x * distance_x + distance_y * distance_y);

			d_event.time = cur_distance / max_distance_;
		}

		std::sort(d_events.begin(), d_events.end(), [](const DeliveringEvent& l, const DeliveringEvent& r) {
			return l.time < r.time;
			});
	}

	void GatheringAndDelivering::PutInTheBag(const size_t g_id, const size_t loot_id)
	{
		if (!all_loot_[loot_id].is_collected_ && !dogs_[g_id]->GetDogState().bag.isBagFull(bag_capacity_)) {

			all_loot_[loot_id].is_collected_ = true;

			dogs_[g_id]->GetDogState().bag.dog_loot_.push_back(all_loot_[loot_id]);
		}
	}

	void GatheringAndDelivering::BringLootToTheBase(const size_t d_id)
	{
		dogs_[d_id]->GetDogState().bag.FreeBag();
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

	void GameTime::GenerateLoot(int tick)
	{
		for (const auto& session : game_.GetSessions()) {
			auto new_loot_num = loot_gen_.Generate(std::chrono::milliseconds(tick), session->GetAllLoot().size(), session->GetDogs().size());

			for (int i = 0; i < new_loot_num; i++)
			{
				session->AddRandomLoot();
			}
		}
	}

	void GameTime::GatheringAndDeliviring(int tick)
	{
		for (auto& session : game_.GetSessions()) {
			GatheringAndDelivering gath_and_del(session, tick);

			gath_and_del.GatherAndDeliver();
		}
	}

	void GameTime::ChangeCoordinates(const Map* map, Dog::DogState& state, int tick) const
	{
		const double ms_to_s = 1000.0;

		std::vector<Road> available_roads = map->FindRoadsAt(state.position.x, state.position.y);

		auto cur_pos = state.position;

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

		state.prev_position = cur_pos;
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

	void Application::Tick(int tick)
	{
		game_time_.MoveAllPlayers(tick);
		game_time_.GenerateLoot(tick);
		game_time_.GatheringAndDeliviring(tick);
	}
}

	