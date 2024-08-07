#pragma once
#include <string>
#include <tuple>
#include <optional>
#include <vector>
#include <unordered_set>

namespace app {

class UseCases {
public:
    using AuthorInfo = std::tuple<std::string, std::string>;
    using AuthorsInfo = std::vector<AuthorInfo>;
    using AuthorBooksInfo = std::vector<std::tuple<std::string, int>>;
    using AllBooksInfo = std::vector<std::tuple<std::string, std::string, std::string, int>>;
    using BookTags = std::optional<std::unordered_set<std::string>>;

    virtual void SetNewUnitOfWork() = 0;

    virtual void AcceptChanges() = 0;

    virtual void AddAuthor(const std::string& name) = 0;

    virtual AuthorsInfo GetAuthorsInfo() = 0;

    virtual std::optional<std::string> GetAuthorIdByName(const std::string& name) = 0;
    virtual std::optional<std::string> GetAuthorNameById(const std::string& id) = 0;

    virtual void AddBook(BookTags& tags, const std::string& author_id, const std::string& title, const int publication_year) = 0;

    virtual void DeleteBook(const std::string& book_id) = 0;

    virtual void AddBookTags(const std::string& book_id, BookTags& tags) = 0;

    virtual std::vector<std::string> GetBookTags(const std::string& book_id) = 0;

    virtual AllBooksInfo GetBooksByTitle(const std::string& title) = 0;

    virtual AuthorBooksInfo GetBooksByAuthorInfo(const std::string& author_id) = 0;

    virtual AllBooksInfo GetAllBooks() = 0;

    virtual void DeleteAuthor(const std::string& author_id) = 0;

    virtual void EditAuthor(const std::string& author_id, const std::string& new_name) = 0;

    virtual void EditBook(const std::string& book_id, std::optional<std::string>& title, std::optional<int>& year, std::optional<std::unordered_set<std::string>>& tags) = 0;

    virtual ~UseCases() = default;
};

}  // namespace app
