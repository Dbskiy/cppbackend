/*#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/json.hpp>
#include <boost/date_time.hpp>*/ //если оставить только logger.h, тоже будет работать
//#define BOOST_BEAST_USE_STD_STRING_VIEW
#include "logger.h" //надо указать #define BOOST_BEAST_USE_STD_STRING_VIEW либо здесь, либо в logger.h

#include "sdk.h"
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>
#include <thread>

#include "request_handler.h"
#include "json_loader.h"


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

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                std::cout << "Signal "sv << signal_number << " received"sv << std::endl;
                ioc.stop();
                //BOOST_LOG_TRIVIAL(info)
                //    << logging::add_value(logger::additional_data, logger::GetServerStopData(0))
                //    << "server exited"sv;
                logger::LOG(logger::GetServerStopData(0), "server exited"sv);
            }
            });
        logger::InitBoostLogFilter(); //объявляем логер

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler{game, fs::path(argv[2])};
        http_handler::LoggingRequestHandler logging_handler{ handler };

        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send)); //присобачить сюда LoggerHandler
        });
        
	    //std::cout << "Hello! Server is starting at port " << port << std::endl;
        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        std::cout << "Server has started..."sv << std::endl;

        //BOOST_LOG_TRIVIAL(info)
        //    << logging::add_value(logger::additional_data, logger::GetServerStartData(port, address.to_string()))
        //    << "server started"sv;

        logger::LOG(logger::GetServerStartData(port, address.to_string()), "server started"sv);

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        //std::cerr << ex.what() << std::endl;
        //BOOST_LOG_TRIVIAL(info)
        //    << logging::add_value(logger::additional_data, logger::GetServerStopData(EXIT_FAILURE, ex))
        //    << "server exited"sv;
        logger::LOG(logger::GetServerStopData(EXIT_FAILURE, ex), "server exited"sv);
        return EXIT_FAILURE;
    }
}
