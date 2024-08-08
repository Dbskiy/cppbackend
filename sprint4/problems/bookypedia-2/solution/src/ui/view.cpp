#include "view.h"

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <iostream>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    out << book.title << ", " << book.publication_year;
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::tuple<std::string, BookInfo>& book_info) {
    out << std::get<1>(book_info).title << " by " << std::get<0>(book_info) << ", " << std::get<1>(book_info).publication_year;
    return out;
}
}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

void PrintTags(std::ostream& out, const std::vector<std::string>& tags) {
    for (int i = 0; i < tags.size(); i++)
    {
        out << tags[i];
        if (i != tags.size() - 1) {
            out << ", ";
        }
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
        // либо
        // [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
                    std::bind(&View::ShowAuthorBooks, this));
    menu_.AddAction("DeleteAuthor"s, "name"s, "Delete author"s, std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, "name"s, "Edit author"s, std::bind(&View::EditAuthor, this, ph::_1));
    menu_.AddAction("ShowBook"s, "title"s, "Show book"s, std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, "title"s, "Delete book"s, std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, "title"s, "Edit book"s, std::bind(&View::EditBook, this, ph::_1));
    
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        use_cases_.SetNewUnitOfWork();
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
	    if (name.empty()) {
		    throw std::runtime_error("Author's name is empty");
	    }   
        use_cases_.AddAuthor(std::move(name));
        use_cases_.AcceptChanges();
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        use_cases_.SetNewUnitOfWork();
        if (auto params = GetBookParams(cmd_input)) {
            output_ << "Enter tags (comma separated):" << std::endl;
            auto tags = EnterTags();
            use_cases_.AddBook(tags, params->author_id, params->title, params->publication_year);
        }
        use_cases_.AcceptChanges();
    } catch (const std::exception&) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    use_cases_.SetNewUnitOfWork();
    PrintVector(output_, GetAuthors());
    use_cases_.AcceptChanges();
    return true;
}

bool View::ShowBooks() const {
    use_cases_.SetNewUnitOfWork();
    PrintVector(output_, GetBooks());
    use_cases_.AcceptChanges();
    return true;
}

bool View::ShowAuthorBooks() const {
    // TODO: handle error
    try {
        use_cases_.SetNewUnitOfWork();
        if (auto author_id = SelectAuthorFromList()) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
        use_cases_.AcceptChanges();

    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const
{
    //assert(!"TODO: implement DeleteAuthor()");
    try {
        use_cases_.SetNewUnitOfWork();

        if (auto author_id = GetAuthorInfo(cmd_input)) {
            use_cases_.DeleteAuthor(*author_id);
        }

        use_cases_.AcceptChanges();
    }
    catch (const std::exception&) {
        throw std::runtime_error("Failed to delete author");
    }
    return true;
}

bool View::EditAuthor(std::istream& cmd_input) const
{
    try {
        use_cases_.SetNewUnitOfWork();

        if (auto author_id = GetAuthorInfo(cmd_input)) {
            output_ << "Enter new name:" << std::endl;
            std::string new_name;
            std::getline(input_, new_name);
            use_cases_.EditAuthor(*author_id, new_name);
        }
        use_cases_.AcceptChanges();
    }
    catch (const std::exception&) {
        throw std::runtime_error("Failed to edit author");
    }
    
    return true;
}

bool View::ShowBook(std::istream& cmd_input) const
{
    try {
        use_cases_.SetNewUnitOfWork();
        auto book = SelectBook(cmd_input);
        if (book) {

            auto& book_info = std::get<1>(*book);

            output_ << "Title: " << book_info.title << std::endl;
            output_ << "Author: " << std::get<0>(*book) << std::endl;
            output_ << "Publication year: " << book_info.publication_year << std::endl;

            auto tags = use_cases_.GetBookTags(book_info.id);
            if (!tags.empty()) {
                output_ << "Tags: ";
                PrintTags(output_, tags);
                output_ << std::endl;
            }
        }
        use_cases_.AcceptChanges();
    }
    catch (...) {
    }

    return true;
}

bool View::DeleteBook(std::istream& cmd_input) const
{
    try {
        use_cases_.SetNewUnitOfWork();

        if (auto book = SelectBook(cmd_input)) {
            use_cases_.DeleteBook(std::get<1>(*book).id);
        } 

        use_cases_.AcceptChanges();
    }
    catch (const std::exception&) {
        throw std::runtime_error("Book not found");
    }
    return true;
}

bool View::EditBook(std::istream& cmd_input) const
{
    try {
        use_cases_.SetNewUnitOfWork();

        if (auto book = SelectBook(cmd_input)) {
            auto title = EnterNewTitle(std::get<1>(*book).title);
            auto year = EnterNewPublYear(std::get<1>(*book).publication_year);

            output_ << "Enter tags (current tags: ";
            PrintTags(output_, use_cases_.GetBookTags(std::get<1>(*book).id));
            output_ << "):" << std::endl;
            auto tags = EnterTags();

            use_cases_.EditBook(std::get<1>(*book).id, title, year, tags);
        }

        use_cases_.AcceptChanges();
    }
    catch (const std::exception&) {
        throw std::runtime_error("Book not found");
    }

    return true;
}


std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);

    boost::algorithm::trim(params.title);

    auto author_id = SelectAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value();
        return params;
    }
}

std::optional<std::string> View::GetAuthorInfo(std::istream& cmd_input) const
{
    detail::AuthorInfo info;
    std::string name;
    std::getline(cmd_input, name);
    boost::algorithm::trim(name);

    if (name.empty()) {
        return SelectAuthorFromList();
    } 
    auto id = use_cases_.GetAuthorIdByName(name);
    if (!id) {
        throw std::runtime_error("No author with this name");
    }
    return id;
}


