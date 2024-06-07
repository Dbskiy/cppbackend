#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/beast/http.hpp>

#include <iomanip>
#include <random>
#include <string>
#include <sstream>
#include <optional>
#include <unordered_set>
#include "model.h"


namespace beast = boost::beast;
namespace http = beast::http;

using namespace model;
namespace detail {
    struct TokenTag {};  
}

namespace app {
    
    using namespace std::literals;

    using Token = util::Tagged<std::string, detail::TokenTag>;
    using TokenHasher = util::TaggedHasher<Token>;

    enum class AuthorizationErrorReason {
        InvalidHeader,
        TokenNotFound
    };

    class AuthorizationError {
    public:
        explicit AuthorizationError(AuthorizationErrorReason reason) : reason_(reason) {}

        AuthorizationErrorReason GetReason() const {
            return reason_;
        }

    private:
        AuthorizationErrorReason reason_;
    };


    class Player {
    public:

        Player(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session)
            : dog_(dog),
            session_(session) {};
        
        Dog::DogId GetId() const {
            return dog_->GetDogId();
        }

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

    enum class JoinGameErrorReason {
        InvalidName,
        InvalidMap
    };

    class JoinGameError {
    public:
        explicit JoinGameError(JoinGameErrorReason reason) : reason_(reason) {}

        JoinGameErrorReason GetReason() const {
            return reason_;
        }

    private:
        JoinGameErrorReason reason_;
    };

    class JoinGameResult //класс для доступа к необходимым полям корректного ответа сервера
    {
    public:
        JoinGameResult(Token token, Dog::DogId dog_id)
            : token_(token),
            dog_id_(dog_id) {};


        Token token_;
        Dog::DogId dog_id_;
        
    };

    class JoinGameUseCase
    {
    public:
        JoinGameUseCase(Game& game, Players& players) 
        : game_(game),
          players_(players) {};

        JoinGameResult Join(const std::string& map_id, const std::string& name);


    private:
        Game& game_;
        Players& players_;
    };



    class ListPlayersResult //класс для доступа к необходимым полям корректного ответа сервера
    {
    public:
        ListPlayersResult(const std::unordered_map<std::uint64_t, std::string>& list_players) 
            :  list_players_(list_players) {};

        std::unordered_map<std::uint64_t, std::string> list_players_;
    };

    class ListPlayersUseCase {
    public:
        ListPlayersUseCase(Game& game, Players& players) 
            : game_(game),
            players_(players) {}

        std::optional<ListPlayersResult> GetPlayersList(const Token& token);

    private:
        Game& game_;
        Players& players_;
    };

    class GameStateResult //класс для доступа к необходимым полям корректного ответа сервера
    {
    public:
        GameStateResult(std::unordered_map<std::uint64_t, Dog::DogState>& players_state) : players_state_(players_state) {}

        std::unordered_map<std::uint64_t, Dog::DogState> GetPlayersState() {
            return players_state_;
        }

    private:
        std::unordered_map<std::uint64_t, Dog::DogState> players_state_;
    };

    
    class GameStateUseCase {
    public:
        GameStateUseCase(Game& game, Players& players) 
        : game_(game),
            players_(players) {}


        std::optional<GameStateResult> GameState(const Token& token);
    private:
        Game& game_;
        Players& players_;
    };

    class PlayerMovement {
    public:
        PlayerMovement(const Map::DogSpeed& speed)
            : change_of_dir{
                {"L", {0.0 - *speed, 0.0}},
                {"R", {*speed, 0.0}},
                {"U", {0.0, 0.0 - *speed}},
                {"D", {0.0, *speed}},
                {"", {0.0, 0.0}}
            } {}

        const std::unordered_map<std::string, std::tuple<double, double>> change_of_dir;

    };

    class PlayerActionUseCase {
    public:
        PlayerActionUseCase(Players& players)
        : players_(players) {}

        std::optional<std::string_view> MovePlayer(const Token&, const std::string&);

    private:
        Players& players_;
    };

    class Application {
    public:
        Application(Game& game, Players& players)
        : game_(game),
          players_(players) {};

        JoinGameResult JoinGame(const std::string& map_id, const std::string& name);

        std::optional<ListPlayersResult> GetPlayersList(const Token& token);

        std::optional<GameStateResult> GetGameState(const Token& token);

        std::optional<std::string_view> MovePlayer(const Token& token, const std::string& move);

    private:
        Game& game_;
        Players& players_;

        JoinGameUseCase join_game_{ game_, players_ };
        ListPlayersUseCase list_players_{ game_, players_ };
        GameStateUseCase game_state_{ game_, players_ };
        PlayerActionUseCase player_action{ players_ };
    };
}
