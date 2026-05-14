#include "../include/data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>

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