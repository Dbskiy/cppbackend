#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <optional>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"

namespace app {
	class UnitOfWork {
	public:
        using AuthorInfo = std::tuple<std::string, std::string>;
        using AuthorsInfo = std::vector<AuthorInfo>;
        using AuthorBooksInfo = std::vector<std::tuple<std::string, int>>;
        using AllBooksInfo = std::vector<std::tuple<std::string, std::string, int>>;

		virtual void Commit() = 0;

		virtual domain::AuthorRepository& Authors() = 0;

		virtual domain::BookRepository& Books() = 0;

        virtual void SaveAuthor(const domain::Author& author) = 0;
        
        virtual AuthorsInfo GetAuthorsInfo() = 0;

        virtual std::optional<std::string> GetAuthorIdByName(const std::string& name) = 0;

        virtual std::optional<std::string> GetAuthorNameById(const std::string& id) = 0;

        virtual void SaveBook(const domain::Book& book) = 0;        

        virtual AuthorBooksInfo GetBooksByAuthor(const std::string& author_id) = 0;

        virtual AllBooksInfo GetAllBooks() = 0;

		virtual ~UnitOfWork() = default;

	};
}