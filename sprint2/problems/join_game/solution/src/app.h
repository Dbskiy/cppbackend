#pragma once
#include <iomanip>
#include <random>
#include <string>
#include <sstream>
#include "model.h"

using namespace model;
namespace detail {
    struct TokenTag {};  
}

namespace app {
    
    using Token = util::Tagged<std::string, detail::TokenTag>;
    using TokenHasher = util::TaggedHasher<Token>;

    class Player {
    public:

        Player(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session)
            : dog_(dog),
            session_(session) {}
        
        std::shared_ptr<GameSession> session_;
        std::shared_ptr<Dog> dog_;


    /*private:*/

    };


    //вариант с одним классом 
    class Players {
    public:
        using PairPlayerAndToken = std::tuple<std::shared_ptr<Player>, Token>;

        Players::PairPlayerAndToken AddPlayer(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session);
        std::shared_ptr<Player> FindByDogIdAndMapId(const Dog::DogId& dog_id, const Map::Id& map_id); 
        
        std::shared_ptr<Player> FindByToken(const Token& token);

        
    private:
        std::random_device random_device_;

        std::mt19937_64 generator1_{ [this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
        }() };

        std::mt19937_64 generator2_{ [this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(random_device_);
        }() };

        Token generateToken() {
            auto number1 = generator1_();
            auto number2 = generator2_();

            std::stringstream ss;
            ss << std::hex << std::setw(16) << std::setfill('0') << number1;
            ss << std::hex << std::setw(16) << std::setfill('0') << number2;

            return Token(ss.str());
        }

        std::unordered_map<Token, std::shared_ptr<Player>, TokenHasher> players_;
    };

}
