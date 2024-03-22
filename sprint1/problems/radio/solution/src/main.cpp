#include "audio.h"
#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>

namespace net = boost::asio;

using net::ip::udp;

using namespace std::literals;

void startServer(uint16_t port)
{
	try {
		Player player(ma_format_u8, 1);

		net::io_context ioContext;
		udp::socket socket(ioContext, udp::endpoint(udp::v4(), port));

		for (;;)
		{
			std::cout << "Server has started his work!!!"sv << std::endl;

			std::vector<char> recv_buf;
			recv_buf.resize(65000);

			udp::endpoint remote_endpoint;

			auto size = socket.receive_from(net::buffer(recv_buf), remote_endpoint); //размер в байтах - количество фреймов
			std::cout << "Client send voice message. "sv << size /player.GetFrameSize()  << " frames" << std::endl;
			std::cout << "Record playing!"sv << std::endl;
			player.PlayBuffer(recv_buf.data(), size, 1.5s);
		}
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

void startClient(uint16_t port)
{
	try
	{ 
		Recorder recorder(ma_format_u8, 1);

		net::io_context ioContext;
		udp::socket socket(ioContext, udp::v4());

		std::string Server_IP;
		std::cout << "Enter server's IP: "sv;
		std::getline(std::cin, Server_IP);


		std::string str;
		std::cout << "Press Enter to record message: "sv;
		std::getline(std::cin, str);

		std::cin.clear();

		auto record_result = recorder.Record(65000, 1.5s);
		boost::system::error_code ec;
		auto endpoint = udp::endpoint(net::ip::make_address(Server_IP, ec), port);
		std::cout << "Amount of bytes: "sv << record_result.frames << std::endl;
		socket.send_to(net::buffer(record_result.data, record_result.frames * recorder.GetFrameSize()), endpoint);
	}
	catch (std::exception& e)
	{
		throw e;
	}
}

int main(int argc, const char** argv)
{
	setlocale(LC_ALL, "rus");

	for (;;)
	{
		if (argc != 3) {
			std::cout << "Usage: "sv << argv[0] << " <server IP>"sv << std::endl;
			return 1;
		}

		if (strcmp(argv[1], "server") == 0)
		{
			try
			{
				startServer(static_cast<uint16_t>(std::stoi(argv[2])));
				std::cout << "Playing done"sv << std::endl;
			}
			catch (std::exception& e)
			{
				std::cout << "Server execution failed" << std::endl;
				std::cout << e.what() << std::endl;
				return 1;
			}
		}
		else if (strcmp(argv[1], "client") == 0)
		{
			try
			{
				startClient(static_cast<uint16_t>(std::stoi(argv[2])));
				std::cout << "Recording done and sent" << std::endl;
			}
			catch (std::exception& e)
			{
				std::cout << "Client execution failed" << std::endl;
				std::cout << e.what() << std::endl;
				return 1;
			}
		}
		else
		{
			std::cout << "Invalid input" << std::endl;
			return 1;
		}
	}
	return 0;
}