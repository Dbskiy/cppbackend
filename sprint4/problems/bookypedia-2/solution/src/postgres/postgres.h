#pragma once
#include <iostream>
#include <tuple>
#include <optional>

#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/transaction>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/book_tags.h"
#include "../app/unit_of_work_factory.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    void Save(const domain::Author& author, pqxx::work& w_) override;

    AuthorsInfo GetAuthorsInfo(pqxx::work& w_) override;

    std::optional<std::string> GetAuthorIdByName(const std::string& name, pqxx::work& w_) override;

    std::optional<std::string> GetAuthorNameById(const std::string& id, pqxx::work& w_) override;

    void DeleteAuthor(const std::string& author_id, pqxx::work& w_) override;

    void EditAuthor(const std::string& author_id, const std::string& new_name, pqxx::work& w_) override;

};

class BookRepositoryImpl : public domain::BookRepository {
public:

    void Save(const domain::Book& book, pqxx::work& w_) override;

    void EditBookTitle(const std::string& book_id, const std::string& title, pqxx::work& w_) override;
    void EditBookPublYear(const std::string& book_id, const int year, pqxx::work& w_) override;

    AllBooksInfo GetBooksByTitle(const std::string& title, pqxx::work& w_) override;

    AuthorBooksInfo GetBooksByAuthor(const std::string& author_id, pqxx::work& w_) override;
    std::string GetBookIdByAuthorIdAndTitle(const std::string& author_id, const std::string& title, pqxx::work& w_) override;

    AllBooksInfo GetAllBooks(pqxx::work& w_) override;

    void DeleteBook(const std::string& book_id, pqxx::work& w_) override;

};

class BookTagRepositoryImpl : public domain::BookTagRepository {
public:
    void Save(const std::vector<domain::BookTag>& book_tags, pqxx::work& w_) override;

    std::vector<std::string> GetBookTags(const std::string& book_id, pqxx::work& w_) override;
    void EditTags(const std::string& book_id, const std::set<std::string>& tags, pqxx::work& w_) override;

    void DeleteBookTags(const std::string book_id, pqxx::work& w_) override;


};

class UnitOfWorkImpl : public app::UnitOfWork {
public:

    UnitOfWorkImpl(pqxx::connection& connection, AuthorRepositoryImpl& authors, BookRepositoryImpl& books, BookTagRepositoryImpl& books_tags)
        : connection_(connection)
        ,authors_(authors)
        , books_(books)
        , books_tags_(books_tags) {}

    void Commit() override {
        try {
            w_.commit();
        }
        catch (pqxx::sql_error& e) {
            throw e.what();
        }
    }

    domain::AuthorRepository& Authors() override {
        return authors_;
    }

    domain::BookRepository& Books() override {
        return books_;
    }

    void SaveAuthor(const domain::Author& author) override {
        authors_.Save(author, w_);
    }

    UnitOfWork::AuthorsInfo GetAuthorsInfo() override {
        return authors_.GetAuthorsInfo(w_);
    }

    std::optional<std::string> GetAuthorIdByName(const std::string& name) override {
        return authors_.GetAuthorIdByName(name, w_);
    }

    std::optional<std::string> GetAuthorNameById(const std::string& id) override {
        return authors_.GetAuthorNameById(id, w_);
    }

    void SaveBook(const domain::Book& book) override {
        books_.Save(book, w_);
    }

    UnitOfWork::AllBooksInfo GetBooksByTitle(const std::string& title) override {
        return books_.GetBooksByTitle(title, w_);
    }

    UnitOfWork::AuthorBooksInfo GetBooksByAuthor(const std::string& author_id) override {
        return books_.GetBooksByAuthor(author_id, w_);
    }

    UnitOfWork::AllBooksInfo GetAllBooks() override {
        return books_.GetAllBooks(w_);
    }

    void SaveBookTags(std::vector<domain::BookTag>& tags) override {
        books_tags_.Save(tags, w_);
    }

    std::vector<std::string> GetBookTags(const std::string& book_id) override {
        return books_tags_.GetBookTags(book_id, w_);
    }

    void DeleteBook(const std::string& book_id) override {
        books_tags_.DeleteBookTags(book_id, w_);
        books_.DeleteBook(book_id, w_);
    }

    void DeleteAuthor(const std::string& author_id) override {
        DeleteAuthorBooks(author_id);
        authors_.DeleteAuthor(author_id, w_);
    }

    void EditAuthor(const std::string& author_id, const std::string& new_name) override {
        authors_.EditAuthor(author_id, new_name, w_);
    }

    void EditBook(const std::string& book_id, std::optional<std::string>& title, std::optional<int>& year, std::optional<std::set<std::string>>& tags) override {
        if (title) {
            books_.EditBookTitle(book_id, *title, w_);
        }
        if (year) {
            books_.EditBookPublYear(book_id, *year, w_);
        }

        if (tags) {
            books_tags_.EditTags(book_id, *tags, w_);
        }
        else {
            books_tags_.DeleteBookTags(book_id, w_);
        }
    }

private:
    void DeleteAuthorBooks(const std::string& author_id) {
        auto books_info = books_.GetBooksByAuthor(author_id, w_);

        for (auto& book_info : books_info) {
            auto book_id = books_.GetBookIdByAuthorIdAndTitle(author_id, std::get<0>(book_info), w_);
            books_tags_.DeleteBookTags(book_id, w_);
            books_.DeleteBook(book_id, w_);
        }
    }
    pqxx::connection& connection_;
    pqxx::work w_{connection_};
    AuthorRepositoryImpl& authors_;
    BookRepositoryImpl& books_;
    BookTagRepositoryImpl& books_tags_;
};

class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
public: 
    explicit UnitOfWorkFactoryImpl(pqxx::connection& connection) : connection_(connection) {}

    std::unique_ptr<app::UnitOfWork> CreateUnitOfWork() override {
        return std::make_unique<UnitOfWorkImpl>(connection_, authors_, books_, books_tags_);
    }

private:
    pqxx::connection& connection_;
    AuthorRepositoryImpl authors_;
    BookRepositoryImpl books_;
    BookTagRepositoryImpl books_tags_;
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    UnitOfWorkFactoryImpl& GetUnitOfWorkFactory() & {
        return unit_of_work_impl_;
    }

private:
    pqxx::connection connection_;

    UnitOfWorkFactoryImpl unit_of_work_impl_{connection_};
};

}  // namespace postgres
