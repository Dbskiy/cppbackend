#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;
    AuthorsInfo authors_info_;

    void Save(const domain::Author& author) override {
        authors_info_.push_back({ author.GetName(), author.GetId().ToString() });
        saved_authors.emplace_back(author);

    }

    AuthorsInfo GetAuthorsInfo() override {
        return authors_info_;
    }
};

struct MockBookRepository : domain::BookRepository {

    std::vector<domain::Book> saved_books;
    BooksInfo books_info_;

    void Save(const domain::Book& book) {
        books_info_.push_back({book.GetTitle(), book.GetPublicationYear()});
        saved_books.emplace_back(book);
    }

    BooksInfo GetBooksByAuthor(domain::AuthorId& author_id) {
        return BooksInfo{};
    }

    BooksInfo GetAllBooks() {
        return BooksInfo{};
    }
};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases{authors, books};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(authors.saved_authors.at(0).GetId() != domain::AuthorId{});
            }
        }
    }
}