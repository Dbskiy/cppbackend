#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include <unordered_set>

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

struct BookInfo {
    std::string id;
    std::string title;
    int publication_year;
};

struct BookTags {
    std::string id;
    std::vector<std::string> tags;
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool ShowBooks() const;
    bool ShowAuthorBooks() const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool ShowBook(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::optional<std::string> GetAuthorInfo(std::istream& cmd_input) const;
    std::optional<std::string> SelectAuthorFromList() const;
    std::optional<std::string> SelectAuthor() const;
    std::optional<std::tuple<std::string, detail::BookInfo>> SelectBookFromList(const std::vector<std::tuple<std::string, detail::BookInfo>>& books) const;
    std::optional<std::tuple<std::string, detail::BookInfo>> SelectBook(std::istream& cmd_input) const;
    std::optional<std::string> EnterNewTitle(const std::string& cur_title) const;
    std::optional<int> EnterNewPublYear(const int year) const;
    std::optional<std::unordered_set<std::string>> EnterTags() const; 
    std::vector<detail::AuthorInfo> GetAuthors() const;
    std::vector<std::tuple<std::string, detail::BookInfo>> GetBooks() const;
    std::vector<detail::BookInfo> GetAuthorBooks(const std::string& author_id) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui