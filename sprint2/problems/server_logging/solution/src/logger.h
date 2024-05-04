#pragma once
#include "sdk.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/json.hpp>
#include <boost/date_time.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>


namespace net = boost::asio;
namespace beast = boost::beast;
namespace logging = boost::log;
namespace json = boost::json;

BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

namespace logger
{
    json::object GetReqData(const std::string& client_ip, const std::string_view uri, const auto& method);

    //функции для формирования json-объекта data
    
    void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);

}