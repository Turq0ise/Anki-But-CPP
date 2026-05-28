/*
 * main.cpp
 * --------
 * Entry point for the Anki C++ terminal app.
 *
 * The app uses a simple state machine pattern — instead of nested
 * loops or recursive calls, every screen is a separate function that
 * returns the next AppState. The main loop in main() switches on
 * that state and calls the matching handler.
 *
 * States currently implemented:
 *   MAIN_MENU        → landing screen (Login / Sign Up / Exit)
 *   LOGIN            → username + password check against data.json
 *   SIGNUP           → create a new account, saved immediately
 *   PROFILES         → list profiles for the active account
 *   CREATE_PROFILE   → add a new profile to the active account
 *
 * States stubbed (ready for you to implement):
 *   PROFILE_DASHBOARD → view/manage decks inside a profile
 *   CREATE_DECK       → create a new deck inside a profile
 *   DECK_DASHBOARD    → view/study cards inside a deck
 *
 * Data flow:
 *   All accounts are loaded from data.json on startup via loadFromFile().
 *   Any change (signup, profile creation, etc.) is immediately written
 *   back to data.json via saveToFile().
 */

#include "data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

// ─────────────────────────────────────────────────────────────
//  getChoice
//  Reads a single character from the user for menu selection.
//  Loops and shows an error message if the input stream fails
//  (e.g. the user sends EOF or a non-character value).
//  Returns the validated character.
// ─────────────────────────────────────────────────────────────

char getChoice() {
    char choice;
    cout << ": ";
    while (!(cin >> choice)) {
        cin.clear();                   // clear the error flag on cin
        cin.ignore(1000, '\n');        // discard the bad input from the buffer
        cout << "Invalid Input! Please try again: ";
    }
    return choice;
}

// ─────────────────────────────────────────────────────────────
//  saveToFile
//  Serializes the entire accounts map to JSON and writes it to
//  "data.json" in the working directory, overwriting any
//  previous version. Called after any data-modifying action
//  (signup, profile creation, etc.) so nothing is lost.
// ─────────────────────────────────────────────────────────────

void saveToFile(const unordered_map<string, Account> &accounts) {
    json j = accounts;                  // triggers Account's to_json serializer
    ofstream outputFile("data.json");
    outputFile << j.dump(4);            // pretty-print with 4-space indentation
    outputFile.close();
}

// ─────────────────────────────────────────────────────────────
//  loadFromFile
//  Reads "data.json" from the working directory and deserializes
//  it into an unordered_map<string, Account>.
//  Returns an empty map if the file doesn't exist yet (first run).
// ─────────────────────────────────────────────────────────────

unordered_map<string, Account> loadFromFile() {
    ifstream file("data.json");
    if (!file.is_open()) return {};     // no file yet — start fresh
    json j;
    file >> j;
    return j.get<unordered_map<string, Account>>(); // triggers Account's from_json
}

// ─────────────────────────────────────────────────────────────
//  AppState
//  Enum that represents every possible screen / step in the app.
//  Each handler function returns the next state to transition to.
// ─────────────────────────────────────────────────────────────

enum class AppState {
    MAIN_MENU,         // Landing screen
    LOGIN,             // Login form
    SIGNUP,            // Sign up form
    PROFILES,          // Profile selection screen
    CREATE_PROFILE,    // Create a new profile
    PROFILE_DASHBOARD, // View / manage decks in a profile  (TODO)
    CREATE_DECK,       // Create a new deck                 (TODO)
    DECK_DASHBOARD,    // Study / manage cards in a deck    (TODO)
    EXIT               // Exits the main loop
};

// ─────────────────────────────────────────────────────────────
//  handleMainMenu
//  Displays the landing screen with Login, Sign Up, and Exit
//  options. Clears the terminal before rendering.
//  Returns the AppState matching the user's choice.
// ─────────────────────────────────────────────────────────────

AppState handleMainMenu() {
    cout << "\033[2J\033[1;1H";    // ANSI escape: clear screen, move cursor to top
    cout << "===== ANKI IN C++ =====\n";
    cout << "[1] Login\n[2] Sign Up\n[0] Exit\n";

    char choice = getChoice();
    if (choice == '1') return AppState::LOGIN;
    if (choice == '2') return AppState::SIGNUP;
    if (choice == '0') return AppState::EXIT;

    return AppState::MAIN_MENU; // any other key: redisplay the menu
}

// ─────────────────────────────────────────────────────────────
//  handleLogin
//  Prompts for username and password, looks up the account in
//  allAccounts, and checks the password.
//  On success: sets activeUser to point at the matching Account
//              in the map and transitions to PROFILES.
//  On failure: shows an error and loops back to LOGIN.
//
//  Note: both "account not found" and "wrong password" show the
//  same generic error message to avoid revealing which usernames exist.
// ─────────────────────────────────────────────────────────────

AppState handleLogin(unordered_map<string, Account> &allAccounts, Account* &activeUser) {
    cout << "\033[2J\033[1;1H";
    cout << "=== LOGIN ===\n";

    string username, password;
    cout << "Enter Username: ";
    getline(cin >> ws, username);  // cin >> ws skips any leading whitespace/newlines
    cout << "Enter Password: ";
    getline(cin, password);

    // Single lookup — avoids searching the map twice
    auto it = allAccounts.find(username);
    if (it == allAccounts.end() || it->second.password != password) {
        cout << "Account does not exist or password is incorrect.\n";
        cout << "Press Enter to try again...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::LOGIN;
    }

    // Point activeUser at the real Account inside the map
    // (so any changes to activeUser also modify allAccounts)
    activeUser = &it->second;
    return AppState::PROFILES;
}

