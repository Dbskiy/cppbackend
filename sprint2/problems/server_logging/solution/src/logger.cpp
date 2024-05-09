#include "logger.h"

#include <boost/log/expressions.hpp>
//#include <boost/log/utility/manipulators/add_value.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/utility/setup/console.hpp>
//#include <boost/json.hpp>
#include <boost/date_time.hpp>

namespace logger
{
	BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime);
	BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value);

	json::object GetReqData(const std::string& client_ip, const std::string_view uri, http::verb method)
	{
		std::string str_method = std::string(http::to_string(method));
		json::object data;
		data["ip"] = client_ip;
		data["URI"] = std::string(uri.data(), uri.size());
		data["method"] = str_method;
		return data;
	}

	json::object GetResData(const milliseconds& response_time, const int& code, const std::string_view content_type)
	{
		json::object data;
		data["response_time"] = response_time.count();
		data["code"] = code;
		if (content_type == "") {
			data["content_type"] = NULL;
		}
		else {
			data["content_type"] = std::string(content_type.data(), content_type.size());
		}

		return data;
	}

	json::object GetServerStartData(const net::ip::port_type port, const std::string& host_ip)
	{
		json::object data;
		data["port"] = port;
		data["address"] = host_ip;
			
		return data;
	}

	json::object GetServerStopData(const int& code)
	{
		json::object data;

		data["code"] = code;

		return data;
	}

	json::object GetServerStopData(const int& code, const std::exception& ex)
	{
		json::object data;

		data["code"] = code;
		data["exception"] = ex.what();

		return data;
	}

	json::object GetErrorData(const beast::error_code& code, const std::string_view where)
	{
		json::object data;

		data["code"] = code.value();
		data["text"] = code.message();
		data["where"] = std::string(where.data(), where.size());

		return data;
	}

	void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm)
	{
		json::object log;

		auto ts = *rec[timestamp];

		log["timestamp"] = to_iso_extended_string(ts);

		auto data = rec[additional_data];

		log["data"] = data.get();

		log["message"] = *rec[logging::expressions::smessage];

		strm << log;
	}

	void InitBoostLogFilter()
	{
		logging::add_common_attributes();


		logging::add_console_log(
			std::clog,
			logging::keywords::format = &logger::MyFormatter,
			logging::keywords::auto_flush = true
		);
	}

	void LOG(const json::object& data, const std::string_view message)
	{
		BOOST_LOG_TRIVIAL(info)
			<< logging::add_value(logger::additional_data, data)
			<< message;
	}

}


