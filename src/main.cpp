#include "../include/data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <limits>
#include <vector>
#include <unordered_map>

using namespace std;

char getChoice() {
    char choice;
    cout << ": ";
    while(!(cin >> choice)) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid Input! Please Try again: ";
    }
    return choice;
}

void saveToFile(const std::unordered_map<string, Account> &accounts) {
    json j = accounts;
    ofstream outputFile("data.json");
    outputFile << j.dump(4);
    outputFile.close();
}

unordered_map<string, Account> loadFromFile() {
    ifstream file("data.json");
    if(!file.is_open()) return {};
    json j;
    file >> j;
    return j.get<unordered_map<string, Account>>();   
}

enum class AppState {
    MAIN_MENU,
    LOGIN,
    SIGNUP,
    PROFILES,
    CREATE_PROFILE,
    PROFILE_DASHBOARD,
    CREATE_DECK,
    DECK_DASHBOARD,
    CARD_MANAGEMENT,
    
    EXIT
};

AppState handleMainMenu() {
    std::cout << "\033[2J\033[1;1H";
    cout << "===== ANKI IN C++ =====\n";
    cout << "[1] Login\n[2] Sign Up\n[0] Exit\n";

    char choice = getChoice();
    if(choice == '1') return AppState::LOGIN;
    if(choice == '2') return AppState::SIGNUP;
    if(choice == '0') return AppState::EXIT;

    return AppState::MAIN_MENU;
}

AppState handleLogin(unordered_map<string, Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== LOGIN ===\n";
    cout << "Enter Username: ";
    string username, password;
    getline(cin >> ws, username);
    cout << "Enter Password: ";
    getline(cin, password);
    if(allAccounts.find(username) == allAccounts.end()) {
        cout << "Account does not exist or Password is Incorrect, Please try again\n";
        cout << "Press Enter to try again...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::LOGIN;
    }
    if(allAccounts[username].password != password) {
        cout << "Account does not exist or Password is Incorrect, Please try again\n";
        cout << "Press Enter to try again...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::LOGIN;
    }

    activeUser = &allAccounts[username]; // Sets active session user pointer cleanly
    return AppState::PROFILES;
}

AppState handleSignup(unordered_map<string, Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== SIGNUP ===\n";
    cout << "Enter Username: ";
    string username, password;
    getline(cin >> ws, username);
    while(allAccounts.find(username) != allAccounts.end()) {
        std::cout << "\033[2J\033[1;1H";
        cout << "=== SIGNUP ===\n";
        cout << "Account already exists/Username has already been taken! Please try again\n";
        cout << "Enter Username: ";
        getline(cin >> ws, username);
    }
    cout << "Enter Password: ";
    getline(cin, password);
    char confirm;
    cout << "Are you sure about this Username and Password? [Y/n]: ";
    cin >> confirm;
    if(confirm == 'Y') {
        Account newAcc(username, password);
        allAccounts[username] = newAcc;
        saveToFile(allAccounts);
        activeUser = &allAccounts[username];
        
        return AppState::PROFILES;
    }

    return AppState::SIGNUP;
}

