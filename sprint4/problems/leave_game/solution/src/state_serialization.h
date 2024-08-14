#include <boost/serialization/vector.hpp>
#include "app.h"

namespace model {
	template <typename Archive>
	void serialize(Archive& ar, Position& point, [[maybe_unused]] const unsigned version) {
		ar& point.x;
		ar& point.y;
	}

	template <typename Archive>
	void serialize(Archive& ar, Loot& obj, [[maybe_unused]] const unsigned version) {
		ar& (obj.id);
		ar& (obj.is_collected_);
		ar& (obj.pos_);
		ar& (obj.type_);
	}
	template <typename Archive>
	void serialize(Archive& ar, Speed& speed, [[maybe_unused]] const unsigned version) {
		ar& (speed.Vx);
		ar& (speed.Vy);
	}

	template <typename Archive>
	void serialize(Archive& ar, Dog::Bag& bag, [[maybe_unused]] const unsigned version) {
		ar& (bag.dog_loot_);
	}

	template <typename Archive>
	void serialize(Archive& ar, Dog::DogState& state, [[maybe_unused]] const unsigned version) {
		ar& (state.bag);
		ar& (state.direction);
		ar& (state.position);
		ar& (state.prev_position);
		ar& (state.score);
		ar& (state.speed);

	}
}

namespace model_serialization {
	using namespace model;
	//Серилизованное предствление класса Dog
	class DogRepr {
	public:
		DogRepr() = default;

		explicit DogRepr(std::shared_ptr<Dog> dog) :
			id_(dog->GetDogId()),
			name_(dog->GetDogName()),
			spawn_pos_(dog->GetSpawnPoint()),
			state_(dog->GetDogState()) {}

		[[nodiscard]] std::shared_ptr<Dog> Restore() const;

		template <typename Archive>
		void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
			ar&* id_;
			ar&* name_;
			ar& spawn_pos_;
			ar& state_;
		}

	private:
		Dog::DogId id_ = Dog::DogId{ 0u };
		Dog::DogName name_{ "" };
		Position spawn_pos_;
		Dog::DogState state_;
	};

	//Серилизованное предствление класса GameSession
	class SessionRepr {
		using VDogRepr = std::vector<DogRepr>;
	public:
		SessionRepr() = default;

		explicit SessionRepr(std::shared_ptr<GameSession> session);

		[[nodiscard]] std::shared_ptr<GameSession> Restore(const Game& game) const;

		template <typename Archive>
		void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
			ar& map_id_;
			ar& dogs_;
			ar& all_loot_;
		}

	private:
		std::string map_id_;
		VDogRepr dogs_;
		GameSession::AllLoot all_loot_;
	};


	//Серилизованное предствление класса Game
	class GameRepr {
	public:
		using SessionsRepr = std::vector<SessionRepr>;
		GameRepr() = default;

		explicit GameRepr(const Game& game);

		[[nodiscard]] void Restore(Game& not_restored_game);

		template <typename Archive>
		void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
			ar& sessions_repr_;
		}

	private:
		SessionsRepr sessions_repr_;
	};
}

namespace app_serialization {
	using namespace app;

	template <typename Archive>
	void serialize(Archive& ar, Token token, [[maybe_unused]] const unsigned version) {
		ar& (*token);
	}

	class PlayerRepr {
	public:
		PlayerRepr() = default;

		explicit PlayerRepr(model::GameSession::SessionId session_id, model::Dog::DogId dog_id, int entering_time, int inactivity_period)
			: 
			session_id_(session_id),
			dog_id_(dog_id),
			entering_time_(entering_time),
			inactivity_period_(inactivity_period) {}


		[[nodiscard]] std::shared_ptr<Player> Restore(const model::Game& game) const;

		template <typename Archive>
		void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
			ar& (*session_id_);
			ar& (*dog_id_);
			ar& entering_time_;
			ar& inactivity_period_;
		}

	private:
		model::GameSession::SessionId session_id_{ 0u };
		model::Dog::DogId dog_id_ = Dog::DogId{ 0u };
		int entering_time_ = 0;
		int inactivity_period_ = 0;
	};

	class PlayersRepr {
	public:
		PlayersRepr() = default;

		explicit PlayersRepr(const Players& players);

		[[nodiscard]] void Restore(const model::Game& game, Players& players) const;

		template <typename Archive>
		void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
			ar& tokens_;
			ar& players_repr_;
		}

	private:
		std::vector<std::string> tokens_;
		std::vector<PlayerRepr> players_repr_;

	};

}