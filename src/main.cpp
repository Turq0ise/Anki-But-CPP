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
    ACCOUNT_SETTINGS,
    PROFILE_DASHBOARD,
    SETTINGS,
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
        These functions return an AppState to indicate what "page" should be displayed to the user next. More on this on the comments on the main function
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

    activeUser = &allAccounts[username];
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
        allAccounts[username] = Account(username, password);
        saveToFile(allAccounts);
        activeUser = &allAccounts[username];
        
        return AppState::PROFILES;
    }

    return AppState::SIGNUP;
}

AppState handleProfiles(unordered_map<string,Account> &allAccounts, Account* &activeUser, int &profilePage, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== PROFILES ===\n";
    cout << "= Page " << profilePage + 1 << " =\n";

    const int limitPerPage = 5;
    int profilesVectorSize = activeUser->profiles.size();
    int runningSize = (profilesVectorSize <= limitPerPage) ? profilesVectorSize : ((profilePage + 1) * limitPerPage) - ((profilesVectorSize / limitPerPage) >= (profilePage + 1) ? 0 : limitPerPage - (profilesVectorSize % limitPerPage));
    for(int i = (5 * profilePage); i < runningSize; i++) {
        int index = (i + 1) - (profilePage * limitPerPage);
        cout << "[" << index << "]: " << activeUser->profiles[i].profileName << "\n";
    }

    cout << "\n";

    int numberOfPages = (profilesVectorSize / limitPerPage) + ( (profilesVectorSize % limitPerPage) > 0 ? 1 : 0);
    if(numberOfPages > 1) {
        if(profilePage == 0) {
            cout << "[>] Next Page";
        } else if(profilePage > 0 && profilePage < (numberOfPages - 1)) {
            cout << "[<] Previous Page  |  Next Page [>]";
        } else if(profilePage == (numberOfPages - 1)) {
            cout << "[<] Previous Page";
        } 
        cout << "\n";
    }
    cout << "[8] Create Profile\n[9] Account Settings\n[0] Back/Sign Out\n";

    char choice = getChoice();
    if(choice == '0') {
        return AppState::MAIN_MENU;
    } else if(choice == '9') {
        return AppState::ACCOUNT_SETTINGS;
    } else if(choice == '8') {
        return AppState::CREATE_PROFILE;
    } else if(choice == '<' && numberOfPages > 1 && ((profilePage > 0 && profilePage < (numberOfPages - 1)) || (profilePage == (numberOfPages - 1)))) {
        profilePage = profilePage - 1;
        return AppState::PROFILES;
    } else if(choice == '>' && numberOfPages > 1 && ((profilePage > 0 && profilePage < (numberOfPages - 1)) || (profilePage == 0))) {
        profilePage = profilePage + 1;
        return AppState::PROFILES;
    }

    if(choice != '1' && choice != '2' && choice != '3' && choice != '4' && choice != '5') {
        return AppState::PROFILES;
    }
    int choiceToInt = choice - '0';
    activeProfile = &activeUser->profiles[(choiceToInt - 1) + (profilePage * limitPerPage)];
    return AppState::PROFILE_DASHBOARD;
}

AppState handleCreateProfile(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== CREATE PROFILE ===\n";
    string profileName;
    cout << "Enter Profile Name: ";
    getline(cin >> ws, profileName);

    bool profileExists = true;
    while (profileExists) {
        profileExists = false;

        for(const Profile &profile : activeUser->profiles) {
            if(profileName == profile.profileName) {
                profileExists = true;
                std::cout << "\033[2J\033[1;1H";
                cout << "=== CREATE PROFILE ===\n";
                cout << "Profile Already Exists, Please Try Again!\n";
                cout << "Enter Profile Name: ";
                getline(cin >> ws, profileName);
                break;
            }
        }
    }

    activeUser->profiles.push_back(Profile(profileName));
    saveToFile(allAccounts);
    cout << "Profile made Successfully!\n";
    cout << "[1] Proceed to " << profileName << "\n[2] Create another Profile\n";
    char choice = getChoice();
    while((choice != '1') && (choice != '2')) {
        cout << "Invalid Input, Please Try Again!\n";
        cout << "[1] Proceed to " << profileName << "\n[2] Create another Profile\n";
        choice = getChoice();
    }
    if(choice == '1') {
        activeProfile = &activeUser->profiles[activeUser->profiles.size() - 1];
        return AppState::PROFILE_DASHBOARD;
    } else if(choice == '2') {
        return AppState::CREATE_PROFILE;
    }
}

