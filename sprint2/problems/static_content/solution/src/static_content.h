#pragma once

#include <filesystem>
#include <boost/beast/http.hpp>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

namespace static_content {

    std::string URL_encode(const std::string&);


    std::string URL_decode(std::string_view);

    bool IsSubPath(fs::path, fs::path);

    std::string DecToHex(int);

    int HexToDec(std::string_view);

}