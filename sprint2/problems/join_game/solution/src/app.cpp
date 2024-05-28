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



}

