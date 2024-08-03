#pragma once

#include <string>
#include <vector>

namespace app {

class UseCases {
public:
    using AuthorsInfo = std::vector<std::tuple<std::string, std::string>>;
    using BooksInfo = std::vector<std::tuple<std::string, int>>;

    virtual void AddAuthor(const std::string& name) = 0;

    virtual AuthorsInfo GetAuthorsInfo() = 0;

    virtual void AddBook(const std::string& author_id, const std::string& title, const int publication_year) = 0;

    virtual BooksInfo GetBooksByAuthorInfo(const std::string& author_id) = 0;

    virtual BooksInfo GetAllBooks() = 0;
    //остальные команды

protected:
    ~UseCases() = default;
};

}  // namespace app
