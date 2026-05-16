#include "../include/data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>

using namespace std;

/*
    getChoice Function:
        Handles user input in decisions like:
            [1] Login
            [2] Sign Up
            [0] Exit
        Will also automatically print the colon space
        Returns a character since a decision like this might show up later:
            [1] Deck 6
            [2] Deck 7
            [3] Deck 8
            [4] Deck 9
            [5] Deck 10
            [<] Previous Page
            [>] Next Page
            [8] Create Deck
            [9] Settings
            [0] Back
*/
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

/*
    saveToFile Function:
        Handles saving the user data into the json file
        Takes only one parameter:  
            Reference to the instance of an accounts class
        Since our data structure nests the cards, decks, and profiles all under the accounts, triggering the class to json conversion on the account class would subsequently trigger the class to json conversions of the profile, deck, and card classes.
*/
void saveToFile(const std::unordered_map<string, Account> &accounts) {
    json j = accounts;
    ofstream outputFile("data.json");
    outputFile << j.dump(4);
    outputFile.close();
}

/*
    loadFromFile Function:
        Handles the extraction of data from the json file into an unordered map of account classes usable in C++ code
        It returns the onrodered map itself
*/
unordered_map<string, Account> loadFromFile() {
    ifstream file("data.json");
    if(!file.is_open()) return {};
    json j;
    file >> j;
    return j.get<unordered_map<string, Account>>();   
}

/*
    enum AppState Class:
        This contains the names of our so-called "pages" in our command line user interface
        Keep in mind when adding a "page":
            All caps, replace spaces with underscore
            Try to keep the EXIT at the bottom of the list for easier readability
        Below this enum class are the functions that handle each page, next set of comments would help explain

        As of 05/16/2026, not all pages have been added

*/
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

/*
    AppState Handle Functions:
        Below are and would be the handle functions of each AppState
        These would contain the user interactions such as the text seen in the command line and the user input
            Ex: MAIN_MENU
                ===== ANKI IN C++ =====
                [1] Login
                [2] Sign Up
                [0] Exit
                : 
            Ex: LOGIN
                === LOGIN ===
                Enter Username: [user input here]
                Enter Password: [user input here]
        Naming convention: Lower Camel Case
        First line of the function should always be, this clears the command line before printing:
            std::cout << "\033[2J\033[1;1H";
*/

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