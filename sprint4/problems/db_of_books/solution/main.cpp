#include <iostream>
#include <optional>
#include <string>
#include <boost/json.hpp>
#include <pqxx/pqxx>

namespace json = boost::json;
using namespace std::literals;
// libpqxx использует zero-terminated символьные литералы вроде "abc"_zv;
using pqxx::operator"" _zv;

int main(int argc, const char* argv[]) {
    try {
        if (argc == 1) {
            std::cout << "Usage: connect_db <conn-string>\n"sv;
            return EXIT_SUCCESS;
        }
        else if (argc != 2) {
            std::cerr << "Invalid command line\n"sv;
            return EXIT_FAILURE;
        }
        //// Подключаемся к БД, указывая её параметры в качестве аргумента
        pqxx::connection conn{ argv[1] };

        pqxx::work w(conn);

        // Используя транзакцию создадим таблицу в выбранной базе данных:
        w.exec(
            "CREATE TABLE IF NOT EXISTS books (id SERIAL PRIMARY KEY, title varchar(100) NOT NULL, author varchar(100) NOT NULL, year integer NOT NULL, ISBN varchar(13) UNIQUE);"_zv);

	w.commit();

        constexpr auto tag_add_book = "add_book";
        conn.prepare(tag_add_book, "INSERT INTO books (title, author, year, ISBN) VALUES ($1, $2, $3, $4)"_zv);

        auto all_books = "SELECT id, title, author, year, ISBN FROM books ORDER BY year DESC, title ASC, author ASC, ISBN ASC"_zv;


        // Создаём транзакцию. Это понятие будет разобрано в следующих уроках.
        // Транзакция нужна, чтобы выполнять запросы.
        while (true) {
            std::string req;
            std::getline(std::cin, req);

            auto json_req = json::parse(req);

            auto& action = json_req.at("action").as_string();

            if (action == "add_book") {
                json::object result;
		        pqxx::work c(conn);
                try {
                    auto& payload = json_req.at("payload");
                    std::string title = payload.at("title").as_string().c_str();
                    std::string author = payload.at("author").as_string().c_str();
                    int year = payload.at("year").as_int64();
                    std::optional<std::string> ISBN;
                    if (!payload.at("ISBN").is_null()) {
                        ISBN = payload.at("ISBN").as_string().c_str();
                    }
                    if (ISBN) {
                        c.exec_prepared(tag_add_book, title, author, year, ISBN.value()); ////Вставка в таблицу
                    }
                    else {
                        c.exec_prepared(tag_add_book, title, author, year, nullptr); ////Вставка в таблицу
                    }
                    result["result"] = true;
                }
                catch (...) {
                    result["result"] = false;
                }
		c.commit();
                std::cout << json::serialize(result) << std::endl;
            }
            else if (action == "all_books") {
                pqxx::read_transaction r(conn);
                json::array books;
                for (auto [id, title, author, year, ISBN] : r.query<int, std::string, std::string, int, std::optional<std::string>> (all_books)) {
                    json::object book;
                    book["id"] = id;
                    book["title"] = title;
                    book["author"] = author;
                    book["year"] = year;
		    if (ISBN) {
		        book["ISBN"] = ISBN.value();		
		    }
		    else {
		        book["ISBN"];
		    }
                    //book["ISBN"] = ISBN.value_or("null");

                    books.push_back(book);
                }
                std::cout << json::serialize(books) << std::endl;
            }
            else if (action == "exit") {
                break;
            }

        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
