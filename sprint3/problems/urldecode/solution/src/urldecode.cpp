#include "urldecode.h"

#include <string>
#include <sstream>
#include <charconv>
#include <stdexcept>

int HexToDec(std::string_view hex_str)
{
    return std::stoi(std::string(hex_str.data(), hex_str.size()), nullptr, 16);
}

bool CanBeDecoded(std::string_view hex_str) {
    int num = HexToDec(hex_str);

    if (num == 45 || num == 46 || num == 95 || num == 126 || (num >= 48 && num <= 57) ||
        (num >= 65 && num <= 90) || (num >= 97 && num <= 122)) return false;

    return true;
}

std::string UrlDecode(std::string_view str) {
    std::stringstream decoded_URL;
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == '%' && i + 2 < str.size() && CanBeDecoded(str.substr(i + 1, 2)))  {
            decoded_URL << static_cast<char>(HexToDec(str.substr(i + 1, 2)));
            i += 2;
        }
        else if (str[i] == '+') decoded_URL << ' ';
        else decoded_URL << str[i];
    }

    return decoded_URL.str();
}
