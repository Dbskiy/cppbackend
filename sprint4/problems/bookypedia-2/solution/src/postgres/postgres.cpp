#include "postgres.h"

#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author, pqxx::work& w_) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    w_.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    //w_->commit();
}

AuthorRepositoryImpl::AuthorsInfo AuthorRepositoryImpl::GetAuthorsInfo(pqxx::work& w_)
{
    AuthorsInfo authors_info;

    auto get_author = "SELECT id, name FROM authors ORDER BY name ASC"_zv;
    //pqxx::read_transaction r(connection_);

    for (auto [id, name] : w_.query<std::string, std::string>(get_author)) {
        authors_info.push_back(std::make_tuple(id, name));
    }

    //w_->commit();
    return authors_info;
}



std::optional<std::string> AuthorRepositoryImpl::GetAuthorIdByName(const std::string& name, pqxx::work& w_)
{
    auto result = w_.query01<std::string, std::string>(
        "SELECT id, name FROM authors WHERE name=" + w_.quote(name) + " LIMIT 1;");

    if (result.has_value()) {
        return std::get<0>(*result);
    }
    return std::nullopt;
}

std::optional<std::string> AuthorRepositoryImpl::GetAuthorNameById(const std::string& id, pqxx::work& w_)
{
    auto result = w_.query01<std::string, std::string>(
        "SELECT id, name FROM authors WHERE id=" + w_.quote(id) + " LIMIT 1;");

    if (result.has_value()) {
        return std::get<1>(*result);
    }
    return std::nullopt;
}

void BookRepositoryImpl::Save(const domain::Book& book, pqxx::work& w_) {
    w_.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES($1, $2, $3, $4);)"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear()
    );

    //w_->.commit();
}

domain::BookRepository::AuthorBooksInfo BookRepositoryImpl::GetBooksByAuthor(const std::string& author_id, pqxx::work& w_) {
    AuthorBooksInfo books_info;

    //pqxx::read_transaction r(connection_);

    auto get_book = "SELECT title, publication_year FROM books WHERE author_id=" + w_.quote(author_id) + " ORDER BY publication_year ASC";

    for (auto [title, publication_year] : w_.query<std::string, int>(get_book)) {
        books_info.push_back(std::make_tuple(title, publication_year));
    }

    //w_->.commit();
    return books_info;
}

domain::BookRepository::AllBooksInfo BookRepositoryImpl::GetAllBooks(pqxx::work& w_)
{
    AllBooksInfo all_books_info;

    auto get_all_books = R"(
    SELECT b.author_id, b.title, b.publication_year
    FROM books b
    JOIN authors a ON b.author_id = a.author_id
    ORDER BY b.title ASC, a.name ASC, b.publication_year ASC
)"_zv;

    //pqxx::read_transaction r(connection_);

    for (auto [author_id, title, publication_year] : w_.query<std::string, std::string, int>(get_all_books)) {

        all_books_info.push_back(std::make_tuple(author_id, title, publication_year));
    }
    //w_->.commit();

    return all_books_info;
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    // Таблица books
    work.exec(R"(CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL, 
    title varchar(100) UNIQUE NOT NULL,
    publication_year integer
);
)"_zv);

    // коммитим изменения
    work.commit();
}

}  // namespace postgres
