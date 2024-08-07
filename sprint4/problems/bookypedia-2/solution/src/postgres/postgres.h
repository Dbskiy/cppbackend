#pragma once
#include <iostream>
#include <tuple>
#include <optional>

#include <pqxx/pqxx>
#include <pqxx/connection>
#include <pqxx/transaction>
#include "../domain/author.h"
#include "../domain/book.h"
#include "../app/unit_of_work_factory.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    void Save(const domain::Author& author, pqxx::work& w_) override;

    AuthorsInfo GetAuthorsInfo(pqxx::work& w_) override;

    std::optional<std::string> GetAuthorIdByName(const std::string& name, pqxx::work& w_) override;

    std::optional<std::string> GetAuthorNameById(const std::string& id, pqxx::work& w_) override;

};

class BookRepositoryImpl : public domain::BookRepository {
public:

    void Save(const domain::Book& book, pqxx::work& w_) override;

    AuthorBooksInfo GetBooksByAuthor(const std::string& author_id, pqxx::work& w_) override;

    AllBooksInfo GetAllBooks(pqxx::work& w_) override;

};

class UnitOfWorkImpl : public app::UnitOfWork {
public:

    UnitOfWorkImpl(pqxx::connection& connection, AuthorRepositoryImpl& authors, BookRepositoryImpl& books)
        : connection_(connection)
        ,authors_(authors)
        , books_(books)
    {

    }

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

    UnitOfWork::AuthorBooksInfo GetBooksByAuthor(const std::string& author_id) override {
        return books_.GetBooksByAuthor(author_id, w_);
    }

    UnitOfWork::AllBooksInfo GetAllBooks() override {
        return books_.GetAllBooks(w_);
    }

private:
    pqxx::connection& connection_;
    pqxx::work w_{connection_};
    AuthorRepositoryImpl& authors_;
    BookRepositoryImpl& books_;
};

class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
public: 
    explicit UnitOfWorkFactoryImpl(pqxx::connection& connection) : connection_(connection) {}

    std::unique_ptr<app::UnitOfWork> CreateUnitOfWork() override {
        return std::make_unique<UnitOfWorkImpl>(connection_, authors_, books_);
    }

private:
    pqxx::connection& connection_;
    AuthorRepositoryImpl authors_;
    BookRepositoryImpl books_;
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