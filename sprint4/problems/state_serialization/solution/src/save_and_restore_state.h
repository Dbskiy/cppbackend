#pragma once
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "state_serialization.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

using namespace model_serialization;
using namespace app_serialization;

namespace save_and_restore {
    namespace fs = std::filesystem;
    class StateSaverAndRestorer {
    public:
        explicit StateSaverAndRestorer(const std::string& file_path)
            : file_path_(file_path),
            prev_state_save_path_("state/prev_save.txt") {}

        void SaveState(const Game& game, const Players& players) {
            RenameFile();
            std::ofstream ofs(file_path_);

            boost::archive::text_oarchive oa{ ofs };

            GameRepr game_repr(game);
            PlayersRepr players_repr(players);

            oa << game_repr;
            oa << players_repr;
        }

        void RestoreState(Game& game, Players& players) {
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

    private:
        void RenameFile() {
            if (fs::exists(file_path_)) {
                fs::rename(file_path_, prev_state_save_path_);
            }
        }

        const fs::path file_path_;
        const fs::path prev_state_save_path_;
    };
}
