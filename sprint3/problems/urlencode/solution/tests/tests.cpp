#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {

    //Пустая входная строка
    EXPECT_EQ(UrlEncode(""sv), ""s);

    //Входная строка без служебных символов
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);

    //Входная строка со служебными символами
    EXPECT_EQ(UrlEncode("abc*"sv), "abc%2A"s)
        ;
    EXPECT_EQ(UrlEncode("!#$&'()*+,/:;=?@[]"sv), "%21%23%24%26%27%28%29%2A%2B%2C%2F%3A%3B%3D%3F%40%5B%5D"s);

    //Входная строка с пробелами
    EXPECT_EQ(UrlEncode("Hello World !"sv), "Hello+World+%21"s);

    // Входная строка с символами с кодами меньше 32
    EXPECT_EQ(UrlEncode("Hello World\n"sv), "Hello+World%0A"s);

    // Входная строка с символами с кодами больше или равными 128
    EXPECT_EQ(UrlEncode("\x80\x81\xFFhello"sv), "%80%81%FFhello"s);


    
}

/* Напишите остальные тесты самостоятельно */
