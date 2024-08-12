#include "data_base.h"

namespace db {
	DataBase::DataBase(ConnectionPool& conn_pool) : conn_pool_(conn_pool)
	{
		auto conn = conn_pool_.GetConnection();
		pqxx::work w_{ *conn };

		//создание таблицы рекордов
		w_.exec(R"(CREATE TABLE IF NOT EXISTS retired_players (
			id UUID PRIMARY KEY,
			name varchar(100) NOT NULL,
			score integer NOT NULL,
			playTime float8 NOT NULL );)"_zv);

		//создание индекса для быстрой сортировки
		w_.exec(R"(CREATE INDEX IF NOT EXISTS idx_retired_players 
				ON retired_players (score DESC, playTime ASC, name ASC);)"_zv);

		w_.commit();

	}

	void DataBase::RetirePlayers(std::vector<RetiredPlayer>& retired_players_)
	{
		auto conn = conn_pool_.GetConnection();

		pqxx::work w_{ *conn };

		for (auto& player : retired_players_) {
			w_.exec_params(R"(INSERT INTO retired_players VALUES($1, $2, $3, $4);)"_zv,
				player.GetId().ToString(), player.GetName(), player.GetScore(), player.GetPlayingTime());
		}
		w_.commit();
	}

	std::vector<RetiredPlayer> DataBase::GetRecords(int start_idx, int max_items)
	{
		std::vector<RetiredPlayer> records;

		auto conn = conn_pool_.GetConnection();
		pqxx::read_transaction r_{ *conn };

		auto get_records = "SELECT id, name, score, playTime FROM retired_players ORDER BY score DESC, playtime, name LIMIT "
			+ std::to_string(max_items) + " OFFSET " + std::to_string(start_idx);

		for (auto [id, name, score, play_time] : r_.query<std::string, std::string, int, double>(get_records)) {
			records.push_back({ PlayerId::FromString(id), name, score, play_time });
		}

		return records;
	}



}
