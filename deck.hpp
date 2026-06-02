#pragma once

/*
 * deck.hpp
 * --------
 * Defines the Deck data structure and its full CRUD engine
 * (Create, Read, Update, Delete) for use in the Anki C++ project.
 *
 * Designed to integrate cleanly with:
 *   - data.hpp  (Card, Profile, Account classes)
 *   - main.cpp  (state machine / account management)
 *   - data.json (persistent storage, shared JSON format)
 *
 * Deck structure (matches your spec exactly):
 *   deckName    | string
 *   level       | int          (0 = top-level, 1 = sub-deck)
 *   cards       | vector<Card> (cards belonging directly to this deck)
 *   subDecks    | vector<Deck> (child decks nested under this deck)
 *   cardsIncSub | vector<Card> (cards from this deck + all sub-decks combined)
 *
 * JSON format mirrors Python's deckDB.json:
 *   Top-level decks keyed by timestamp ID string.
 *   Sub-decks nested under parent's "subdecks" object.
 *   Every deck stores: deck_name, description, creation_date,
 *                      subdecks (or cards for sub-decks).
 *
 * All CRUD operations save to data.json immediately after any change.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  Forward declaration — Card is defined in data.hpp.
//  Include data.hpp before deck.hpp in your translation units.
//  Declared here so deck.hpp can reference Card independently
//  if needed during integration testing.
// ─────────────────────────────────────────────────────────────
struct Card {
    string front;
    string back;

    Card() = default;
    Card(string f, string b) : front(f), back(b) {}
};

// ─────────────────────────────────────────────────────────────
//  SetMode
//  Mirrors Python's SetMode enum.
//  Controls whether the deck form is in ADD or EDIT mode.
// ─────────────────────────────────────────────────────────────
enum class SetMode {
    ADD,   // Creating a brand-new deck
    EDIT   // Modifying an existing deck's name or description
};

// ─────────────────────────────────────────────────────────────
//  CardInfo
//  Mirrors Python's CardInfo enum.
//  Used as readable index labels for the deck_info tuple.
// ─────────────────────────────────────────────────────────────
enum class CardInfo {
    DECK    = 0,  // Index of the parent deck name
    SUBDECK = 1,  // Index of the sub-deck name (empty string if none)
    DESC    = 2   // Index of the description
};

// ─────────────────────────────────────────────────────────────
//  Deck
//  Core data structure. Matches your specification:
//    deckName    | string
//    level       | int
//    cards       | vector<Card>
//    subDecks    | vector<Deck>
//    cardsIncSub | vector<Card>
//
//  Additional fields (matching Python's JSON schema):
//    deckId      | string  — timestamp-based unique key (e.g. "1718000000")
//    description | string  — optional user-provided description
//    creationDate| string  — human-readable creation timestamp
// ─────────────────────────────────────────────────────────────
struct Deck {
    string         deckId;       // Unique key derived from creation timestamp
    string         deckName;     // Display name of the deck
    string         description;  // Optional description of the deck's content
    string         creationDate; // Human-readable creation date/time string
    int            level;        // Nesting depth: 0 = top-level, 1 = sub-deck
    vector<Card>   cards;        // Cards belonging directly to this deck only
    vector<Deck>   subDecks;     // Child decks nested under this deck
    vector<Card>   cardsIncSub;  // Cards from this deck AND all sub-decks combined

    Deck() : level(0) {}

    // Constructs a deck with a name, description, and nesting level.
    // deckId and creationDate are assigned separately (see DeckManager::createDeck).
    Deck(string id, string name, string desc, int lvl)
        : deckId(id), deckName(name), description(desc), level(lvl) {}
};

// ─────────────────────────────────────────────────────────────
//  DeckManager
//  The CRUD engine for decks. Owns the in-memory deck store
//  (userData) and syncs every change to data.json immediately.
//
//  Mirrors all Python functions:
//    createDeck()   ← push_entry(SetMode.ADD)
//    updateDeck()   ← push_entry(SetMode.EDIT)
//    deleteDeck()   ← delete_deck()
//    listDecks()    ← updateData() display logic
//    saveData()     ← saveData()
//    loadData()     ← readData()
//    fillDeckParent()  ← fill_deck_parent()
//
//  Integration note:
//    DeckManager works on the SAME data.json file as the rest of
//    the app. Pass the activeProfile's deck list by reference when
//    integrating with Profile / Account logic from main.cpp.
// ─────────────────────────────────────────────────────────────
class DeckManager {
public:

    // The full deck store: maps deck ID string → JSON object.
    // Mirrors Python's user_data dict.
    json userData;

    // Maps deck display name → deck ID.
    // Used to resolve parent deck selection. Mirrors deck_name2id.
    unordered_map<string, string> deckName2Id;

    // Currently selected deck: {deckId, subDeckId}.
    // subDeckId is "" if a top-level deck is selected.
    pair<string, string> currentViewDeck;

    // Info about the currently selected deck: {deckName, subDeckName, description}.
    // Mirrors Python's deck_info tuple.
    array<string, 3> deckInfo;

    // Path to the JSON file used for persistent storage.
    string dataFilePath = "data.json";

    // ── Constructor ──────────────────────────────────────────
    DeckManager() {
        currentViewDeck = {"", ""};
        deckInfo        = {"", "", ""};
    }

    // ─────────────────────────────────────────────────────────
    //  saveData
    //  Writes the current userData to data.json immediately.
    //  Called after every Create, Update, or Delete operation
    //  so no changes are ever lost. Mirrors Python's saveData().
    // ─────────────────────────────────────────────────────────
    void saveData() {
        ofstream file(dataFilePath);
        if (!file.is_open()) {
            cerr << "[DeckManager] ERROR: Could not open " << dataFilePath << " for writing.\n";
            return;
        }
        file << userData.dump(4); // pretty-print with 4-space indent
        file.close();
        cout << "[DeckManager] Data saved to " << dataFilePath << "\n";
    }

    // ─────────────────────────────────────────────────────────
    //  loadData
    //  Reads data.json into userData.
    //  Silently does nothing if the file doesn't exist yet.
    //  Mirrors Python's readData().
    // ─────────────────────────────────────────────────────────
    void loadData() {
        ifstream file(dataFilePath);
        if (!file.is_open()) return; // first run — no file yet, start empty

        try {
            file >> userData;
        } catch (const json::parse_error &e) {
            cerr << "[DeckManager] ERROR: Failed to parse " << dataFilePath
                 << ": " << e.what() << "\n";
            userData = json::object(); // reset to empty on corrupt file
        }
        file.close();

        fillDeckParent(); // rebuild name→id map after loading
    }

    // ─────────────────────────────────────────────────────────
    //  fillDeckParent
    //  Rebuilds deckName2Id from the current userData.
    //  "Main Deck" always maps to "" (means top-level / no parent).
    //  Mirrors Python's fill_deck_parent().
    // ─────────────────────────────────────────────────────────
    void fillDeckParent() {
        deckName2Id.clear();
        deckName2Id["Main Deck"] = ""; // "" = no parent (top-level deck)

        for (auto& [id, deck] : userData.items()) {
            if (deck.contains("deck_name")) {
                deckName2Id[deck["deck_name"].get<string>()] = id;
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    //  updateDeckInfo
    //  Reads the currently selected deck from userData and
    //  populates deckInfo with {deckName, subDeckName, description}.
    //  Must be called before any operation that reads deckInfo.
    //  Mirrors Python's update_deck_info().
    // ─────────────────────────────────────────────────────────
    void updateDeckInfo() {
        const string& did = currentViewDeck.first;
        const string& sid = currentViewDeck.second;

        if (!userData.contains(did)) return;

        string deckName = userData[did]["deck_name"].get<string>();

        if (!sid.empty() && userData[did]["subdecks"].contains(sid)) {
            // Sub-deck selected
            string subName = userData[did]["subdecks"][sid]["deck_name"].get<string>();
            string desc    = userData[did]["subdecks"][sid]["description"].get<string>();
            deckInfo = {deckName, subName, desc};
        } else {
            // Top-level deck selected
            string desc = userData[did]["description"].get<string>();
            deckInfo = {deckName, "", desc};
        }
    }

    // ─────────────────────────────────────────────────────────
    //  generateId
    //  Generates a unique deck ID from the current Unix timestamp.
    //  Mirrors Python's:  math.floor(datetime.now().timestamp())
    //  Returns a string because JSON object keys must be strings.
    // ─────────────────────────────────────────────────────────
    string generateId() {
        auto now = chrono::system_clock::now();
        auto ts  = chrono::duration_cast<chrono::seconds>(
                       now.time_since_epoch()).count();
        return to_string(ts);
    }

    // ─────────────────────────────────────────────────────────
    //  formatTimestamp
    //  Converts a Unix timestamp string to a human-readable date.
    //  Mirrors Python's:
    //    datetime.fromtimestamp(ts).strftime("%B %d, %Y %I:%M %p")
    //  Example output: "June 01, 2025 02:30 PM"
    // ─────────────────────────────────────────────────────────
    string formatTimestamp(const string& tsStr) {
        time_t ts = (time_t)stoll(tsStr);
        tm* t = localtime(&ts);
        ostringstream oss;
        oss << put_time(t, "%B %d, %Y %I:%M %p");
        return oss.str();
    }

    // ─────────────────────────────────────────────────────────
    //  listDecks  (READ)
    //  Prints all top-level decks and their sub-decks to the
    //  console, indented to show hierarchy.
    //  Mirrors Python's updateData() display loop.
    //
    //  Each deck is printed with its index for menu selection.
    //  Returns a flat list of {deckId, subDeckId} pairs in the
    //  order displayed, so callers can map user input → deck.
    // ─────────────────────────────────────────────────────────
    vector<pair<string,string>> listDecks() {
        vector<pair<string,string>> indexed; // maps display index → {did, sid}

        if (userData.empty()) {
            cout << "  (No decks yet — use [A] to add one)\n";
            return indexed;
        }

        int i = 1;
        for (auto& [id, deck] : userData.items()) {
            // Print top-level deck
            cout << "  [" << i << "] "
                 << deck["deck_name"].get<string>();

            if (!deck["description"].get<string>().empty())
                cout << "  —  " << deck["description"].get<string>();

            cout << "\n";
            indexed.push_back({id, ""});
            i++;

            // Print sub-decks indented underneath
            if (deck.contains("subdecks")) {
                for (auto& [sid, subdeck] : deck["subdecks"].items()) {
                    cout << "      [" << i << "] "
                         << subdeck["deck_name"].get<string>();

                    if (!subdeck["description"].get<string>().empty())
                        cout << "  —  " << subdeck["description"].get<string>();

                    cout << "\n";
                    indexed.push_back({id, sid});
                    i++;
                }
            }
        }

        return indexed; // caller uses this to resolve "user picked [3]" → deck ids
    }

    // ─────────────────────────────────────────────────────────
    //  createDeck  (CREATE)
    //  Prompts the user for a deck name, description, and parent.
    //  Creates the deck in userData and saves immediately.
    //  Mirrors Python's push_entry(SetMode.ADD).
    //
    //  If parent == "Main Deck" (or no parent):
    //    Creates a top-level deck with a "subdecks" field.
    //  If parent is an existing deck name:
    //    Creates a sub-deck nested inside that parent's "subdecks".
    // ─────────────────────────────────────────────────────────
    void createDeck() {
        cout << "\033[2J\033[1;1H";
        cout << "=== ADD A DECK ===\n\n";

        // --- Deck Name (required, cannot be blank) ---
        string name;
        cout << "Deck Name: ";
        getline(cin >> ws, name);
        while (name.empty()) {
            cout << "[!] Name cannot be blank. Try again: ";
            getline(cin >> ws, name);
        }

        // --- Deck Description (optional) ---
        string desc;
        cout << "Deck Description (optional, press Enter to skip): ";
        getline(cin, desc);

        // --- Parent Deck selection ---
        // Build and display the list of valid parent options
        cout << "\nSelect Parent Deck:\n";
        vector<string> parentOptions;
        parentOptions.push_back("Main Deck"); // index 0 → top-level
        int idx = 1;
        for (auto& [id, deck] : userData.items()) {
            cout << "  [" << idx << "] " << deck["deck_name"].get<string>() << "\n";
            parentOptions.push_back(id);
            idx++;
        }
        cout << "  [0] Main Deck (no parent / top-level)\n";
        cout << ": ";

        int parentChoice = 0;
        while (!(cin >> parentChoice) || parentChoice < 0 || parentChoice >= (int)parentOptions.size()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "[!] Invalid choice. Enter 0 for Main Deck or a number from the list: ";
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        // --- Generate ID and creation date ---
        string newId   = generateId();
        string created = formatTimestamp(newId);

        if (parentChoice == 0) {
            // ── Create TOP-LEVEL deck ──
            // Top-level decks have a "subdecks" object (can contain children)
            userData[newId] = {
                {"deck_name",     name},
                {"description",   desc},
                {"creation_date", created},
                {"subdecks",      json::object()} // empty subdecks map
            };
            cout << "\n[+] Top-level deck \"" << name << "\" created.\n";

        } else {
            // ── Create SUB-DECK inside selected parent ──
            // Sub-decks have a "cards" object instead of subdecks
            string parentId = parentOptions[parentChoice];
            if (!userData.contains(parentId)) {
                cout << "[!] Parent deck not found. Deck was not created.\n";
                return;
            }
            userData[parentId]["subdecks"][newId] = {
                {"deck_name",     name},
                {"description",   desc},
                {"creation_date", created},
                {"cards",         json::object()} // empty cards map
            };
            cout << "\n[+] Sub-deck \"" << name << "\" created under \""
                 << userData[parentId]["deck_name"].get<string>() << "\".\n";
        }

        saveData();      // persist immediately
        fillDeckParent(); // rebuild name→id map
    }

    // ─────────────────────────────────────────────────────────
    //  updateDeck  (UPDATE)
    //  Lets the user rename a deck or change its description.
    //  Requires a deck to already be selected (currentViewDeck).
    //  Mirrors Python's push_entry(SetMode.EDIT).
    //
    //  Pressing Enter without typing keeps the existing value,
    //  so the user can update just one field at a time.
    // ─────────────────────────────────────────────────────────
    void updateDeck() {
        // Guard: nothing is selected
        if (currentViewDeck.first.empty()) {
            cout << "[!] No deck selected. Select a deck first with [S].\n";
            return;
        }

        updateDeckInfo(); // make sure deckInfo is current before editing

        const string& did = currentViewDeck.first;
        const string& sid = currentViewDeck.second;

        cout << "\033[2J\033[1;1H";
        cout << "=== EDIT DECK ===\n\n";
        cout << "Editing: " << deckInfo[(int)CardInfo::DECK];
        if (!deckInfo[(int)CardInfo::SUBDECK].empty())
            cout << " > " << deckInfo[(int)CardInfo::SUBDECK];
        cout << "\n\n";
        cout << "(Press Enter to keep the current value)\n\n";

        // --- New name ---
        string currentName = sid.empty()
            ? deckInfo[(int)CardInfo::DECK]
            : deckInfo[(int)CardInfo::SUBDECK];

        cout << "Deck Name [" << currentName << "]: ";
        string newName;
        getline(cin >> ws, newName);
        if (newName.empty()) newName = currentName; // keep existing if blank

        // --- New description ---
        string currentDesc = deckInfo[(int)CardInfo::DESC];
        cout << "Description [" << currentDesc << "]: ";
        string newDesc;
        getline(cin, newDesc);
        if (newDesc.empty()) newDesc = currentDesc; // keep existing if blank

        // --- Apply changes to userData ---
        if (!sid.empty()) {
            // Editing a sub-deck
            userData[did]["subdecks"][sid]["deck_name"]   = newName;
            userData[did]["subdecks"][sid]["description"] = newDesc;
        } else {
            // Editing a top-level deck
            userData[did]["deck_name"]   = newName;
            userData[did]["description"] = newDesc;
        }

        saveData();       // persist immediately
        fillDeckParent(); // rebuild name→id map (name may have changed)
        updateDeckInfo(); // refresh deckInfo with new values

        cout << "\n[~] Deck updated successfully.\n";
    }

    // ─────────────────────────────────────────────────────────
    //  deleteDeck  (DELETE)
    //  Asks for confirmation then permanently removes the
    //  currently selected deck from userData and saves.
    //  Mirrors Python's delete_deck().
    //
    //  If a top-level deck is deleted, ALL its sub-decks are
    //  also removed (they live inside the parent's JSON object).
    //  If a sub-deck is deleted, only that sub-deck is removed.
    //
    //  After deletion, currentViewDeck is cleared (no selection).
    // ─────────────────────────────────────────────────────────
    void deleteDeck() {
        // Guard: nothing is selected
        if (currentViewDeck.first.empty()) {
            cout << "[!] No deck selected. Select a deck first with [S].\n";
            return;
        }

        updateDeckInfo(); // make sure deckInfo is current

        const string& did = currentViewDeck.first;
        const string& sid = currentViewDeck.second;

        // Determine which name to show in the confirmation prompt
        string deckToDelete = sid.empty()
            ? deckInfo[(int)CardInfo::DECK]
            : deckInfo[(int)CardInfo::SUBDECK];

        // --- Confirmation prompt (mirrors Python's messagebox.askyesno) ---
        cout << "\n[!] Are you sure you want to delete \""
             << deckToDelete << "\"?";
        if (sid.empty())
            cout << "\n    WARNING: All sub-decks inside will also be deleted!";
        cout << "\n    [Y] Yes  [N] No\n: ";

        char confirm;
        cin >> confirm;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (confirm != 'Y' && confirm != 'y') {
            cout << "[-] Delete cancelled.\n";
            return;
        }

        // --- Perform deletion ---
        if (!sid.empty()) {
            // Delete sub-deck from parent's subdecks object
            userData[did]["subdecks"].erase(sid);
            cout << "\n[x] Sub-deck \"" << deckToDelete << "\" deleted.\n";
        } else {
            // Delete entire top-level deck (including all its sub-decks)
            userData.erase(did);
            cout << "\n[x] Deck \"" << deckToDelete << "\" and all its sub-decks deleted.\n";
        }

        // --- Clear selection and persist ---
        currentViewDeck = {"", ""};
        deckInfo        = {"", "", ""};

        saveData();       // persist immediately
        fillDeckParent(); // rebuild name→id map
    }

    // ─────────────────────────────────────────────────────────
    //  selectDeck
    //  Lets the user pick a deck from the displayed list by
    //  entering its index number. Sets currentViewDeck and
    //  prints the selected deck's details.
    //  Mirrors Python's click_event_view() triggered by mouse click.
    //
    //  Returns true if a valid deck was selected, false otherwise.
    // ─────────────────────────────────────────────────────────
    bool selectDeck(const vector<pair<string,string>>& indexed, int choice) {
        // Convert 1-based user input to 0-based vector index
        int idx = choice - 1;
        if (idx < 0 || idx >= (int)indexed.size()) {
            cout << "[!] Invalid selection.\n";
            return false;
        }

        currentViewDeck = indexed[idx];
        updateDeckInfo(); // populate deckInfo for the selected deck

        // Display selected deck info (mirrors Python's click_event_view display)
        cout << "\n--- Selected Deck ---\n";
        cout << "Deck:        " << deckInfo[(int)CardInfo::DECK] << "\n";
        if (!deckInfo[(int)CardInfo::SUBDECK].empty())
            cout << "Sub-deck:    " << deckInfo[(int)CardInfo::SUBDECK] << "\n";
        cout << "Description: " << deckInfo[(int)CardInfo::DESC] << "\n";
        cout << "─────────────────────\n";

        return true;
    }

    // ─────────────────────────────────────────────────────────
    //  clearSelection
    //  Deselects the current deck, disabling Edit and Delete.
    //  Mirrors Python's no_select_deck().
    // ─────────────────────────────────────────────────────────
    void clearSelection() {
        currentViewDeck = {"", ""};
        deckInfo        = {"", "", ""};
        cout << "[-] Selection cleared.\n";
    }

    // ─────────────────────────────────────────────────────────
    //  toDeckObjects
    //  Converts the flat JSON userData into a vector<Deck> tree,
    //  populating all fields including subDecks and cardsIncSub.
    //  Use this when passing deck data to other modules that
    //  expect the Deck struct (e.g. a study/quiz engine).
    // ─────────────────────────────────────────────────────────
    vector<Deck> toDeckObjects() const {
        vector<Deck> result;

        for (auto& [id, d] : userData.items()) {
            Deck deck;
            deck.deckId      = id;
            deck.deckName    = d.value("deck_name",     "");
            deck.description = d.value("description",   "");
            deck.creationDate= d.value("creation_date", "");
            deck.level       = 0; // top-level

            // Load direct cards (if any)
            if (d.contains("cards")) {
                for (auto& [cid, c] : d["cards"].items()) {
                    deck.cards.push_back({
                        c.value("front", ""),
                        c.value("back",  "")
                    });
                }
            }

            // Load sub-decks
            if (d.contains("subdecks")) {
                for (auto& [sid, sd] : d["subdecks"].items()) {
                    Deck sub;
                    sub.deckId      = sid;
                    sub.deckName    = sd.value("deck_name",     "");
                    sub.description = sd.value("description",   "");
                    sub.creationDate= sd.value("creation_date", "");
                    sub.level       = 1; // sub-deck

                    if (sd.contains("cards")) {
                        for (auto& [cid, c] : sd["cards"].items()) {
                            sub.cards.push_back({
                                c.value("front", ""),
                                c.value("back",  "")
                            });
                        }
                    }

                    // cardsIncSub for the sub-deck is just its own cards
                    sub.cardsIncSub = sub.cards;
                    deck.subDecks.push_back(sub);

                    // Accumulate into parent's cardsIncSub
                    deck.cardsIncSub.insert(
                        deck.cardsIncSub.end(),
                        sub.cards.begin(), sub.cards.end()
                    );
                }
            }

            // cardsIncSub = own cards + all sub-deck cards
            deck.cardsIncSub.insert(
                deck.cardsIncSub.begin(),
                deck.cards.begin(), deck.cards.end()
            );

            result.push_back(deck);
        }

        return result;
    }

    // ─────────────────────────────────────────────────────────
    //  runDeckMenu  (main interactive loop for deck management)
    //  Displays the deck list and action menu, then dispatches
    //  to Create / Read / Update / Delete based on user input.
    //  Call this from your Profile Dashboard state in main.cpp.
    //
    //  Returns when the user chooses to go back (pressing 0).
    // ─────────────────────────────────────────────────────────
    void runDeckMenu() {
        while (true) {
            cout << "\033[2J\033[1;1H";
            cout << "=== DECK MANAGER ===\n\n";

            // READ: display all decks
            auto indexed = listDecks();
            cout << "\n";

            // Show currently selected deck (if any)
            if (!currentViewDeck.first.empty()) {
                cout << "Selected: [" << deckInfo[(int)CardInfo::DECK];
                if (!deckInfo[(int)CardInfo::SUBDECK].empty())
                    cout << " > " << deckInfo[(int)CardInfo::SUBDECK];
                cout << "]\n";
            }

            // Action menu
            cout << "\n[A] Add Deck   ";
            if (!currentViewDeck.first.empty()) {
                cout << "[E] Edit Deck  [D] Delete Deck  [X] Deselect";
            }
            cout << "\n[S] Select Deck   [0] Back\n: ";

            char action;
            cin >> action;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            switch (toupper(action)) {

                case 'A':
                    // CREATE
                    createDeck();
                    pressEnterToContinue();
                    break;

                case 'E':
                    // UPDATE — only available when a deck is selected
                    if (currentViewDeck.first.empty()) {
                        cout << "[!] Select a deck first with [S].\n";
                        pressEnterToContinue();
                    } else {
                        updateDeck();
                        pressEnterToContinue();
                    }
                    break;

                case 'D':
                    // DELETE — only available when a deck is selected
                    if (currentViewDeck.first.empty()) {
                        cout << "[!] Select a deck first with [S].\n";
                        pressEnterToContinue();
                    } else {
                        deleteDeck();
                        pressEnterToContinue();
                    }
                    break;

                case 'S': {
                    // SELECT a deck by index
                    if (indexed.empty()) {
                        cout << "[!] No decks to select.\n";
                        pressEnterToContinue();
                        break;
                    }
                    cout << "Enter deck number: ";
                    int choice;
                    while (!(cin >> choice)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "[!] Invalid input. Enter a number: ";
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    selectDeck(indexed, choice);
                    pressEnterToContinue();
                    break;
                }

                case 'X':
                    // DESELECT
                    clearSelection();
                    pressEnterToContinue();
                    break;

                case '0':
                    // Back to caller (Profile Dashboard / main.cpp)
                    return;

                default:
                    cout << "[!] Unknown option.\n";
                    pressEnterToContinue();
                    break;
            }
        }
    }

private:
    // ─────────────────────────────────────────────────────────
    //  pressEnterToContinue
    //  Small helper to pause the screen before redrawing.
    // ─────────────────────────────────────────────────────────
    void pressEnterToContinue() {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
};
