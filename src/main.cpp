#include "../include/data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <chrono>      // timestamps
#include <filesystem>  // backups
#include <sstream>     // string streams
#include <algorithm>   // sort

using namespace std;

int getCurrentDay() {
    static int day = 0;
    return day++;
}

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
    CREATE_DECK,
    DECK_DASHBOARD,
    BACKUPS,
    STATISTICS,

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
        Account newAcc(username, password);
        allAccounts[username] = newAcc;
        saveToFile(allAccounts);
        activeUser = &allAccounts[username];
        
        return AppState::PROFILES;
    }

    return AppState::SIGNUP;
}

AppState handleProfiles(unordered_map<string,Account> &allAccounts, Account* &activeUser, int &profilePage, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== PROFILES ===\n";
    cout << "= Page " << profilePage << " =\n";
    int limitPerPage = 5;
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
// AppState handleCreateProfile(unordered_map<string,Account> &allAccounts, Account* &activeUser, int* &pProfilePage, Profile* &activeProfile) {

// }

// FSRS functions [MJ]
double getNewDifficulty(double currentDiff, int rating) {
    // rating: 1=Again, 2=Hard, 3=Good, 4=Easy
    double next = currentDiff;
    if (rating == 1) next += 2;
    else if (rating == 2) next += 1;
    else if (rating == 4) next -= 2;
    if (next < 1) return 1;
    if (next > 10) return 10;
    return next;
}

double getNewStability(double currentStab, int rating) {
    if (rating == 1) return currentStab / 2;
    if (rating == 2) return currentStab * 1.5;
    if (rating == 3) return currentStab * 2.5;
    return currentStab * 4;
}

int getNewInterval(double stability, int rating) {
    if (rating == 1) return 1;
    int days = (int)stability;
    if (days < 1) days = 1;
    if (days > 365) days = 365;
    return days;
}

void updateCard(Card& card, int rating) {
    card.difficulty = getNewDifficulty(card.difficulty, rating);
    card.stability = getNewStability(card.stability, rating);
    card.nextReview = getCurrentDay() + getNewInterval(card.stability, rating);
    card.repetitions++;
}

void showStats(Profile* p) {
    cout << "\033[2J\033[1;1H";
    cout << "=== STATISTICS ===\n";
    cout << "Total studied: " << p->totalStudied << "\n";
    cout << "Correct: " << p->totalCorrect << "\n";
    cout << "Wrong: " << p->totalWrong << "\n";
    if (p->totalStudied > 0)
        cout << "Success: " << (p->totalCorrect * 100 / p->totalStudied) << "%\n";
    cout << "Current streak: " << p->streak << "\n";
    cout << "\n[0] Back\n";
}

void simpleBackup(unordered_map<string, Account>& accounts) {
    json j = accounts;
    ofstream file("backup.json");
    file << j.dump(4);
    cout << "Backup saved to backup.json\n";
}

void simpleRestore(unordered_map<string, Account>& accounts) {
    ifstream file("backup.json");
    if (!file) {
        cout << "No backup found.\n";
        return;
    }
    json j;
    file >> j;
    accounts = j.get<unordered_map<string, Account>>();
    saveToFile(accounts);
    cout << "Restored from backup.json\n";
}

AppState handleBackups(unordered_map<string, Account>& accounts) {
    cout << "\033[2J\033[1;1H";
    cout << "=== BACKUP ===\n";
    cout << "[1] Create Backup\n";
    cout << "[2] Restore Backup\n";
    cout << "[0] Back\n";
    char c = getChoice();
    if (c == '1') simpleBackup(accounts);
    else if (c == '2') simpleRestore(accounts);
    cout << "Press Enter...";
    cin.ignore(); cin.get();
    return AppState::BACKUPS;
}

AppState handleStatistics(Profile* p) {
    showStats(p);
    char c = getChoice();
    if (c == '0') return AppState::PROFILE_DASHBOARD;
    return AppState::STATISTICS;
}

/*
    main Function:
        This is the function that gets called as soon as the program starts.
*/
int main() {
    unordered_map<string, Account> allAccounts = loadFromFile();
    Account* activeUser = nullptr;
    Profile* activeProfile = nullptr;
    int profilePage = 0;

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
            case AppState::BACKUPS:  //TO BE FIXED
                currentState = handleBackups();
                break;
            case AppState::STATISTICS: //TO BE FIXED
                currentState = handleStatistics(activeProfile);
                break;
        }
    }

    return 0;
}   