#pragma once

#include <string>

/*
Возвращает URL-декодированное представление строки str.
Пример: "Hello+World%20%21" должна превратиться в "Hello World !"
В случае ошибки выбрасывает исключение std::invalid_argument
*/
int HexToDec(std::string_view hex_str);
bool CanBeDecoded(std::string_view hex_str);
std::string UrlDecode(std::string_view str);
