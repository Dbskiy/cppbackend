#include "state_serialization.h"

namespace model_serialization 
{
	std::shared_ptr<Dog> DogRepr::Restore() const
	{
		Dog dog{ id_, name_, spawn_pos_ };

		auto& state = dog.GetDogState();
		state.bag = state_.bag;
		state.direction = state_.direction;
		state.position = state_.position;
		state.prev_position = state_.prev_position;
		state.score = state_.score;
		state.speed = state_.speed;

		return std::make_shared<Dog>(dog);
	}

	SessionRepr::SessionRepr(std::shared_ptr<GameSession> session) : map_id_(*session->GetMap()->GetId()), all_loot_(session->GetAllLoot())
	{
		for (auto& dog : session->GetDogs()) {
			dogs_.push_back(DogRepr(dog));
		}
	}

	std::shared_ptr<GameSession> SessionRepr::Restore(const Game& game) const
	{
		GameSession session(GameSession::SessionId(game.GetSessions().size()), game.FindMap(Map::Id(map_id_)));

		GameSession::Dogs dogs;
		for (int i = 0; i < dogs_.size(); i++)
		{
			dogs.push_back(dogs_[i].Restore());
		}
		session.SetDogs(dogs);

		session.SetAllLoot(all_loot_);

		return std::make_shared<GameSession>(session);
	}


	GameRepr::GameRepr(const Game& game)
	{
		for (auto& session : game.GetSessions()) {
			sessions_repr_.push_back(SessionRepr(session));
		}
	}

	void GameRepr::Restore(Game& not_restored_game)
	{
		Game::Sessions sessions;

		for (auto& session_repr_ : sessions_repr_) {
			std::shared_ptr<GameSession> session = session_repr_.Restore(not_restored_game);
			sessions.push_back(session);
		}

		not_restored_game.SetSessions(sessions);
	}
}//namespace model_serialization

namespace app_serialization
{
	std::shared_ptr<Player> PlayerRepr::Restore(const model::Game& game) const
	{
		auto session = game.FindGameSession(session_id_);
		auto dog = session->GetDogById(dog_id_);

		int entering_time = entering_time_;

		int inactivity_period = inactivity_period_;

		std::shared_ptr<Player> player = std::make_shared<Player>(dog, session, entering_time);
		player->inactivity_period_ = inactivity_period;
		return player;
	}


	PlayersRepr::PlayersRepr(const Players& players)
	{
		auto& players_info = players.GetPlayersInfo();

		for (auto& pair : players_info) {
			tokens_.push_back(*pair.first);
			players_repr_.push_back(PlayerRepr(pair.second->GetSession()->GetId(),
				pair.second->GetId(), pair.second->GetEnteringTime(), pair.second->inactivity_period_));
		}
	}

	void PlayersRepr::Restore(const model::Game& game, Players& players) const
	{
		Players::PlayersInfo players_info;
		for (size_t i = 0; i < tokens_.size(); i++)
		{
			auto& token = tokens_[i];
			auto player = players_repr_[i].Restore(game);

			players_info[Token(token)] = player;
		}
		players.SetPlayersInfo(players_info);
	}

} //namespace app_serialization