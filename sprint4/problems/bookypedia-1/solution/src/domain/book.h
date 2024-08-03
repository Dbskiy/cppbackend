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
		using BooksInfo = std::vector<std::tuple<std::string, int>>;
		virtual void Save(const Book& book) = 0;

		virtual BooksInfo GetBooksByAuthor(AuthorId& author_id) = 0;

		virtual BooksInfo GetAllBooks() = 0;
	protected:
		~BookRepository() = default;
	};

} //namespace domain