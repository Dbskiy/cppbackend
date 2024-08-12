#include "logger.h"
#include <boost/program_options.hpp>

#include "sdk.h"
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <cstdlib>
#include <thread>

#include "request_handler.h"
#include "json_loader.h"
#include "ticker.h"
#include "serializing_listener.h"

using namespace std::literals;
using namespace ticker;
using namespace save_and_restore;
using namespace serializing_listener;
namespace net = boost::asio;
namespace sys = boost::system;
namespace fs = std::filesystem;

namespace {

    // Запускает функцию fn на n потоках, включая текущий
    template <typename Fn>
    void RunWorkers(unsigned num_threads, const Fn& fn) {
        num_threads = std::max(1u, num_threads);
        std::vector<std::jthread> workers;
        workers.reserve(num_threads - 1);
        // Запускаем n-1 рабочих потоков, выполняющих функцию fn
        while (--num_threads) {
            workers.emplace_back(fn);
        }
        fn();
    }
}  // namespace

struct Args {
    int tick_period = 0;
    std::string config_file;
    std::string static_files_root;
    bool randomize_spawn_points = false;
    std::string state_file = "";
    int save_state_period = 0;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;

    po::options_description desc{ "Allowed options"s };

    Args args{};

    desc.add_options()
        ("help,h", "Show help")
        ("tick-period,t", po::value(&args.tick_period)->value_name("milliseconds"s), "set tick period")
        ("config-file,c", po::value(&args.config_file)->value_name("file"s), "set config file path")
        ("www-root,w", po::value(&args.static_files_root)->value_name("dir"s), "set static files root")
        ("randomize-spawn-points", po::bool_switch(&args.randomize_spawn_points), "spawn dogs at random positions")
        ("state-file", po::value(&args.state_file)->value_name("file"s), "set state file path")
        ("save-state-period", po::value(&args.save_state_period)->value_name("milliseconds"s), "set save state period");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::cout << desc;


    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    if (!vm.contains("config-file"s)) {
        throw std::runtime_error("Configuration file path is not specified"s);
    }
    if (!vm.contains("www-root"s)) {
        throw std::runtime_error("Statis files root is not specified"s);
    }

    return args;
}

constexpr const char DB_URL_ENV_NAME[]{ "GAME_DB_URL" };

std::string GetURLFromEnv() {
    std::string db_url;
    if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
        db_url = url;
    }
    else {
        throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
    }
    return db_url;
}


int main(int argc, const char* argv[]) {
    try {

        if (auto args = ParseCommandLine(argc, argv)) {

            //если параметра нет, то значение будет false => спавн в начале карты
            bool randomize_spawn = args->randomize_spawn_points;

            const unsigned num_threads = std::thread::hardware_concurrency();

            // 1. Инициализируем io_context
            net::io_context ioc(num_threads);

            // 2. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    ioc.stop();
                    logger::LOG(logger::GetServerStopData(0), "server exited"sv);
                }
                });

            auto api_strand = net::make_strand(ioc);

            // 3. Объявление логера
            logger::InitBoostLogFilter(); 

            // 4. Загружаем карту из файла, строим модель игры. Создаем объект класса Игроков
            model::Game game = json_loader::LoadGame(args->config_file);
            app::Players players;

            // 5. Создаем объект для генерации игрового лута
            auto interval = std::chrono::seconds(static_cast<int64_t>(game.GetLootGeneratorConfig()->period_));
            loot_gen::LootGenerator loot_generator(interval, game.GetLootGeneratorConfig()->probability_);

            // 6. Создаем объект с игроками и сценариями использования игровой модели
            std::string db_url = GetURLFromEnv();

            // 7 . Создаем пул соединений и объект управления базой данных 
            db::ConnectionPool conn_pool{ 10, [db_url] {
                auto conn = std::make_shared<pqxx::connection>(db_url);
                return conn;
            }};
            db::DataBase data_base{ conn_pool };

            app::Application App_{ data_base, game, players, loot_generator };
            std::shared_ptr<StateSaverAndRestorer> state_saver_;

            // 8. Проверяем наличие пути к файлу автосхоранения игрового сохранения
            auto& state_file_path = args->state_file;
            //Если путь к сохраненному файлу не пустой
            if (state_file_path != "") {
                state_saver_ = std::make_shared<StateSaverAndRestorer>(state_file_path);

                // 9. Проверяеме возможность восстановления состояния
                try {
                    state_saver_->RestoreState(game, players);
                } //в случае, если восстановление заврешилось ошибкой
                catch (const std::exception& ex) {
                    logger::LOG(logger::GetServerStopData(EXIT_FAILURE, ex), "Failed to restore state"sv);
                    throw std::runtime_error("Invalid autosave file"s);
                }
                
                // 10. Инициализируем слушателя для App_, если указан период автосохранения
                if (args->save_state_period > 0) {
                    App_.SetAutoSaveListener(std::make_shared<SerializingListener>(state_saver_, args->save_state_period, game, players));
                }
            }

            // 11. Запускаем игровое время, если указан интервал
            int tick_period = args->tick_period;
            if (tick_period > 0) {
                auto ticker = std::make_shared<Ticker>(api_strand, std::chrono::milliseconds(tick_period),
                    [&App_, &loot_generator](std::chrono::milliseconds delta) { App_.Tick(delta.count()); }
                );

                ticker->Start();
            }


            // 12. Создаём обработчик HTTP-запросов и связываем его с моделью игры
            auto handler = std::make_shared<http_handler::RequestHandler>(api_strand, App_, fs::path(args->static_files_root),
                args->tick_period, args->randomize_spawn_points);


            http_handler::LoggingRequestHandler logging_handler{ *handler };

            // 13. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов

            const auto address = net::ip::make_address("0.0.0.0");
            constexpr net::ip::port_type port = 8080;
            http_server::ServeHttp(ioc, { address, port }, [&logging_handler](auto&& req, auto&& send) {
                logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
                });


            logger::LOG(logger::GetServerStartData(port, address.to_string()), "server has started"sv);

            // 14. Запускаем обработку асинхронных операций
            RunWorkers(std::max(1u, num_threads), [&ioc] {
                ioc.run();
                });

            // 15. Сохраненяем состояние после заврешения работы сервера
            if (state_saver_) {
                state_saver_->SaveState(game, players);
            }
        }
    }
    catch (const std::exception& ex) {
        logger::LOG(logger::GetServerStopData(EXIT_FAILURE, ex), "server exited"sv);
        return EXIT_FAILURE;
    }
}
