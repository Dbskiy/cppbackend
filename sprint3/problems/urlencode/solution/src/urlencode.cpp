#include "urlencode.h"


std::string DecToHex(int ascii_code)
{
    std::stringstream ss; 
    if (ascii_code < 10) {
        ss << std::hex << 0 << ascii_code;
    }
    else {
        ss << std::hex << ascii_code;
    }
    return ss.str();
}

std::string UrlEncode(std::string_view str) {
    std::stringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    // Набор зарезервированных символов
    std::unordered_set<char> reservedChars = {
        '!', '#', '$', '&', '\'', '(', ')', '*', '+', ',', '/',
        ':', ';', '=', '?', '@', '[', ']'
    };

    for (char ch : str) {
        if (isalnum(static_cast<unsigned char>(ch)) ||
            ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            // Оставляем символы без изменений
            encoded << ch;
        }
        else if (ch == ' ') {
            // Пробел кодируется как +
            encoded << '+';
        }
        else if (reservedChars.find(ch) != reservedChars.end() ||
            static_cast<unsigned char>(ch) < 32 ||
            static_cast<unsigned char>(ch) >= 128) {
            // Зарезервированные символы или символы вне диапазона ASCII кодируем как %XX
            encoded << '%' << std::setw(2) << std::uppercase << static_cast<int>(static_cast<unsigned char>(ch));
        }
        else {
            // Прочие символы оставляем без изменений
            encoded << ch;
        }
    }

    return encoded.str();
}
/*
 * URL-кодирует строку str.
 * Пробел заменяется на +,
 * Символы, отличные от букв английского алфавита, цифр и -._~ а также зарезервированные символы
 * заменяются на их %-кодированные последовательности.
 * Зарезервированные символы: !#$&'()*+,/:;=?@[]
 */