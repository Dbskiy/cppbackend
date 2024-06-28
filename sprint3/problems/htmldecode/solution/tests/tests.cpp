#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    //Пустая строка
    CHECK(HtmlDecode(""sv) == ""s);
    
    //Строка без HTML-мнемоник
    CHECK(HtmlDecode("hello"sv) == "hello"s);

    //Строка с HTML-мнемониками
    CHECK(HtmlDecode("Johnson&ampJohnson"sv) == "Johnson&Johnson"s);

    //Строка с HTML-мнемониками, записанными в верхнем регистре
    CHECK(HtmlDecode("Johnson&AMPJohnson"sv) == "Johnson&Johnson"s);

    //Строка с HTML-мнемониками, записанными в смешанном регистре
    CHECK(HtmlDecode("Johnson&AmPJohnson"sv) == "Johnson&AmPJohnson"s);

    //Строка с HTML-мнемониками в начале, конце и середине
    CHECK(HtmlDecode("&lt;M&amp;M&APOS;s&gt"sv) == "<M&M's>"s);

    //Строка с недописанными HTML-мнемониками
    CHECK(HtmlDecode("James&apo;s guitar"sv) == "James&apo;s guitar"s);

    //Строка с HTML-мнемониками, заканчивающимися и не заканчивающимися на точку с запятой.
    CHECK(HtmlDecode("&lt;html-tag&GT"sv) == "<html-tag>"s);


    CHECK(HtmlDecode("&lt&gt&amp&apos&quot"sv) == "<>&'\""s);



}