#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <set>
#include <optional>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "../domain/book_tags_fwd.h"

namespace app {
	class UnitOfWork {
	public:
        using AuthorInfo = std::tuple<std::string, std::string>;
        using AuthorsInfo = std::vector<AuthorInfo>;
        using AuthorBooksInfo = std::vector<std::tuple<std::string, int>>;
        using AllBooksInfo = std::vector<std::tuple<std::string, std::string, std::string, int>>;


		virtual void Commit() = 0;

		virtual domain::AuthorRepository& Authors() = 0;

		virtual domain::BookRepository& Books() = 0;

        virtual void SaveAuthor(const domain::Author& author) = 0;
        
        virtual AuthorsInfo GetAuthorsInfo() = 0;

        virtual std::optional<std::string> GetAuthorIdByName(const std::string& name) = 0;

        virtual std::optional<std::string> GetAuthorNameById(const std::string& id) = 0;

        virtual void SaveBook(const domain::Book& book) = 0;

        virtual AllBooksInfo GetBooksByTitle(const std::string& title) = 0;

        virtual AuthorBooksInfo GetBooksByAuthor(const std::string& author_id) = 0;

        virtual AllBooksInfo GetAllBooks() = 0;

        virtual void SaveBookTags(std::vector<domain::BookTag>& tags) = 0;

        virtual std::vector<std::string>GetBookTags(const std::string& book_id) = 0;

        virtual void DeleteBook(const std::string& book_id) = 0;

        virtual void DeleteAuthor(const std::string& author_id) = 0;

        virtual void EditAuthor(const std::string& author_id, const std::string& new_name) = 0;

        virtual void EditBook(const std::string& book_id, std::optional<std::string>& title, std::optional<int>& year, std::optional<std::set<std::string>>& tags) = 0;

		virtual ~UnitOfWork() = default;

	};
}