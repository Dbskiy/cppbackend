#pragma once

#include <filesystem>
#include <boost/beast/http.hpp>
#include <string>
#include <sstream>
//#include "request_handler.h"

namespace fs = std::filesystem;
namespace beast = boost::beast;
namespace http = beast::http;
namespace sys = boost::system;
using namespace std::literals;

namespace static_content {

    std::string URL_encode(const std::string&);


    std::string URL_decode(std::string_view);

    bool IsSubPath(fs::path, fs::path);

    std::string DecToHex(int);

    int HexToDec(std::string_view);

    std::string_view ToValidStaticFilePath(std::string_view);

}