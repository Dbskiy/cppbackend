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
            : file_path_(file_path) 
        {
            prev_state_save_path_ = file_path_.parent_path() / "prev_save.txt";
        }

        void SaveState(const Game& game, const Players& players);

        void RestoreState(Game& game, Players& players) const;

    private:
        void RenameFile() const;

        fs::path file_path_;
        fs::path prev_state_save_path_;
    };
}
