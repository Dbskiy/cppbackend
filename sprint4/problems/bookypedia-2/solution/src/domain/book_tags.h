#pragma once
#include <pqxx/pqxx>
#include <string>
#include <unordered_set>

#include "book.h"
#include "../util/tagged_uuid.h"

namespace domain {

    class BookTag {
    public:
        BookTag(const BookId& id, std::string tag)
            : id_(id)
            , tag_(std::move(tag)) {
        }

        const BookId& GetBookId() const noexcept {
            return id_;
        }

        const std::string& GetTag() const noexcept {
            return tag_;
        }

    private:
        BookId id_;
        std::string tag_;
    };

    class BookTagRepository {
    public:
        virtual void Save(const std::vector<BookTag>& book_tags, pqxx::work& w_) = 0;

        virtual std::vector<std::string> GetBookTags(const std::string& book_id, pqxx::work& w_) = 0;
        virtual void EditTags(const std::string& book_id, const std::unordered_set<std::string>& tags, pqxx::work& w_) = 0;

        virtual void DeleteBookTags(const std::string book_id, pqxx::work& w_) = 0;

    protected:
        ~BookTagRepository() = default;
    };

}  // namespace domain