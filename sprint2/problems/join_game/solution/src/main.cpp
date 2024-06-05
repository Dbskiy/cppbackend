#include "logger.h"

#include "sdk.h"
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>

#include "request_handler.h"
#include "json_loader.h"
#include "app.h"


using namespace std::literals;
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

int main(int argc, const char* argv[]) {
    if (argc != 3) { //3
        std::cerr << "Usage: game_server <game-config-json>"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);

        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // strand для выполнения запросов к API

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
        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        auto handler = std::make_shared<http_handler::RequestHandler>(api_strand, game, fs::path(argv[2]), players);


        http_handler::LoggingRequestHandler logging_handler{*handler};

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send)); //присобачить сюда LoggerHandler
        });
        
       
        logger::LOG(logger::GetServerStartData(port, address.to_string()), "server has started"sv);

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        logger::LOG(logger::GetServerStopData(EXIT_FAILURE, ex), "server exited"sv);
        return EXIT_FAILURE;
    }
}
