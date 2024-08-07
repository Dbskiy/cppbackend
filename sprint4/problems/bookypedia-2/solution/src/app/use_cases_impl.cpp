#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"
#include "../domain/book_tags.h"

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

void UseCasesImpl::AddBook(BookTags& tags, const std::string& author_id, const std::string& title, const int publication_year)
{
    auto author_id_ = AuthorId::FromString(author_id);
    auto book_id = BookId::New();

    std::string str_book_id = book_id.ToString();

    cur_unit_of_work_->SaveBook({ book_id, author_id_, title, publication_year });

    AddBookTags(str_book_id, tags);

}

void UseCasesImpl::DeleteBook(const std::string& book_id)
{
    cur_unit_of_work_->DeleteBook(book_id);
}

void UseCasesImpl::AddBookTags(const std::string& book_id, BookTags& tags)
{
    if (tags.has_value()) {
        std::vector<BookTag> book_tags;
        for (auto& tag : *tags)
        {
            book_tags.push_back({ BookId::FromString(book_id), tag });
        }
        cur_unit_of_work_->SaveBookTags(book_tags);
    }

}

std::vector<std::string> UseCasesImpl::GetBookTags(const std::string& book_id)
{
    return cur_unit_of_work_->GetBookTags(book_id);
}

UseCasesImpl::AllBooksInfo UseCasesImpl::GetBooksByTitle(const std::string& title)
{
    return cur_unit_of_work_->GetBooksByTitle(title);
}

UseCasesImpl::AuthorBooksInfo UseCasesImpl::GetBooksByAuthorInfo(const std::string& author_id)
{
    return cur_unit_of_work_->GetBooksByAuthor(author_id);
}

UseCasesImpl::AllBooksInfo UseCasesImpl::GetAllBooks()
{
    return cur_unit_of_work_->GetAllBooks();
}

void UseCasesImpl::DeleteAuthor(const std::string& author_id)
{
    cur_unit_of_work_->DeleteAuthor(author_id);
}

void UseCasesImpl::EditAuthor(const std::string& author_id, const std::string& new_name)
{
    cur_unit_of_work_->EditAuthor(author_id, new_name);
}

void UseCasesImpl::EditBook(const std::string& book_id, std::optional<std::string>& title, std::optional<int>& year, std::optional<std::unordered_set<std::string>>& tags)
{
    cur_unit_of_work_->EditBook(book_id, title, year, tags);
}

}  // namespace app
