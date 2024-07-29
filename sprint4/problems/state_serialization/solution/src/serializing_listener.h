#pragma once
#include "save_and_restore_state.h"

namespace serializing_listener {
    using namespace save_and_restore;
    class SerializingListener : public ApplicationListener {
    public:
        SerializingListener(const std::shared_ptr<StateSaverAndRestorer>& saver, const int save_period, const model::Game& game, const app::Players& players)
            : saver_(saver),
            save_period_(save_period),
            game_(game),
            players_(players) {}

        void OnTick(int tick) override {
            time_since_save_ += tick;

            if (time_since_save_ >= save_period_) {
                saver_->SaveState(game_, players_);
                time_since_save_ = 0;
            }
        }

    private:
        const int save_period_;
        int time_since_save_ = 0;

        const model::Game& game_;
        const app::Players& players_;
        std::shared_ptr<StateSaverAndRestorer> saver_;

    };
}
