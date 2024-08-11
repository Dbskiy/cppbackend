#pragma once
#include "connection_pool.h"

namespace db {
	using namespace conn_pool;
	using pqxx::operator"" _zv;


	class RetiredPlayer {
	public:
		RetiredPlayer(std::string name, int score, double playing_time) 
			: name_(std::move(name))
			, score_(std::move(score))
			, playing_time_(std::move(playing_time_))
		{}

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
		std::string name_;
		int score_;
		double playing_time_;
	};


	class DataBase {
	public:
		explicit DataBase(ConnectionPool& conn_pool);

		void RetirePlayers(std::vector<RetiredPlayer> retired_players_);

		std::vector<RetiredPlayer> GetRecords(int start_idx, int max_items);

	private:
		ConnectionPool& conn_pool_;
	};
}