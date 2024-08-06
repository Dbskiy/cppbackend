#pragma once
#include <string>
#include <tuple>
#include <optional>
#include <vector>

namespace app {

class UseCases {
public:
    using AuthorInfo = std::tuple<std::string, std::string>;
    using AuthorsInfo = std::vector<AuthorInfo>;
    using AuthorBooksInfo = std::vector<std::tuple<std::string, int>>;
    using AllBooksInfo = std::vector<std::tuple<std::string, std::string, int>>;

    virtual void SetNewUnitOfWork() = 0;

    virtual void AcceptChanges() = 0;

    virtual void AddAuthor(const std::string& name) = 0;

    virtual AuthorsInfo GetAuthorsInfo() = 0;

    virtual std::optional<std::string> GetAuthorIdByName(const std::string& name) = 0;
    virtual std::optional<std::string> GetAuthorNameById(const std::string& id) = 0;

    virtual void AddBook(const std::string& author_id, const std::string& title, const int publication_year) = 0;

    virtual AuthorBooksInfo GetBooksByAuthorInfo(const std::string& author_id) = 0;

    virtual AllBooksInfo GetAllBooks() = 0;

    virtual ~UseCases() = default;
};

}  // namespace app
