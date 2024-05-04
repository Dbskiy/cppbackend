#include "logger.h"

namespace logger
{
	json::object GetReqData(const std::string& client_ip, const std::string_view uri, const auto& method)
	{
		json::object data;
		data["ip"] = client_ip;
		data["URI"] = uri;
		data["method"] = method;

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

}