std::optional<std::string> View::SelectAuthorFromList() const {
    output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

std::optional<std::string> View::SelectAuthor() const {

    output_ << "Enter author name or empty line to select from list:" << std::endl;

    std::string str;
    std::getline(input_, str);
    boost::algorithm::trim(str);

    if (str.empty()) {
        return SelectAuthorFromList();
    }

    auto author_info = use_cases_.GetAuthorIdByName(str);
    if (!author_info) {
        output_ << "No author found. Do you want to add " << str << " (y/n)" << std::endl;
            
        std::string answer;
        std::getline(input_, answer);
            boost::algorithm::trim(answer);
            boost::algorithm::to_lower(answer);
        if (answer == "y") {
            use_cases_.AddAuthor(str);

            return *use_cases_.GetAuthorIdByName(str);
        } 
        else {
            throw std::runtime_error("Invalid answer");
        }
    }
    
    return author_info;

}

std::optional<std::tuple<std::string, detail::BookInfo>> View::SelectBookFromList(const std::vector<std::tuple<std::string, detail::BookInfo>>& books) const
{
    //output_ << "Select book:" << std::endl;
    PrintVector(output_, books);
    output_ << "Enter the book # or empty line to cancel:" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    }
    catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num");
    }

    auto& book = books[book_idx];
    return book;
}

std::optional<std::tuple<std::string, detail::BookInfo>> View::SelectBook(std::istream& cmd_input) const
{
    std::string str;
    std::getline(cmd_input, str);
    boost::algorithm::trim(str);

    if (str.empty()) {
        //¬вод книги из списка
        return SelectBookFromList(GetBooks());
    }
    else {
        auto titled_book = use_cases_.GetBooksByTitle(str);
        if (titled_book.size() > 1) {
            std::vector<std::tuple<std::string, detail::BookInfo>> dubl_books;

            for (auto& book : titled_book) {
                detail::BookInfo info{ .id = std::get<0>(book), .title = std::get<2>(book), .publication_year = std::get<3>(book) };
                dubl_books.push_back(std::make_tuple(*use_cases_.GetAuthorNameById(std::get<1>(book)), info));
            }
            return SelectBookFromList(dubl_books);

        }
        else if (titled_book.size() == 1) {
            detail::BookInfo info{ .id = std::get<0>(titled_book[0]), .title = std::get<2>(titled_book[0]), .publication_year = std::get<3>(titled_book[0])};
            return std::make_tuple(*use_cases_.GetAuthorNameById(std::get<1>(titled_book[0])), info);
        }
        else {
            throw std::runtime_error("Invalid book title");
        }
    }

    return std::nullopt;
}

std::optional<std::string> View::EnterNewTitle(const std::string& cur_title) const
{
    output_ << "Enter new title or empty line to use the current one (" << cur_title << "):" << std::endl;
    std::string new_title_;

    std::getline(input_, new_title_);
    boost::algorithm::trim(new_title_);

    if (new_title_.empty()) {
        return std::nullopt;
    }
    return new_title_;
}

std::optional<int> View::EnterNewPublYear(const int year) const
{
    output_ << "Enter publication year or empty line to use the current one (" << year << "):" << std::endl;
    std::string new_year_;

    std::getline(input_, new_year_);
    boost::algorithm::trim(new_year_);

    if (new_year_.empty()) {
        return std::nullopt;
    }

    return std::stoi(new_year_);
}

std::optional<std::set<std::string>> View::EnterTags() const
{
    std::string str;
    std::vector<std::string> tags;
    std::getline(input_, str);

    boost::split(tags, str, boost::is_any_of(","));
    
    std::set<std::string> result;
    for (auto& tag : tags) {
        boost::trim(tag);

        std::string::iterator new_end = std::unique(tag.begin(), tag.end(), [](char lhs, char rhs) {
            return (lhs == rhs) && (lhs == ' ');
            });

        tag.erase(new_end, tag.end());

        if (!tag.empty()) {
            result.insert(tag);
        }
    }

    if (!result.empty()) {
        return result;
    }

    return std::nullopt;
}



std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_autors;
    //assert(!"TODO: implement GetAuthors()");
    auto authors_info = use_cases_.GetAuthorsInfo();

    for (auto& pair : authors_info) {
        detail::AuthorInfo author{ .id = std::get<0>(pair), .name = std::get<1>(pair)};
        dst_autors.push_back(author);
    }
    return dst_autors;
}

std::vector<std::tuple<std::string, detail::BookInfo>> View::GetBooks() const {
    std::vector<std::tuple<std::string, detail::BookInfo>> books;
    //assert(!"TODO: implement GetBooks()");

    auto all_books_info = use_cases_.GetAllBooks();

    for(auto & info : all_books_info) {
        auto name = use_cases_.GetAuthorNameById(std::get<1>(info));
        detail::BookInfo book{ .id = std::get<0>(info), .title = std::get<2>(info), .publication_year = std::get<3>(info) };
        books.push_back(std::make_tuple(*name, book));
    }
    return books;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
    std::vector<detail::BookInfo> books;
    //assert(!"TODO: implement GetAuthorBooks()");

    auto books_by_author = use_cases_.GetBooksByAuthorInfo(author_id);

    for (auto& pair : books_by_author) {
        detail::BookInfo book{ .title = std::get<0>(pair), .publication_year = std::get<1>(pair) };
        books.push_back(book);
    }

    return books;
}

}  // namespace ui
