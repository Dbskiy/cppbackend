#pragma once
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

	namespace detail {
		struct BookTag {};
	}  // namespace detail

	using BookId = util::TaggedUUID<detail::BookTag>;

	class Book {
	public:
		Book(BookId id, domain::AuthorId& author_id, std::string title, int publication_year)
			: id_(std::move(id))
			, author_id_(author_id)
			, title_(std::move(title))
			, publication_year_(publication_year) {}


		const BookId& GetId() const noexcept {
			return id_;
		}

		const domain::AuthorId GetAuthorId() const noexcept {
			return author_id_;
		}

		const std::string& GetTitle() const noexcept {
			return title_;
		}

		const int& GetPublicationYear() const noexcept {
			return publication_year_;
		}

	private:
		BookId id_;
		domain::AuthorId author_id_;
		std::string title_;
		int publication_year_;
	};

	class BookRepository {
	public:
		using AuthorBooksInfo = std::vector<std::tuple<std::string, int>>;
		using AllBooksInfo = std::vector<std::tuple<std::string, std::string, std::string, int>>;
		virtual void Save(const Book& book, pqxx::work& w_) = 0;

		virtual void EditBookTitle(const std::string& book_id, const std::string& title, pqxx::work& w_) = 0;
		virtual void EditBookPublYear(const std::string& book_id, const int year, pqxx::work& w_) = 0;

		virtual AllBooksInfo GetBooksByTitle(const std::string& title, pqxx::work& w_) = 0;

		virtual AuthorBooksInfo GetBooksByAuthor(const std::string& author_id, pqxx::work& w_) = 0;

		virtual std::string GetBookIdByAuthorIdAndTitle(const std::string& author_id, const std::string& title, pqxx::work& w_) = 0; 

		virtual AllBooksInfo GetAllBooks(pqxx::work& w_) = 0;

		virtual void DeleteBook(const std::string& book_id, pqxx::work& w_) = 0;
	protected:
		~BookRepository() = default;
	};

} //namespace domain