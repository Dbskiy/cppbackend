#pragma once
#include "connection_pool.h"
#include "tagged_uuid.h"

namespace db {
	using namespace conn_pool;
	using pqxx::operator"" _zv;

	namespace detail {
		struct RetiredPlayerTag {};
	}  // namespace detail

	using PlayerId = util::TaggedUUID<detail::RetiredPlayerTag>;

	class RetiredPlayer {
	public:
		RetiredPlayer(PlayerId id, std::string name, int score, double playing_time) 
			: id_(std::move(id))
			, name_(std::move(name))
			, score_(std::move(score))
			, playing_time_(std::move(playing_time_))
		{}

		const PlayerId& GetId() {
			return id_;
		}

		const std::string& GetName() const {
			return name_;
		}

		const int& GetScore() const {
			return score_;
		}

		const double& GetPlayingTime() const {
			return playing_time_;
		}

	private:
		PlayerId id_;
		std::string name_;
		int score_;
		double playing_time_;
	};


	class DataBase {
	public:
		explicit DataBase(ConnectionPool& conn_pool);

		void RetirePlayers(std::vector<RetiredPlayer>& retired_players_);

		std::vector<RetiredPlayer> GetRecords(int start_idx, int max_items);

	private:
		ConnectionPool& conn_pool_;
	};
}
