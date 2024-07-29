#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/beast/http.hpp>

#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <optional>
#include <unordered_set>
#include "model.h"
#include "loot_generator.h"
#include "providers.h"


namespace beast = boost::beast;
namespace http = beast::http;

using namespace model;
using namespace providers;

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

    };

    class Players {
    public:
        using PairPlayerAndToken = std::tuple<std::shared_ptr<Player>, Token>;
        using PlayersInfo = std::unordered_map<Token, std::shared_ptr<Player>, TokenHasher>;

        void SetPlayersInfo(PlayersInfo& players_info) {
            players_info_ = players_info;
        }

        const PlayersInfo& GetPlayersInfo() const {
            return players_info_;
        }

        PairPlayerAndToken AddPlayer(std::shared_ptr<Dog> dog, std::shared_ptr<GameSession> session);
        
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
        
        PlayersInfo players_info_;
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

        JoinGameResult Join(const std::string& map_id, const std::string& name, const bool randomize_spawn);


    private:
        Game& game_;
        Players& players_;
    };



    class ListPlayersResult 
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

    class GameStateResult 
    {
    public:
        GameStateResult(std::unordered_map<std::uint64_t, Dog::DogState>& players_state, std::unordered_map<int, Loot>& loot_state) 
            : players_state_(players_state),
              loot_state_(loot_state)
        {}

        std::unordered_map<std::uint64_t, Dog::DogState> GetPlayersState() {
            return players_state_;
        }

        std::unordered_map<int, Loot> GetLootState() {
            return loot_state_;
        }

    private:
        std::unordered_map<std::uint64_t, Dog::DogState> players_state_;
        std::unordered_map<int, Loot> loot_state_;
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

    class GatheringAndDelivering {
    public:
        GatheringAndDelivering(std::shared_ptr<GameSession> session, int tick) : 
            session_(session), 
            dogs_(session->GetDogs()), 
            offices_(session->GetMap()->GetOffices()),
            all_loot_(session->GetAllLoot()),
            bag_capacity_(session->GetMap()->GetBagCapacity()),
            max_distance_(*session->GetMap()->GetDogSpeed() * static_cast<double>(tick) / ms_to_s) {};

        void GatherAndDeliver();

    private:

        GathererProvider GetGathererProvider();
        DelivererProvider GetDelivererProvider();

        void SetCorrectChronology(std::vector<GatheringEvent>& g_events, std::vector<DeliveringEvent>& d_events,
            const GameSession::Dogs& dogs) const;

        void PutInTheBag(const size_t g_id, const size_t loot_id);
        void BringLootToTheBase(const size_t d_id);

        std::shared_ptr<GameSession> session_;
        GameSession::Dogs& dogs_;
        const Map::Offices& offices_;
        GameSession::AllLoot& all_loot_;

        const double ms_to_s = 1000.0;
        Map::BagCapacity bag_capacity_;
        const double max_distance_;

    };

    class GameTime {
    public:
        GameTime(Game& game, loot_gen::LootGenerator& loot_gen) : game_(game), loot_gen_(loot_gen) {}

        void MoveAllPlayers(int tick);

        void GenerateLoot(int tick);

        void GatheringAndDeliviring(int tick);

    private:
        Game& game_;
        loot_gen::LootGenerator loot_gen_;
        void ChangeCoordinates(const Map* map, Dog::DogState& state, int tick) const;

        void PointXOutOfRoad(Dog::DogState& state, int start_x, int end_x) const;
        void PointYOutOfRoad(Dog::DogState& state, int start_y, int end_y) const;
    };

    class ApplicationListener {
    public:
        virtual ~ApplicationListener() = default;
        virtual void OnTick(int tick) = 0;
    };

    class Application {
    public:
        Application(Game& game, Players& players, loot_gen::LootGenerator& loot_gen)
            : game_(game),
            players_(players),
            loot_gen_(loot_gen) {}
        
        Game& game_;
        Players& players_;

        void SetApplicationListener(std::shared_ptr<ApplicationListener> listener) {
            listener_ = listener;
        }

        loot_gen::LootGenerator loot_gen_;

        JoinGameResult JoinGame(const std::string& map_id, const std::string& name, const bool randomize_spawn);

        std::optional<ListPlayersResult> GetPlayersList(const Token& token);

        std::optional<GameStateResult> GetGameState(const Token& token);

        std::optional<std::string_view> MovePlayer(const Token& token, const std::string& move);

        void Tick(int tick);

    private:

        std::shared_ptr<ApplicationListener> listener_;

        JoinGameUseCase join_game_{ game_, players_ };
        ListPlayersUseCase list_players_{ game_, players_ };
        GameStateUseCase game_state_{ game_, players_ };
        PlayerActionUseCase player_action{ players_ };
        GameTime game_time_{ game_, loot_gen_ };
    };
}
