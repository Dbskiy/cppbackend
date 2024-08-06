#pragma once
#include <memory>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "../app/unit_of_work_factory.h"
#include "use_cases.h"



namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(app::UnitOfWorkFactory& unit_of_work_factory) : factory_(unit_of_work_factory) {}

    void SetNewUnitOfWork() override {
        if (cur_unit_of_work_->isTransactionClosed()) {
            cur_unit_of_work_ = factory_.CreateUnitOfWork();
        }
    }
    
    void AcceptChanges() override {
        if (cur_unit_of_work_ && !cur_unit_of_work_->isTransactionClosed()) {
            cur_unit_of_work_->Commit();
        }
    }

    void AddAuthor(const std::string& name) override;

    AuthorsInfo GetAuthorsInfo() override;

    std::optional<std::string> GetAuthorIdByName(const std::string& name) override;
    std::optional<std::string> GetAuthorNameById(const std::string& id) override;

    void AddBook(const std::string& author_id, const std::string& title, const int publication_year) override;

    AuthorBooksInfo GetBooksByAuthorInfo(const std::string& author_id) override;

    AllBooksInfo GetAllBooks() override;

private:
    app::UnitOfWorkFactory& factory_;
    std::unique_ptr<UnitOfWork> cur_unit_of_work_ = factory_.CreateUnitOfWork();
};

}  // namespace app
