#pragma once
#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <tuple>
#include <optional>
#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    using AuthorInfo = std::tuple<std::string, std::string>;
    using AuthorsInfo = std::vector<AuthorInfo>;

    virtual void Save(const Author& author, pqxx::work& w_) = 0;
    virtual AuthorsInfo GetAuthorsInfo(pqxx::work& w_) = 0;
    virtual std::optional<std::string> GetAuthorIdByName(const std::string& name, pqxx::work& w_) = 0;
    virtual std::optional<std::string> GetAuthorNameById(const std::string& id, pqxx::work& w_) = 0;
    virtual void DeleteAuthor(const std::string& author_id, pqxx::work& w_) = 0;
    virtual void EditAuthor(const std::string& author_id, const std::string & new_name, pqxx::work& w_) = 0;

protected:
    ~AuthorRepository() = default;
};

}  // namespace domain
