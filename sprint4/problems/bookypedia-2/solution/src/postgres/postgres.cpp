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
}

AuthorRepositoryImpl::AuthorsInfo AuthorRepositoryImpl::GetAuthorsInfo(pqxx::work& w_)
{
    AuthorsInfo authors_info;

    auto get_author = "SELECT id, name FROM authors ORDER BY name ASC"_zv;

    for (auto [id, name] : w_.query<std::string, std::string>(get_author)) {
        authors_info.push_back(std::make_tuple(id, name));
    }

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

void AuthorRepositoryImpl::DeleteAuthor(const std::string& author_id, pqxx::work& w_)
{
    w_.exec_params(R"(DELETE FROM authors WHERE id=$1;)"_zv, author_id);
}

void AuthorRepositoryImpl::EditAuthor(const std::string& author_id, const std::string& new_name, pqxx::work& w_)
{
    w_.exec_params(R"(UPDATE authors SET name=$1 WHERE id=$2;)"_zv, new_name, author_id);
}

void BookRepositoryImpl::Save(const domain::Book& book, pqxx::work& w_) {
    w_.exec_params(
        R"(INSERT INTO books (id, author_id, title, publication_year) VALUES($1, $2, $3, $4);)"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetPublicationYear()
    );

}

void BookRepositoryImpl::EditBookTitle(const std::string& book_id, const std::string& title, pqxx::work& w_)
{
    w_.exec_params(R"(UPDATE books SET title=$1 WHERE id=$2;)"_zv, title, book_id);
}

void BookRepositoryImpl::EditBookPublYear(const std::string& book_id, const int year, pqxx::work& w_)
{
    w_.exec_params(R"(UPDATE books SET publication_year=$1 WHERE id=$2;)"_zv, year, book_id);
}

domain::BookRepository::AllBooksInfo BookRepositoryImpl::GetBooksByTitle(const std::string& title, pqxx::work& w_)
{
    AllBooksInfo books_info;

    auto get_all_books = "SELECT b.id, b.author_id, b.title, b.publication_year FROM books b JOIN authors a ON b.author_id = a.id WHERE b.title=" 
        + w_.quote(title) + " ORDER BY b.title ASC, a.name ASC, b.publication_year ASC";

    for (auto [id, author_id, title, publication_year] : w_.query<std::string, std::string, std::string, int>(get_all_books)) {

        books_info.push_back(std::make_tuple(id, author_id, title, publication_year));
    }

    return books_info;
}

domain::BookRepository::AuthorBooksInfo BookRepositoryImpl::GetBooksByAuthor(const std::string& author_id, pqxx::work& w_) {
    AuthorBooksInfo books_info;

    auto get_book = "SELECT title, publication_year FROM books WHERE author_id=" + w_.quote(author_id) + " ORDER BY publication_year ASC";

    for (auto [title, publication_year] : w_.query<std::string, int>(get_book)) {
        books_info.push_back(std::make_tuple(title, publication_year));
    }

    return books_info;
}

std::string BookRepositoryImpl::GetBookIdByAuthorIdAndTitle(const std::string& author_id, const std::string& title, pqxx::work& w_)
{
    try {
        auto get_id = "SELECT id from books WHERE author_id=" + w_.quote(author_id) + " AND title=" + w_.quote(title);
        return w_.query_value<std::string>(get_id);
    }
    catch (pqxx::sql_error& e) {
        std::cout << e.what() << std::endl;
    }
}

domain::BookRepository::AllBooksInfo BookRepositoryImpl::GetAllBooks(pqxx::work& w_)
{
    AllBooksInfo all_books_info;

    auto get_all_books = R"(
    SELECT b.id, b.author_id, b.title, b.publication_year
    FROM books b
    JOIN authors a ON b.author_id = a.id
    ORDER BY b.title ASC, a.name ASC, b.publication_year ASC
)"_zv;

    for (auto [id, author_id, title, publication_year] : w_.query<std::string, std::string, std::string, int>(get_all_books)) {

        all_books_info.push_back(std::make_tuple(id, author_id, title, publication_year));
    }

    return all_books_info;
}

void BookRepositoryImpl::DeleteBook(const std::string& book_id, pqxx::work& w_)
{
    w_.exec_params(R"(DELETE FROM books WHERE id=$1)"_zv, book_id);
}

void BookTagRepositoryImpl::Save(const std::vector<domain::BookTag>& book_tags, pqxx::work& w_)
{
    for (auto& tag : book_tags)
    {
        w_.exec_params(R"(INSERT INTO book_tags (book_id, tag) VALUES ($1, $2))"_zv, tag.GetBookId().ToString(), tag.GetTag());
    }
}

std::vector<std::string> BookTagRepositoryImpl::GetBookTags(const std::string& book_id, pqxx::work& w_)
{
    std::vector<std::string> tags;

    auto get_tags = "SELECT tag FROM book_tags WHERE book_id=" + w_.quote(book_id) + " ORDER BY tag ASC";

    for (auto [tag] : w_.query<std::string>(get_tags)) {
        tags.push_back(tag);
    }

    return tags;
}

void BookTagRepositoryImpl::EditTags(const std::string& book_id, const std::unordered_set<std::string>& tags, pqxx::work& w_)
{
    DeleteBookTags(book_id, w_);

    for (auto& tag : tags)
    {
        w_.exec_params(R"(INSERT INTO book_tags (book_id, tag) VALUES ($1, $2))"_zv, book_id, tag);
    }
}

void BookTagRepositoryImpl::DeleteBookTags(const std::string book_id, pqxx::work& w_)
{
    w_.exec_params(R"(DELETE FROM book_tags WHERE book_id=$1)"_zv, book_id);
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
    title varchar(100) NOT NULL,
    publication_year integer
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id UUID,
    tag varchar(30) NOT NULL,
    CONSTRAINT book_tag_unique UNIQUE (book_id, tag)
);)"_zv);


    // коммитим изменения
    work.commit();
}


}  // namespace postgres
