#include "serializing_listener.h"

namespace serializing_listener {
	void SerializingListener::OnTick(int tick)
	{
        time_since_save_ += tick;

        if (time_since_save_ >= save_period_) {
            saver_->SaveState(game_, players_);
            time_since_save_ = 0;
        }
	}
}