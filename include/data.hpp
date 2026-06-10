#include <iomanip>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "json.hpp" // Do no touch this and the file itself

using namespace std;
using json = nlohmann::json;

class Card {
    public:
        string front, back;
        int type = 0; // 0: Regular, 1: Reversible, 2: Input Answer
        vector<string> tags;  //for tags
        bool flagged = false;

        // FSRS DIFFICULTY! [mj]
        double difficulty = 5.0;
        double stability = 1.0;
        int repetitions = 0;
        int nextReview = 0;

        Card() = default;
        // Updated constructor to accept the type layout parameter
        Card(string front, string back, int type = 0):
            front(front), back(back), type(type) {}
};

class Deck {
    public:
        string deckName;
        int level;
        vector<Card> cards;
        vector<Deck> subDecks;
        vector<Card> cardsIncSub;

        Deck() = default;
        Deck(string deckName):
            deckName(deckName) {
                level = 0;
            }
        Deck(string deckName, int level):
            deckName(deckName), level(level) {}
};

class Profile {
    public:
        string profileName;
        vector<Deck> decks;

        // Statistics Fields [mj]
        int totalStudied = 0;
        int totalCorrect = 0;
        int totalWrong = 0;
        int streak = 0;

        vector<string> tags;

        Profile() = default;
        Profile(string profileName):
            profileName(profileName) {
                decks.push_back(Deck("Default Deck"));
            }
};

class Account {
    public:
        string accountName, password;
        vector<Profile> profiles;

        Account() = default;
        Account(string accountName, string password):
            accountName(accountName), password(password) {
                profiles.push_back(Profile("Default Profile"));
            }
};

/*
    The code below is responsible for the conversion of C++ Class to JSON Object and vice versa
    Each class has their own corresponding adl_serializer struct containing a to_json and a from_json functions
    Most important thing to understand here is understanding the attributes of the classes and their data types
*/
namespace nlohmann {
    template<>
    struct adl_serializer<Card> {
        static void to_json(json &j, const Card &c) {
            j = json {
                {"front", c.front},
                {"back", c.back},
                {"type", c.type},
                {"difficulty", c.difficulty},
                {"stability", c.stability},
                {"repetitions", c.repetitions},
                {"nextReview", c.nextReview},
                {"tags", c.tags},
                {"flagged", c.flagged}
            };
        } 

        static void from_json(const json &j, Card &c) {
            c.front = j.at("front").get<string>();
            c.back = j.at("back").get<string>();
            c.type = j.at("type").get<int>();
            c.difficulty = j.at("difficulty").get<double>();
            c.stability = j.at("stability").get<double>();
            c.repetitions = j.at("repetitions").get<int>();
            c.nextReview = j.at("nextReview").get<int>();
            c.tags = j.value("tags", vector<string>{});
            c.flagged = j.value("flagged", false);
        }
    };

    template<>
    struct adl_serializer<Deck> {
        static void to_json(json &j, const Deck &c) {
            j = json {
                {"deckName", c.deckName},
                {"level", c.level},
                {"cards", c.cards},
                {"subDecks", c.subDecks},
                {"cardsIncSub", c.cardsIncSub}
            };
        } 

        static void from_json(const json &j, Deck &c) {
            c.deckName = j.at("deckName").get<string>();
            c.level = j.at("level").get<int>();
            c.cards = j.at("cards").get<vector<Card>>();
            c.subDecks = j.at("subDecks").get<vector<Deck>>();
            c.cardsIncSub = j.at("cardsIncSub").get<vector<Card>>();
        }
    };

    template<>
    struct adl_serializer<Profile> {
        static void to_json(json &j, const Profile &c) {
            j = json {
                {"profileName", c.profileName},
                {"decks", c.decks},
                {"totalStudied", c.totalStudied},
                {"totalCorrect", c.totalCorrect},
                {"totalWrong", c.totalWrong},
                {"streak", c.streak},
                {"tags", c.tags}
            };
        } 

        static void from_json(const json &j, Profile &c) {
            c.profileName = j.at("profileName").get<string>();
            c.decks = j.at("decks").get<vector<Deck>>();
            c.totalStudied = j.at("totalStudied").get<int>();
            c.totalCorrect = j.at("totalCorrect").get<int>();
            c.totalWrong = j.at("totalWrong").get<int>();
            c.streak = j.at("streak").get<int>();
            c.tags = j.at("tags").get<vector<string>>();
        }
    };

    template<>
    struct adl_serializer<Account> {
        static void to_json(json &j, const Account &c) {
            j = json {
                {"accountName", c.accountName},
                {"password", c.password},
                {"profiles", c.profiles}
            };
        }

        static void from_json(const json &j, Account &c) {
            c.accountName = j.at("accountName").get<string>();
            c.password = j.at("password").get<string>();
            c.profiles = j.at("profiles").get<vector<Profile>>();
        }
    };
}