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
    void Save(const domain::Author& author) override;

    AuthorsInfo GetAuthorsInfo() override;

    std::optional<std::string> GetAuthorIdByName(const std::string& name) override;

    std::optional<std::string> GetAuthorNameById(const std::string& id) override;

    void SetNewTransaction(pqxx::work* w) {
        w_ = w;
    }

private:
    pqxx::work* w_{};
};

class BookRepositoryImpl : public domain::BookRepository {
public:

    void Save(const domain::Book& book) override;

    AuthorBooksInfo GetBooksByAuthor(domain::AuthorId& author_id) override;

    AllBooksInfo GetAllBooks() override;

    void SetNewTransaction(pqxx::work* w) {
        w_ = w;
    }

private:
    pqxx::work* w_{};
};

class UnitOfWorkImpl : public app::UnitOfWork {
public:
    UnitOfWorkImpl(AuthorRepositoryImpl& authors, BookRepositoryImpl& books, pqxx::work* w) 
        : authors_(authors)
        , books_(books)
        , w_(w) 
    {
        authors_.SetNewTransaction(w_);
        books_.SetNewTransaction(w_);
    }

    void Commit() override {
        try {
            w_->commit();
        }
        catch (...) {
            w_->abort();
        }
        w_ = nullptr;
    }

    bool isTransactionClosed() override {
        if (!w_) return true;
        return false;
    }

    domain::AuthorRepository& Authors() override {
        return authors_;
    }

    domain::BookRepository& Books() override {
        return books_;
    }

private:
    pqxx::work* w_;
    AuthorRepositoryImpl& authors_;
    BookRepositoryImpl& books_;
};

class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
public: 
    explicit UnitOfWorkFactoryImpl(pqxx::connection& connection) : connection_(connection) {}

    std::unique_ptr<app::UnitOfWork> CreateUnitOfWork() override {
        pqxx::work w{ connection_ };
        return std::make_unique<UnitOfWorkImpl>(authors_, books_, &w);
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