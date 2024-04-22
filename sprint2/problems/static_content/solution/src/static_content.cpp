#include "static_content.h"
#include <iostream>

namespace static_content {
    std::string URL_encode(const std::string& path)
    {
        std::stringstream encoded_URL;
        for (size_t i = 0; i < path.size(); i++) {
            if (isalnum(path[i]) || path[i] == '-' || path[i] == '_' || path[i] == '.' || path[i] == '~') {
                encoded_URL << path[i];
            }
            else {
                encoded_URL << '%';
                //преобразование const char в код символа в таблице ascii
                encoded_URL << DecToHex(static_cast<int>(path[i]));
            }
        }
        return encoded_URL.str();
    }

    std::string URL_decode(std::string_view path)
    {
        std::stringstream decoded_URL;
        for (size_t i = 0; i < path.size(); i++) {
            if (path[i] == '%' && i + 2 < path.size()) {
                decoded_URL << static_cast<char>(HexToDec(path.substr(i + 1, 2)));
                i += 2;
            }
            else if (path[i] == '+') decoded_URL << ' ';
            else decoded_URL << path[i];
        }

        return decoded_URL.str();
    }

    bool IsSubPath(fs::path path, fs::path base)
    {
        // Приводим оба пути к каноничному виду (без . и ..)
        path = fs::weakly_canonical(path);

        base = fs::weakly_canonical(base);

        std::cout << "Root path: " << base.string() << std::endl;


        // Проверяем, что все компоненты base содержатся внутри path
        for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
            if (p == path.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }

    std::string DecToHex(int ascii_code)
    {
        std::stringstream ss;
        ss << std::hex << ascii_code;
        return ss.str();
    }

    int HexToDec(std::string_view hex_str)
    {
        return std::stoi(std::string(hex_str.data(), hex_str.size()), nullptr, 16);
    }
}