#include "../include/data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>

using namespace std;

// Sample change
// As you can see, there is a green line near the line number on the left side of this comment
// That would signify new lines of code that was not present in the previous version of this branch
// Red arrow would point to where you need to click when you want to save/send your current progress, usually when you're done with a feature or you want to get it checked

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

// Allan's Card Feature Testing Screen
AppState handleCardManagement() {
    int choice = -1;
    
    // Local testing variables (Simulating a Deck)
    // TODO for Team: Connect these to activeUser->profiles[0].decks[0].cards
    vector<string> testFronts;
    vector<string> testBacks;
    vector<int> testTypes;

    while (choice != 0) {
        std::cout << "\033[2J\033[1;1H"; 
        cout << "===========================================\n";
        cout << "  MANAGING CARDS PANEL (TEST MODE)\n";
        cout << "===========================================\n";
        cout << "[1] Create (Add Card & Tag Type)\n";
        cout << "[2] Read (View Existing Cards)\n";
        cout << "[3] Update (Edit Card Data)\n";
        cout << "[4] Delete (Remove Card)\n";
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
            cout << "--- SELECT CARD TYPE ---\n";
            cout << "[0] Basic (Standard)\n";
            cout << "[1] Basic (and reversed card)\n";
            cout << "[2] Basic (type in the answer)\n";
            cout << "Choice: ";
            int typeChoice;
            if (!(cin >> typeChoice) || typeChoice < 0 || typeChoice > 2) typeChoice = 0;
            
            string front, back;
            cout << "\nEnter Front (Question/Prompt): ";
            getline(cin >> ws, front);
            cout << "Enter Back (Answer/Definition): ";
            getline(cin, back);

            // Storing locally for now
            testFronts.push_back(front);
            testBacks.push_back(back);
            testTypes.push_back(typeChoice);

            // TODO for Team: Call saveToFile() here when integrating JSON
            cout << "\nSuccess: Card added locally! Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
        // --- READ ---
        else if (choice == 2) {
            std::cout << "\033[2J\033[1;1H";
            cout << "--- CURRENT DECK CARDS ---\n";
            if (testFronts.empty()) {
                cout << "[This deck is completely empty]\n";
            } else {
                for (size_t i = 0; i < testFronts.size(); ++i) {
                    string typeLabel = (testTypes[i] == 1) ? "Basic (and reversed)" : 
                                       (testTypes[i] == 2) ? "Basic (type-in answer)" : "Basic";
                    cout << "[" << i + 1 << "] Type: " << typeLabel << "\n"
                         << "    Front: " << testFronts[i] << "\n"
                         << "    Back:  " << testBacks[i] << "\n"
                         << "-------------------------------------------\n";
                }
            }
            cout << "Press Enter to go back...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
        // --- UPDATE ---
        else if (choice == 3) {
            cout << "\nEnter Card Index Number to Edit: ";
            int index;
            if (cin >> index && index > 0 && index <= static_cast<int>(testFronts.size())) {
                index--;
                string newFront, newBack;
                cout << "New Front (Leave empty + hit Enter to retain): ";
                getline(cin >> ws, newFront);
                cout << "New Back (Leave empty + hit Enter to retain): ";
                getline(cin, newBack);

                if (!newFront.empty()) testFronts[index] = newFront;
                if (!newBack.empty()) testBacks[index] = newBack;
                
                cout << "Success: Card information updated locally!\n";
            } else {
                cout << "Invalid index selection!\n";
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
            if (cin >> index && index > 0 && index <= static_cast<int>(testFronts.size())) {
                index--;
                testFronts.erase(testFronts.begin() + index);
                testBacks.erase(testBacks.begin() + index);
                testTypes.erase(testTypes.begin() + index);
                cout << "Success: Card removed cleanly from local memory.\n";
            } else {
                cout << "Invalid selection!\n";
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Press Enter to continue...";
            cin.get();
        }
    }
    return AppState::MAIN_MENU; 
}
// AppState handleExit() {
    // progress saving could also happen here'
// }

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
                // Instantly jumps straight into your function to test it
                currentState = handleCardManagement();
                break;
        }
    }

    // json j_class = sampleAccount;
    // cout << j_class.dump(4);

    // ofstream output_file("data.json");
    // output_file << j_class.dump(4);
    // output_file.close();

    // Account current;
    // cout << "Welcome to Anki!\n";
    // while(true) {
    //     cout << "[1] Login\n[2] Sign Up\n[0] Exit\n: ";
    //     int choice;
    //     cin >> choice;
    //     if (choice == 0) {
    //         break;
    //     } else if (choice == 1) {
    //         cout << "No Accounts Found";
    //     } else if (choice == 2) {
    //         while (true) {
    //             string username, password, confirm;
    //             cout <<  "Username: ";
    //             cin >> username;
    //             cout << "Password: ";
    //             cin >> password;
    //             cout << "Are you sure about this Username and Password? [y/n]: ";
    //             cin >> confirm;
    //             if (confirm == "y") {
    //                 current.accountName = username;
    //                 current.password = password;

    //                 json j_class = current;

    //                 ofstream output_file("data.json");
    //                 output_file << j_class.dump(4);
    //                 output_file.close();

    //                 break;
    //             }
    //         }
    //     } else {
    //         cout << "Invalid Option";
    //     }
    // }

    // Account sampleAccount("Sample Account", "Sample Password");
    
    // Profile sampleProfileOne("Profile One");
    // Profile sampleProfileTwo("Profile Two");
    // Profile sampleProfileThree("Profile Three");

    // Deck sampleDeckOne("Deck One", 0);
    // Deck sampleDeckTwo("Deck Two", 0);
    // Deck sampleDeckThree("Deck Three", 0);

    // Card sampleCardOne("Question One", "Answer One");
    // Card sampleCardTwo("Question Two", "Answer Two");
    // Card sampleCardThree("Question Three", "Answer Three");
    // Card sampleCardFour("Question Four", "Answer Four");
    // Card sampleCardFive("Question Five", "Answer Five");
    // Card sampleCardSix("Question Six", "Answer Six");
    // Card sampleCardSeven("Question Seven", "Answer Seven");

    // sampleDeckOne.cards = {sampleCardOne, sampleCardTwo};
    // sampleDeckTwo.cards = {sampleCardThree, sampleCardFour, sampleCardFive};
    // sampleDeckThree.cards = {sampleCardOne, sampleCardTwo, sampleCardFour, sampleCardSix, sampleCardSeven};

    // sampleProfileOne.decks = {sampleDeckOne, sampleDeckTwo, sampleDeckThree};
    // sampleProfileTwo.decks = {sampleDeckTwo};
    // sampleProfileThree.decks = {sampleDeckOne, sampleDeckThree};

    // sampleAccount.profiles = {sampleProfileOne, sampleProfileTwo, sampleProfileThree};

    return 0;
}   