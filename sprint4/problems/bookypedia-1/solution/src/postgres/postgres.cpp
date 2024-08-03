#include "postgres.h"
#include "postgres.h"
#include "postgres.h"

#include <pqxx/zview.hxx>


namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

AuthorRepositoryImpl::AuthorsInfo AuthorRepositoryImpl::GetAuthorsInfo() {
    AuthorsInfo authors_info;
    
    auto get_author = "SELECT id, name FROM authors ORDER BY name ASC"_zv;
    pqxx::read_transaction r(connection_);

    for (auto [id, name] : r.query<std::string, std::string>(get_author)) {
        authors_info.push_back(std::make_tuple(id, name));
    }

    return authors_info;
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    pqxx::work work{ connection_ };

    work.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES($1, $2, $3, $4)
ON CONFLICT (id) DO UPDATE author_id=$2, title=$3, publication_year=$4;
    )"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear()
    );
    work.commit();
}

BookRepositoryImpl::BooksInfo BookRepositoryImpl::GetBooksByAuthor(domain::AuthorId& author_id) {
    BooksInfo books_info;
    
    auto get_book = "SELECT title, publication_year FROM books WHERE author_id=" + author_id.ToString() + " ORDER BY title ASC";

    pqxx::read_transaction r(connection_);

    for (auto [title, publication_year] : r.query<std::string, int>(get_book)) {
        books_info.push_back(std::make_tuple(title, publication_year));
    }

    return books_info;
}

BookRepositoryImpl::BooksInfo BookRepositoryImpl::GetAllBooks()
{
    BooksInfo all_books_info;

    auto get_all_books = "SELECT title, publication_year FROM books ORDER BY title ASC"_zv;

    pqxx::read_transaction r(connection_);

    for (auto [title, publication_year] : r.query<std::string, int>(get_all_books)) {
        all_books_info.push_back(std::make_tuple(title, publication_year));
    }

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
    integer publication_year
);
)"_zv);

    // коммитим изменения
    work.commit();
}

}  // namespace postgres