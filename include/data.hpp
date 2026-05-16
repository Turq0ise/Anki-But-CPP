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

        Card() = default;
        Card(string front, string back):
            front(front), back(back) {}
};

// Doesn't work yet, just a placeholder
class ReversibleCard: public Card {
    public:
    string test;
};

// Doesn't work yet, just a placeholder
class InputCard: public Card {
    public:
        string test;
};

class Deck {
    public:
        string deckName;
        int level;
        vector<Card> cards, cardsIncSub;

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
                {"back", c.back}
            };
        } 

        static void from_json(const json &j, Card &c) {
            c.front = j.at("front").get<string>();
            c.back = j.at("back").get<string>();
        }
    };

    template<>
    struct adl_serializer<Deck> {
        static void to_json(json &j, const Deck &c) {
            j = json {
                {"deckName", c.deckName},
                {"level", c.level},
                {"cards", c.cards},
                {"cardsIncSub", c.cardsIncSub}
            };
        } 

        static void from_json(const json &j, Deck &c) {
            c.deckName = j.at("deckName").get<string>();
            c.level = j.at("level").get<int>();
            c.cards = j.at("cards").get<vector<Card>>();
            c.cardsIncSub = j.at("cardsIncSub").get<vector<Card>>();
        }
    };

    template<>
    struct adl_serializer<Profile> {
        static void to_json(json &j, const Profile &c) {
            j = json {
                {"profileName", c.profileName},
                {"decks", c.decks}
            };
        } 

        static void from_json(const json &j, Profile &c) {
            c.profileName = j.at("profileName").get<string>();
            c.decks = j.at("decks").get<vector<Deck>>();
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