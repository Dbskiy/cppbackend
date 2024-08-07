#include "use_cases_impl.h"
#include "use_cases_impl.h"
#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    cur_unit_of_work_->SaveAuthor({ AuthorId::New(), name });
}

UseCasesImpl::AuthorsInfo UseCasesImpl::GetAuthorsInfo()
{
    return cur_unit_of_work_->GetAuthorsInfo();
}

std::optional<std::string> UseCasesImpl::GetAuthorIdByName(const std::string& name)
{
    return cur_unit_of_work_->GetAuthorIdByName(name);
}

std::optional<std::string> UseCasesImpl::GetAuthorNameById(const std::string& id)
{
    return cur_unit_of_work_->GetAuthorNameById(id);
}

void UseCasesImpl::AddBook(const std::string& author_id, const std::string& title, const int publication_year)
{
    auto author_id_ = AuthorId::FromString(author_id);
    cur_unit_of_work_->SaveBook({ BookId::New(), author_id_, title, publication_year });
}

UseCasesImpl::AuthorBooksInfo UseCasesImpl::GetBooksByAuthorInfo(const std::string& author_id)
{
    return cur_unit_of_work_->GetBooksByAuthor(author_id);
}

UseCasesImpl::AllBooksInfo UseCasesImpl::GetAllBooks()
{
    return cur_unit_of_work_->GetAllBooks();
}

}  // namespace app
