#include "save_and_restore_state.h"

namespace save_and_restore {
	void StateSaverAndRestorer::SaveState(const Game& game, const Players& players)
	{
        RenameFile();
        std::ofstream ofs(file_path_);

        boost::archive::text_oarchive oa{ ofs };

        GameRepr game_repr(game);
        PlayersRepr players_repr(players);

        oa << game_repr;
        oa << players_repr;
	}

    void StateSaverAndRestorer::RestoreState(Game& game, Players& players) const
    {
        try {
            if (fs::exists(file_path_) && !fs::is_empty(file_path_)) {
                std::ifstream ifs(file_path_);
                boost::archive::text_iarchive ia{ ifs };

                GameRepr game_repr;
                PlayersRepr players_repr;

                ia >> game_repr;
                ia >> players_repr;

                game_repr.Restore(game);
                players_repr.Restore(game, players);
            }
        }
        catch (const std::exception& ex) {
            throw std::runtime_error("Unable to restore state");
        }
    }

    void StateSaverAndRestorer::RenameFile() const
    {
        if (fs::exists(file_path_)) {
            fs::rename(file_path_, prev_state_save_path_);
        }
    }
} //namespace save_and_restore