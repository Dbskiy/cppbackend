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
            //assert(!"TODO: implement book adding");

            use_cases_.AddBook(params->author_id, params->title, params->publication_year);
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
        output_ << "No author found. Do you want to add " << str << " (y / n) ?" << std::endl;
            
        std::string answer;
        std::getline(input_, answer);
            boost::algorithm::trim(answer);
            boost::algorithm::to_lower(answer);
        if (answer == "y") {
            use_cases_.AddAuthor(str);

            return *use_cases_.GetAuthorIdByName(str);
        } 
        return std::nullopt;
    }
    else {
        return *author_info;
    }

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
        auto name = use_cases_.GetAuthorNameById(std::get<0>(info));
        detail::BookInfo book{ .title = std::get<1>(info), .publication_year = std::get<2>(info) };
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
