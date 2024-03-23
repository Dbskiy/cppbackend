#ifdef WIN32
#include <sdkddkver.h>
#endif

#include "seabattle.h"

#include <atomic>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <string_view>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;

void PrintFieldPair(const SeabattleField& left, const SeabattleField& right) {
    auto left_pad = "  "s;
    auto delimeter = "    "s;
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
    for (size_t i = 0; i < SeabattleField::field_size; ++i) {
        std::cout << left_pad;
        left.PrintLine(std::cout, i);
        std::cout << delimeter;
        right.PrintLine(std::cout, i);
        std::cout << std::endl;
    }
    std::cout << left_pad;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << delimeter;
    SeabattleField::PrintDigitLine(std::cout);
    std::cout << std::endl;
}

template <size_t sz>
static std::optional<std::string> ReadExact(tcp::socket& socket) {
    boost::array<char, sz> buf;
    boost::system::error_code ec;

    net::read(socket, net::buffer(buf), net::transfer_exactly(sz), ec);

    if (ec) {
        return std::nullopt;
    }

    return {{buf.data(), sz}};
}

static bool WriteExact(tcp::socket& socket, std::string_view data) {
    boost::system::error_code ec;

    net::write(socket, net::buffer(data), net::transfer_exactly(data.size()), ec);

    return !ec;
}

class SeabattleAgent {
public:
    SeabattleAgent(const SeabattleField& field)
        : my_field_(field) {
    }

    void StartGame(tcp::socket& socket, bool my_initiative) {
        boost::system::error_code ec;

        while (!IsGameEnded())
        {
            PrintFields();

            if (my_initiative) {
                std::cout << "Your turn: "sv;

                std::cin.clear();
                std::string str;
                std::getline(std::cin, str);

                std::string_view myMove (str);
                std::optional<std::pair<int, int>> coordinates = ParseMove(myMove);

                Send(socket, str, ec);

                std::string myResult = Read(socket, ec);
                std::cout << myResult << std::endl;

                if (myResult == "Miss!") {
                    other_field_.MarkMiss(coordinates->second, coordinates->first);
                    my_initiative = false;
                }
                else if (myResult == "Hit!") {
                    other_field_.MarkHit(coordinates->second, coordinates->first);
                }
                else {
                    other_field_.MarkKill(coordinates->second, coordinates->first);;
                }

            }

            else {
                std::cout << "Waiting for turn..."sv << std::endl;

                std::string str = Read(socket, ec);
                std::cout << "Shot " << str << std::endl;;
                std::string_view otherMove(str);
               
                std::optional<std::pair<int, int>> coordinates = ParseMove(otherMove);

                SeabattleField::ShotResult shotResult = my_field_.Shoot(coordinates->second, coordinates->first);

                if (shotResult == SeabattleField::ShotResult::MISS) {
                    Send(socket, "Miss!", ec);
                    my_field_.MarkMiss(coordinates->second, coordinates->first);

                    my_initiative = true;
                }
                else if (shotResult == SeabattleField::ShotResult::HIT) {
                    Send(socket, "Hit!", ec);
                    my_field_.MarkHit(coordinates->second, coordinates->first);
                }
                else {
                    Send(socket, "Kill!", ec);
                    my_field_.MarkKill(coordinates->second, coordinates->first);
                }
            }
        }

        if (my_initiative) {
            std::cout << "You won!"sv << std::endl;
        }
        else {
            std::cout << "You lost :("sv << std::endl;
        }
    }

private:
    static std::optional<std::pair<int, int>> ParseMove(const std::string_view& sv) {
        if (sv.size() != 2) return std::nullopt;

        int p1 = sv[0] - 'A', p2 = sv[1] - '1';

        if (p1 < 0 || p1 > 8) return std::nullopt;
        if (p2 < 0 || p2 > 8) return std::nullopt;

        return {{p1, p2}};
    }

    static std::string MoveToString(std::pair<int, int> move) {
        char buff[] = {static_cast<char>(move.first) + 'A', static_cast<char>(move.second) + '1'};
        return {buff, 2};
    }

    void PrintFields() const {
        PrintFieldPair(my_field_, other_field_);
    }

    bool IsGameEnded() const {
        return my_field_.IsLoser() || other_field_.IsLoser();
    }

    void Send(tcp::socket& socket, const std::string& move, boost::system::error_code ec)
    {
        std::string temp = move + "\n";
        socket.write_some(net::buffer(temp), ec);

        if (ec) {
            std::cerr << "Error sending data"sv << std::endl;
        }
    }

    std::string Read(tcp::socket& socket, boost::system::error_code ec)
    {
        net::streambuf stream_buf;
        net::read_until (socket, stream_buf, '\n', ec);
        std::string temp{ std::istreambuf_iterator<char>(&stream_buf),
            std::istreambuf_iterator<char>() };
        if (ec) {
            std::cerr << "Error reading data"sv << std::endl;
        }
        std::string coordinates = temp.substr(0, temp.size() - 1);

        return coordinates;
    }
    

private:
    SeabattleField my_field_;
    SeabattleField other_field_;
};

void StartServer(const SeabattleField& field, unsigned short port) {
    SeabattleAgent agent(field);

    bool my_initiative = false;

    net::io_context io_context;

    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "Waiting for connection..."sv << std::endl;

    boost::system::error_code ec;

    tcp::socket socket{ io_context };
    acceptor.accept(socket, ec);

    if (ec) {
        std::cerr << "Can't accept connection"sv << std::endl;
    }
    else {
        agent.StartGame(socket, my_initiative);
    }
};

void StartClient(const SeabattleField& field, const std::string& ip_str, unsigned short port) {
    SeabattleAgent agent(field);

    bool my_initiative = true;

    boost::system::error_code ec;
    auto endpoint = tcp::endpoint(net::ip::make_address(ip_str, ec), port);

    if (ec) {
        std::cerr << "Wrong IP format"sv << std::endl;
    }

    net::io_context io_context;
    tcp::socket socket{ io_context };
    socket.connect(endpoint, ec);

    if (ec) {
        std::cerr << "Can't connect to server"sv << std::endl;
    }
    else {
        agent.StartGame(socket, my_initiative);
    }

};

int main(int argc, const char** argv) {
    if (argc != 3 && argc != 4) {
        std::cout << "Usage: program <seed> [<ip>] <port>" << std::endl;
        return 1;
    }

    std::mt19937 engine(std::stoi(argv[1]));
    SeabattleField fieldL = SeabattleField::GetRandomField(engine);

    if (argc == 3) {
        StartServer(fieldL, std::stoi(argv[2]));
    }
    else if (argc == 4) {
        StartClient(fieldL, argv[2], std::stoi(argv[3]));
    }
}
