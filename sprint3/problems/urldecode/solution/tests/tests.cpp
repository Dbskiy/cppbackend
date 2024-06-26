#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    //Пустая строка
    BOOST_TEST(UrlDecode(""sv) == ""s);

    //Строка без %-последовательностей
    BOOST_TEST(UrlDecode("Hello World !"sv) == "Hello World !"s);

    //Строка с валидными %-последовательностями, записанными в разном регистре
    BOOST_TEST(UrlDecode("Hello%20World%20%21"sv) == "Hello World !"s);

    //Строка с невалидными % -последовательностями
    BOOST_TEST(UrlDecode("%48ello World !"sv) != "Hello World !"s);

    //Строка с символом +
    BOOST_TEST(UrlDecode("Hello+World+!"sv) == "Hello World !"s);


}