// =========================================================================
// PRODUCTION CARD MANAGEMENT - TARGETING REAL LIVE STRUCTURE DATA POINTERS
// =========================================================================
AppState handleCardManagement(Deck* activeDeck, unordered_map<string, Account> &allAccounts) {
    if (!activeDeck) {
        cout << "Error: No active deck selected! Returning to Dashboard...\n";
        cout << "Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::DECK_DASHBOARD;
    }

    int choice = -1;
    while (choice != 0) {
        std::cout << "\033[2J\033[1;1H"; 
        cout << "===========================================\n";
        cout << "  MANAGING CARDS PANEL (" << activeDeck->deckName << ")\n";
        cout << "===========================================\n";
        cout << "[1] Create (Add Card & Tag Type Layout)\n";
        cout << "[2] Read (View Existing Cards)\n";
        cout << "[3] Update (Edit Card Content)\n";
        cout << "[4] Delete (Remove Card From Deck Collection)\n";
        cout << "[0] Exit back to Dashboard\n";
        cout << "Choice: ";
        
        if (!(cin >> choice)) {
            cout << "Please enter a valid number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        // --- CREATE ---
        if (choice == 1) {
            std::cout << "\033[2J\033[1;1H";
            cout << "--- SELECT CARD TYPE STYLE layout ---\n";
            cout << "[0] Basic (Standard)\n";
            cout << "[1] Basic (and reversed card variant)\n";
            cout << "[2] Basic (type in the answer field layout)\n";
            cout << "Choice: ";
            int typeChoice;
            if (!(cin >> typeChoice) || typeChoice < 0 || typeChoice > 2) typeChoice = 0;
            
            string front, back;
            cout << "\nEnter Front (Question/Prompt): ";
            getline(cin >> ws, front);
            cout << "Enter Back (Answer/Definition): ";
            getline(cin, back);

            // Directly pushed to your database struct!
            activeDeck->cards.push_back(Card(front, back, typeChoice));
            saveToFile(allAccounts); // Instantly commits updates to data.json

            cout << "\nSuccess: Card added and synchronized directly! Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
        // --- READ ---
        else if (choice == 2) {
            std::cout << "\033[2J\033[1;1H";
            cout << "--- CURRENT ACTIVE CARDS IN DECK ---\n";
            if (activeDeck->cards.empty()) {
                cout << "[This deck is completely empty. Go add some cards!]\n";
            } else {
                for (size_t i = 0; i < activeDeck->cards.size(); ++i) {
                    string typeLabel = (activeDeck->cards[i].type == 1) ? "Basic (and reversed)" : 
                                       (activeDeck->cards[i].type == 2) ? "Basic (type-in answer)" : "Basic Standard";
                    cout << "[" << i + 1 << "] Template Configuration: " << typeLabel << "\n"
                         << "    Front Field: " << activeDeck->cards[i].front << "\n"
                         << "    Back Field:  " << activeDeck->cards[i].back << "\n"
                         << "-------------------------------------------\n";
                }
            }
            cout << "Press Enter to go back to control screen...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
        // --- UPDATE ---
        else if (choice == 3) {
            cout << "\nEnter Card Index Number to Edit: ";
            int index;
            if (cin >> index && index > 0 && index <= static_cast<int>(activeDeck->cards.size())) {
                index--;
                string newFront, newBack;
                cout << "New Front (Leave empty + hit Enter to retain original value): ";
                getline(cin >> ws, newFront);
                cout << "New Back (Leave empty + hit Enter to retain original value): ";
                getline(cin, newBack);

                if (!newFront.empty()) activeDeck->cards[index].front = newFront;
                if (!newBack.empty()) activeDeck->cards[index].back = newBack;
                
                saveToFile(allAccounts); // Writes modifications straight to the JSON disk file
                cout << "Success: Card information updated in storage!\n";
            } else {
                cout << "Invalid index selection bound context!\n";
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Press Enter to continue...";
            cin.get();
        }
        // --- DELETE ---
        else if (choice == 4) {
            cout << "\nEnter Card Index Number to Erase: ";
            int index;
            if (cin >> index && index > 0 && index <= static_cast<int>(activeDeck->cards.size())) {
                index--;
                
                // Safe lookup erase directly from the deck card vector vector matrix array track
                activeDeck->cards.erase(activeDeck->cards.begin() + index);
                saveToFile(allAccounts); // Drops updates safely down to backend file
                
                cout << "Success: Card removed cleanly from persistent system tracking layout memory.\n";
            } else {
                cout << "Invalid index bounds selection assignment!\n";
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Press Enter to continue...";
            cin.get();
        }
    }
    return AppState::DECK_DASHBOARD; 
}

int main() {
    unordered_map<string, Account> allAccounts = loadFromFile();
    Account* activeUser = nullptr;

    AppState currentState = AppState::MAIN_MENU;
    while(currentState != AppState::EXIT) {
        switch(currentState) {
            case AppState::MAIN_MENU:
                currentState = handleMainMenu();
                break;
            case AppState::LOGIN:
                currentState = handleLogin(allAccounts, activeUser);
                break;
            case AppState::SIGNUP:
                currentState = handleSignup(allAccounts, activeUser);
                break;
            case AppState::DECK_DASHBOARD:
                // Normal navigation handles moving the user down to CARD_MANAGEMENT state when invoked
                break;
            case AppState::CARD_MANAGEMENT: {
                Deck* activeDeckPtr = nullptr;
                if (activeUser && !activeUser->profiles.empty() && !activeUser->profiles[0].decks.empty()) {
                    activeDeckPtr = &activeUser->profiles[0].decks[0]; 
                }
                
                currentState = handleCardManagement(activeDeckPtr, allAccounts);
                break;
            }
            default:
                currentState = AppState::MAIN_MENU;
                break;
        }
    }

    return 0;
}