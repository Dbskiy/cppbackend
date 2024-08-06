#pragma once
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"

namespace app {
	class UnitOfWork {
	public:
		virtual void Commit() = 0;

		virtual bool isTransactionClosed() = 0;

		virtual domain::AuthorRepository& Authors() = 0;
		virtual domain::BookRepository& Books() = 0;

		virtual ~UnitOfWork() = default;

	};
}