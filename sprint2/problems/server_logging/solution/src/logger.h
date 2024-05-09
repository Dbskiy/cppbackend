#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include <boost/json.hpp>

#include <boost/beast/core.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#include <boost/beast/http.hpp>

#include <chrono>

#include "sdk.h"


namespace logging = boost::log;
namespace json = boost::json;
namespace beast = boost::beast;
namespace net = boost::asio;
namespace http = beast::http;

using namespace std::chrono;

namespace logger
{
    json::object GetReqData(const std::string& client_ip, const std::string_view uri, http::verb method);

    json::object GetResData(const milliseconds& response_time, const int& code, const std::string_view content_type);
    
    json::object GetServerStartData(const net::ip::port_type port, const std::string& host_ip);

    //перегрузка функции
    json::object GetServerStopData(const int& code);
    json::object GetServerStopData(const int& code, const std::exception& ex);
    
    json::object GetErrorData(const beast::error_code& code, const std::string_view where);

    
    void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);
    void InitBoostLogFilter();
    void LOG(const json::object& data, const std::string_view message);

    static std::string client_ip;
}