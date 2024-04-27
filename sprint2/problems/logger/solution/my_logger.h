#pragma once

#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <optional>
#include <mutex>
#include <thread>

using namespace std::literals;

#define LOG(...) Logger::GetInstance().Log(__VA_ARGS__)

class Logger {
    std::chrono::system_clock::time_point GetTime() const {
        if (manual_ts_) {
            return *manual_ts_;
        }

        return std::chrono::system_clock::now();
    }

    auto GetTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        return std::put_time(std::localtime(&t_c), "%F %T");
    }

    // Для имени файла возьмите дату с форматом "%Y_%m_%d"
    std::string GetFileTimeStamp() const {
        const auto now = GetTime();
        const auto t_c = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t_c), "%Y_%m_%d");
        return ss.str();
    }

    Logger() = default;
    Logger(const Logger&) = delete;

public:
    static Logger& GetInstance() {
        static Logger obj;
        return obj;
    }

    // Выведите в поток все аргументы.
    template<class... Ts>
    void Log(const Ts&... args) {

        std::lock_guard<std::mutex> lock(mutex_);

        if (current_date_ != GetFileTimeStamp()) {
            current_date_ = GetFileTimeStamp();

            if (log_file_.is_open()) {
                log_file_.close();
            }

            /*log_file_.open("/var/log/sample_log_" + current_date_ + ".log", std::ios::app);*/
            log_file_.open("sample_log_" + current_date_ + ".log", std::ios::app);
        }
        if (log_file_.is_open()) {

            log_file_ << GetTimeStamp() << ": ";

            ((log_file_ << ... << args)) << std::endl;

            log_file_ << std::endl;
        }
    }

    // Установите manual_ts_. Учтите, что эта операция может выполняться
    // параллельно с выводом в поток, вам нужно предусмотреть 
    // синхронизацию.
    void SetTimestamp(std::chrono::system_clock::time_point ts) {
        std::lock_guard<std::mutex> lock(mutex_);
        manual_ts_ = ts;
    }


private:
    std::optional<std::chrono::system_clock::time_point> manual_ts_;

    std::mutex mutex_;
    std::ofstream log_file_;
    std::string current_date_;
};