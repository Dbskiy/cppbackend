#pragma once

#include <string>
#include <sstream>
#include <unordered_set>
#include <iomanip>

/*
 * URL-кодирует строку str.
 * Пробел заменяется на +,
 * Символы, отличные от букв английского алфавита, цифр и -._~ а также зарезервированные символы
 * заменяются на их %-кодированные последовательности.
 * Зарезервированные символы: !#$&'()*+,/:;=?@[]
 */
std::string DecToHex(int ascii_code);
std::string UrlEncode(std::string_view str);