// ─────────────────────────────────────────────────────────────
//  handleSignup
//  Prompts for a new username and password.
//  Re-prompts for the username if it's already taken.
//  Asks for confirmation before creating the account.
//  On confirm: creates the Account (which auto-creates a Default
//              Profile and Default Deck), saves to file, and
//              transitions to PROFILES.
//  On decline: loops back to SIGNUP.
// ─────────────────────────────────────────────────────────────

AppState handleSignup(unordered_map<string, Account> &allAccounts, Account* &activeUser) {
    cout << "\033[2J\033[1;1H";
    cout << "=== SIGNUP ===\n";

    string username, password;
    cout << "Enter Username: ";
    getline(cin >> ws, username);

    // Keep re-asking until a unique username is entered
    while (allAccounts.find(username) != allAccounts.end()) {
        cout << "\033[2J\033[1;1H";
        cout << "=== SIGNUP ===\n";
        cout << "Username already taken! Please try again.\n";
        cout << "Enter Username: ";
        getline(cin >> ws, username);
    }

    cout << "Enter Password: ";
    getline(cin, password);

    char confirm;
    cout << "Are you sure about this Username and Password? [Y/n]: ";
    cin >> confirm;

    if (confirm == 'Y') {
        // Account constructor automatically creates a Default Profile + Default Deck
        Account newAcc(username, password);
        allAccounts[username] = newAcc;
        saveToFile(allAccounts);           // persist immediately
        activeUser = &allAccounts[username];
        return AppState::PROFILES;
    }

    return AppState::SIGNUP; // user said no — restart the form
}

// ─────────────────────────────────────────────────────────────
//  handleProfiles
//  Lists all profiles belonging to the active account and lets
//  the user pick one to enter, create a new one, or log out.
//  Numbered choices map directly to the profiles vector index.
// ─────────────────────────────────────────────────────────────

AppState handleProfiles(Account* &activeUser) {
    cout << "\033[2J\033[1;1H";
    cout << "=== PROFILES (" << activeUser->accountName << ") ===\n";

    // Print each profile with a 1-based index
    for (int i = 0; i < (int)activeUser->profiles.size(); i++) {
        cout << "[" << (i + 1) << "] " << activeUser->profiles[i].profileName << "\n";
    }
    cout << "[C] Create Profile\n[0] Logout\n";

    char choice = getChoice();
    if (choice == '0') return AppState::MAIN_MENU;          // logout → back to menu
    if (choice == 'C' || choice == 'c') return AppState::CREATE_PROFILE;

    // Convert character digit to zero-based vector index
    int idx = choice - '1';
    if (idx >= 0 && idx < (int)activeUser->profiles.size()) {
        // TODO: store selected profile index and go to PROFILE_DASHBOARD
        return AppState::PROFILE_DASHBOARD;
    }

    return AppState::PROFILES; // invalid choice — redisplay
}

// ─────────────────────────────────────────────────────────────
//  handleCreateProfile
//  Prompts for a new profile name, creates a Profile object
//  (which auto-creates a Default Deck), adds it to the active
//  account, saves to file, and returns to PROFILES.
// ─────────────────────────────────────────────────────────────

AppState handleCreateProfile(unordered_map<string, Account> &allAccounts,
                             Account* &activeUser) {
    cout << "\033[2J\033[1;1H";
    cout << "=== CREATE PROFILE ===\n";
    cout << "Enter Profile Name: ";

    string name;
    getline(cin >> ws, name);

    // Profile constructor automatically creates a Default Deck inside it
    activeUser->profiles.push_back(Profile(name));
    saveToFile(allAccounts);   // write changes to data.json immediately

    cout << "Profile \"" << name << "\" created!\n";
    cout << "Press Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();

    return AppState::PROFILES;
}

// ─────────────────────────────────────────────────────────────
//  main
//  Loads all account data from data.json, then runs the state
//  machine loop until the user chooses EXIT.
//
//  activeUser is a raw pointer into the allAccounts map.
//  It's set by handleLogin / handleSignup and used by any
//  handler that needs to read or modify the logged-in account.
// ─────────────────────────────────────────────────────────────

int main() {
    // Load all saved accounts from disk (empty map if first run)
    unordered_map<string, Account> allAccounts = loadFromFile();

    // Pointer to whichever account is currently logged in (nullptr = no one)
    Account* activeUser = nullptr;

    AppState currentState = AppState::MAIN_MENU;

    // State machine loop — runs until EXIT is returned by any handler
    while (currentState != AppState::EXIT) {
        switch (currentState) {
            case AppState::MAIN_MENU:
                currentState = handleMainMenu();
                break;

            case AppState::LOGIN:
                currentState = handleLogin(allAccounts, activeUser);
                break;

            case AppState::SIGNUP:
                currentState = handleSignup(allAccounts, activeUser);
                break;

            case AppState::PROFILES:
                currentState = handleProfiles(activeUser);
                break;

            case AppState::CREATE_PROFILE:
                currentState = handleCreateProfile(allAccounts, activeUser);
                break;

            // ── TODO states ──────────────────────────────────
            // Implement these handlers and add their cases here
            // when you're ready to build out those screens.
            case AppState::PROFILE_DASHBOARD:
            case AppState::CREATE_DECK:
            case AppState::DECK_DASHBOARD:
                // Placeholder: fall back to profiles until implemented
                currentState = AppState::PROFILES;
                break;

            default:
                currentState = AppState::MAIN_MENU;
                break;
        }
    }

    return 0;
}
