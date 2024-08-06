#include "use_cases_impl.h"
#include "use_cases_impl.h"
#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    cur_unit_of_work_->Authors().Save({AuthorId::New(), name});
}

UseCasesImpl::AuthorsInfo UseCasesImpl::GetAuthorsInfo()
{
    return cur_unit_of_work_->Authors().GetAuthorsInfo();
}

std::optional<std::string> UseCasesImpl::GetAuthorIdByName(const std::string& name)
{
    return cur_unit_of_work_->Authors().GetAuthorIdByName(name);
}

std::optional<std::string> UseCasesImpl::GetAuthorNameById(const std::string& id)
{
    return cur_unit_of_work_->Authors().GetAuthorNameById(id);
}

void UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, const int publication_year)
{
    auto author_id_ = AuthorId::FromString(author_id);
    cur_unit_of_work_->Books().Save({ BookId::New(), author_id_, title, publication_year });
}

UseCasesImpl::AuthorBooksInfo UseCasesImpl::GetBooksByAuthorInfo(const std::string& author_id)
{
    auto id = AuthorId::FromString(author_id);
    return cur_unit_of_work_->Books().GetBooksByAuthor(id);
}

UseCasesImpl::AllBooksInfo UseCasesImpl::GetAllBooks()
{
    return cur_unit_of_work_->Books().GetAllBooks();
}

}  // namespace app
