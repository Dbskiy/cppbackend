#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

UseCasesImpl::AuthorsInfo UseCasesImpl::GetAuthorsInfo()
{
    return authors_.GetAuthorsInfo();
}

void UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, const int publication_year)
{
    auto author_id_ = AuthorId::FromString(author_id);
    books_.Save({ BookId::New(), author_id_, title, publication_year });
}

UseCasesImpl::BooksInfo UseCasesImpl::GetBooksByAuthorInfo(const std::string& author_id)
{
    auto id = AuthorId::FromString(author_id);
    return books_.GetBooksByAuthor(id);
}

UseCasesImpl::BooksInfo UseCasesImpl::GetAllBooks()
{
    return books_.GetAllBooks();
}

}  // namespace app
