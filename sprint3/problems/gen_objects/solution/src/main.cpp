#include "logger.h"
#include <boost/program_options.hpp>

#include "sdk.h"
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>

#include "request_handler.h"
#include "json_loader.h"
#include "ticker.h"
#include "loot_generator.h"


using namespace std::literals;
using namespace ticker;
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
        ("randomize-spawn-points", po::bool_switch(&args.randomize_spawn_points), "spawn dogs at random positions");

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

int main(int argc, const char* argv[]) {
    try {
        if (auto args = ParseCommandLine(argc, argv)) {

            //если параметра нет, то присвоится nullopt
            int tick_period = args->tick_period;

            //если параметра нет, то значение будет false => спавн в начале карты
            bool randomize_spawn = args->randomize_spawn_points;

            // 1. Загружаем карту из файла и построить модель игры
            model::Game game = json_loader::LoadGame(args->config_file);

            auto interval = std::chrono::milliseconds(static_cast<int64_t>(game.GetLootGeneratorConfig()->period_));
            loot_gen::LootGenerator loot_generator(interval, game.GetLootGeneratorConfig()->probability_);

            const unsigned num_threads = std::thread::hardware_concurrency();
            
            // 2. Инициализируем io_context
            net::io_context ioc(num_threads);

            // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
            net::signal_set signals(ioc, SIGINT, SIGTERM);
            signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
                if (!ec) {
                    ioc.stop();
                    logger::LOG(logger::GetServerStopData(0), "server exited"sv);
                }
                });

            auto api_strand = net::make_strand(ioc);

            logger::InitBoostLogFilter(); //объявляем логер
            
            app::Players players;

            app::Application App_{game, players};

            if (tick_period > 0) {
                auto ticker = std::make_shared<Ticker>(api_strand, std::chrono::milliseconds(tick_period),
                    [&App_, &loot_generator](std::chrono::milliseconds delta) { App_.Tick(delta.count(), loot_generator); }
                );

                ticker->Start();
            }


            // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
            auto handler = std::make_shared<http_handler::RequestHandler>(api_strand, App_, loot_generator, fs::path(args->static_files_root),
                args->tick_period, args->randomize_spawn_points);


            http_handler::LoggingRequestHandler logging_handler{ *handler };

            // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов

            const auto address = net::ip::make_address("0.0.0.0");
            constexpr net::ip::port_type port = 8080;
            http_server::ServeHttp(ioc, { address, port }, [&logging_handler](auto&& req, auto&& send) {
                logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
                });


            logger::LOG(logger::GetServerStartData(port, address.to_string()), "server has started"sv);

            // 6. Запускаем обработку асинхронных операций
            RunWorkers(std::max(1u, num_threads), [&ioc] {
                ioc.run();
                });
        }
    }
    catch (const std::exception& ex) {
        logger::LOG(logger::GetServerStopData(EXIT_FAILURE, ex), "server exited"sv);
        return EXIT_FAILURE;
    }
}