AppState handleProfileDashboard(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, int &deckPage, Deck* &selectedDeck) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== PROFILE DASHBOARD: " << activeProfile->profileName << " ===\n\n";

    const int limitPerPage = 5;
    int decksVectorSize = activeProfile->decks.size();
    int runningSize = (decksVectorSize<= limitPerPage) ? decksVectorSize : ((deckPage + 1) * limitPerPage) - ((decksVectorSize/ limitPerPage) >= (deckPage + 1) ? 0 : limitPerPage - (decksVectorSize% limitPerPage));
    for(int i = (5 * deckPage); i < runningSize; i++) {
        int index = (i + 1) - (deckPage * limitPerPage);
        cout << "[" << index << "]: " << activeProfile->decks[i].deckName << "\n";
    }

    cout << "\n";

    int numberOfPages = (decksVectorSize / limitPerPage) + ( (decksVectorSize % limitPerPage) > 0 ? 1 : 0);
    if(numberOfPages > 1) {
        if(deckPage == 0) {
            cout << "[>] Next Page";
        } else if(deckPage > 0 && deckPage< (numberOfPages - 1)) {
            cout << "[<] Previous Page  |  Next Page [>]";
        } else if(deckPage== (numberOfPages - 1)) {
            cout << "[<] Previous Page";
        } 
        cout << "\n";
    }
    cout << "[8] Create Deck\n[9] Settings\n[0] Back\n";

    char choice = getChoice();
    if(choice == '0') {
        return AppState::PROFILES;
    } else if(choice == '9') {
        return AppState::SETTINGS;
    } else if(choice == '8') {
        return AppState::CREATE_DECK;
    } else if(choice == '<' && numberOfPages > 1 && ((deckPage > 0 && deckPage < (numberOfPages - 1)) || (deckPage == (numberOfPages - 1)))) {
        deckPage = deckPage - 1;
        return AppState::DECK_DASHBOARD;
    } else if(choice == '>' && numberOfPages > 1 && ((deckPage > 0 && deckPage < (numberOfPages - 1)) || (deckPage == 0))) {
        deckPage = deckPage + 1;
        return AppState::DECK_DASHBOARD;
    }

    if(choice != '1' && choice != '2' && choice != '3' && choice != '4' && choice != '5') {
        return AppState::DECK_DASHBOARD;
    }
    int choiceToInt = choice - '0';
    selectedDeck = &activeProfile->decks[(choiceToInt - 1) + (deckPage * limitPerPage)];
    return AppState::PROFILE_DASHBOARD;
}

/*
    main Function:
        This is the function that gets called as soon as the program starts.
*/
int main() {
    unordered_map<string, Account> allAccounts = loadFromFile();
    Account* activeUser = nullptr;
    Profile* activeProfile = nullptr;
    Deck* selectedDeck = nullptr;
    int profilePage = 0;
    int deckPage = 0;

    /*
        The code below connects the AppStates to their respective handle Functions
    */
    AppState currentState = AppState::MAIN_MENU;
    while(currentState != AppState::EXIT) {
        switch(currentState) {
            case AppState::MAIN_MENU:
                currentState = handleMainMenu();
                activeUser = nullptr;
                activeProfile = nullptr;
                break;
            case AppState::LOGIN:
                currentState = handleLogin(allAccounts, activeUser);
                break;
            case AppState::SIGNUP:
                currentState = handleSignup(allAccounts, activeUser);
                break;
            case AppState::PROFILES:
                currentState = handleProfiles(allAccounts, activeUser, profilePage, activeProfile);
                break;
            case AppState::CREATE_PROFILE:
                currentState = handleCreateProfile(allAccounts, activeUser, activeProfile);
                break;
            case AppState::PROFILE_DASHBOARD:
                currentState = handleProfileDashboard(allAccounts, activeUser, activeProfile, deckPage, selectedDeck);
                break;
        }
    }

    return 0;
}   