#include "../include/data.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <functional>
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;

void showStatistics(const Profile& activeProfile) {
    cout << "\n====================================\n";
    cout << "      YOUR STUDY STATISTICS\n";
    cout << "====================================\n";
    cout << "Profile Name: " << activeProfile.profileName << "\n";
    cout << "Current Streak: " << activeProfile.streak << " days\n";
    cout << "Total Cards Studied: " << activeProfile.totalStudied << "\n";
    cout << "Total Correct: " << activeProfile.totalCorrect << "\n";
    cout << "Total Wrong: " << activeProfile.totalWrong << "\n";

    if (activeProfile.totalStudied > 0) {
        double accuracy = (static_cast<double>(activeProfile.totalCorrect) / activeProfile.totalStudied) * 100.0;
        cout << "Accuracy: " << fixed << setprecision(2) << accuracy << "%\n";
    } else {
        cout << "Accuracy: N/A (Study some cards first!)\n";
    }
    cout << "====================================\n\n";
}

void createBackup() {
    cout << "\nCreating backup...\n";
    
    // Open the original file to read
    ifstream sourceFile("data.json");
    if (!sourceFile.is_open()) {
        cout << "[ERROR] Could not find 'data.json' to backup. Have you saved any data yet?\n";
        return;
    }

    // Create a new file to write to
    ofstream backupFile("data_backup.json");
    if (!backupFile.is_open()) {
        cout << "[ERROR] Something went wrong creating the backup file.\n";
        return;
    }

    // Copy everything from one file to another
    backupFile << sourceFile.rdbuf();

    // Close files
    sourceFile.close();
    backupFile.close();
    
    cout << "[SUCCESS] Backup created successfully as 'data_backup.json'!\n";
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

vector<string> splitString(const string &str, const string &delimiter) {
    vector<string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    // Loop as long as the delimiter is found in the remaining string
    while (end != string::npos) {
        // Extract the substring before the delimiter
        tokens.push_back(str.substr(start, end - start));
        
        // Move the start position past the delimiter
        start = end + delimiter.length();
        
        // Look for the next occurrence of the delimiter
        end = str.find(delimiter, start);
    }

    // Push the very last remaining token (everything after the last delimiter)
    tokens.push_back(str.substr(start));

    return tokens;
}

vector<Card*> getCardsFromDeck(Deck &baseDeck) {
    vector<Card*> cards;
    
    for(size_t i = 0; i < baseDeck.cards.size(); ++i) {
        cards.push_back(&(baseDeck.cards[i]));
    }

    for(size_t i = 0; i < baseDeck.subDecks.size(); ++i) {
        vector<Card*> cardsFromSubDecks = getCardsFromDeck(baseDeck.subDecks[i]);
        cards.insert(cards.end(), cardsFromSubDecks.begin(), cardsFromSubDecks.end());
    }
    return cards;
}

void editCardWhileReview(unordered_map<string,Account> &allAccounts, Card* &selectedCard) {
    int choice = -1;
    while(choice != 0) {
        std::cout << "\033[2J\033[1;1H\n"; 
        cout << "===========================================\n";
        cout << "  EDIT CARD (" << selectedCard->front << ")\n";
        cout << "===========================================\n";
        cout << "[1] Change Front\n";
        cout << "[2] Change Back\n";
        cout << "[0] Back to Review\n";

        if(!(cin >> choice)) {
            cout << "Please enter a valid input.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        string input;
        if(choice == 1) {
            std::cout << "\033[2J\033[1;1H\n";
            cout << "--- CHANGE FRONT (" << selectedCard->front << ") ---\n\n";
            cout << "New Front: ";
            getline(cin >> ws, input); selectedCard->front = input;

            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        } else if(choice == 2) {
            std::cout << "\033[2J\033[1;1H\n";
            cout << "--- CHANGE BACK (" << selectedCard->back << ") ---\n\n";
            cout << "New Back: ";
            getline(cin >> ws, input); selectedCard->back = input;

            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
        saveToFile(allAccounts);
    }
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
        CHANGE_ACCOUNT_NAME,
        CHANGE_ACCOUNT_PASSWORD,
        DELETE_ACCOUNT,
    PROFILE_DASHBOARD,
    SETTINGS,
    PROFILE_SETTINGS,
        CHANGE_PROFILE_NAME,
        DELETE_PROFILE,
    CREATE_DECK,
    DECK_DASHBOARD,
    DECK_SETTINGS,
        CHANGE_DECK_NAME,
        DELETE_DECK,
    SHOW_SUBDECKS,
    CARD_MANAGEMENT,
    REVIEW_CARDS,
    
    BACK,
    EXIT
};

vector<AppState> breadcrumbs;
vector<Deck*> deckBreadcrumbs;

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
            std::cout << "\033[2J\033[1;1H\n";
*/

AppState handleMainMenu(unordered_map<string, Account> &allAccounts) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";
    cout << "=============== ANKI IN C++ ===============\n";
    cout << "===========================================\n\n";
    cout << "[1] Login\n";
    cout << "[2] Sign Up\n";
    cout << "[0] Exit\n";

    char choice = getChoice();
    if(choice == '1') {
        if(!filesystem::exists("data.json")) {
            cout << "No accounts found. Press any key to return...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
            return AppState::MAIN_MENU;
        }

        if(!filesystem::is_empty("data.json")) {
            if(allAccounts.size() == 0) {
                cout << "No accounts found. Press any key to return...";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                return AppState::MAIN_MENU;
            }
        }

        return AppState::LOGIN;
    }
    if(choice == '2') return AppState::SIGNUP;
    if(choice == '0') return AppState::EXIT;

    return AppState::MAIN_MENU;
}

AppState handleBack(vector<AppState> &breadcrumbs, Account* &activeUser, Deck* &activeDeck) {
    if(activeUser == nullptr) {
        breadcrumbs.clear();
        // breadcrumbs.push_back(AppState::MAIN_MENU);
        return AppState::MAIN_MENU;
    }

    if(breadcrumbs.rbegin()[0] == AppState::DECK_DASHBOARD || breadcrumbs.rbegin()[0] == AppState::SHOW_SUBDECKS) {
        if(breadcrumbs.rbegin()[1] == AppState::CREATE_DECK) {
            deckBreadcrumbs.clear();
            activeDeck = nullptr;
        } else {
            deckBreadcrumbs.pop_back();
            activeDeck = deckBreadcrumbs.rbegin()[0];
        }
        
    }

    if(breadcrumbs.rbegin()[1] == breadcrumbs.rbegin()[0]) breadcrumbs.pop_back();
    breadcrumbs.pop_back();

    bool invalid = true;
    while(invalid) {
        if( breadcrumbs.back() == AppState::LOGIN ||
            breadcrumbs.back() == AppState::SIGNUP ||
            breadcrumbs.back() == AppState::CHANGE_ACCOUNT_NAME ||
            breadcrumbs.back() == AppState::CHANGE_ACCOUNT_PASSWORD ||
            breadcrumbs.back() == AppState::DELETE_ACCOUNT ||
            breadcrumbs.back() == AppState::CREATE_PROFILE ||
            breadcrumbs.back() == AppState::CHANGE_PROFILE_NAME ||
            breadcrumbs.back() == AppState::DELETE_PROFILE ||
            breadcrumbs.back() == AppState::CREATE_DECK) {
                breadcrumbs.pop_back();
                invalid = true;
            } else invalid = false;
    }

    while(breadcrumbs.rbegin()[1] == breadcrumbs.rbegin()[0]) {
        breadcrumbs.pop_back();
    }

    return breadcrumbs.back();
}

AppState handleLogin(unordered_map<string, Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "                   LOGIN\n";
    cout << "===========================================\n\n";
    
    cout << "Enter Username: ";
    string username, password;
    getline(cin >> ws, username);
    cout << "Enter Password: ";
    getline(cin, password);
    if(allAccounts.find(username) == allAccounts.end()) {
        cout << "Account does not exist or Password is Incorrect, Please try again\n";
        cout << "Press Enter to try again...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return AppState::LOGIN;
    }
    if(allAccounts[username].password != password) {
        cout << "Account does not exist or Password is Incorrect, Please try again\n";
        cout << "Press Enter to try again...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return AppState::LOGIN;
    }

    activeUser = &allAccounts[username];
    return AppState::PROFILES;
}

AppState handleSignup(unordered_map<string, Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "                 SIGN UP\n";
    cout << "===========================================\n\n";

    cout << "Enter Username: ";
    string username, password;
    getline(cin >> ws, username);
    while(allAccounts.find(username) != allAccounts.end()) {
        std::cout << "\033[2J\033[1;1H\n";
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
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "                 PROFILES\n";
    cout << "===========================================\n\n";
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
        activeUser = nullptr;
        return AppState::BACK;
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
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "               CREATE PROFILE\n";
    cout << "===========================================\n\n";

    string profileName;
    cout << "Enter Profile Name: ";
    getline(cin >> ws, profileName);

    bool profileExists = true;
    while (profileExists) {
        profileExists = false;

        for(const Profile &profile : activeUser->profiles) {
            if(profileName == profile.profileName) {
                profileExists = true;
                std::cout << "\033[2J\033[1;1H\n";
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

    return AppState::CREATE_PROFILE;
}

AppState handleProfileDashboard(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, int &deckPage, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  PROFILE DASHBOARD (" << activeProfile->profileName << ")\n";
    cout << "===========================================\n\n";

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
    cout << "[6] Show Statistics\n[7] Create Deck\n[8] Settings\n[9] Back\n[0] Sign Out\n";

    char choice = getChoice();
    if(choice == '0') {
        activeUser = nullptr;
        return AppState::BACK;
    } else if(choice == '9') { 
        return AppState::BACK;
    } else if(choice == '8') {
        return AppState::SETTINGS;
    } else if(choice == '7') {
        return AppState::CREATE_DECK;
    } else if(choice == '6') {
        std::cout << "\033[2J\033[1;1H\n";
        showStatistics(*activeProfile);
        cout << "Press Enter to go back...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::PROFILE_DASHBOARD;
    } else if(choice == '<' && numberOfPages > 1 && ((deckPage > 0 && deckPage < (numberOfPages - 1)) || (deckPage == (numberOfPages - 1)))) {
        deckPage = deckPage - 1;
        return AppState::PROFILE_DASHBOARD;
    } else if(choice == '>' && numberOfPages > 1 && ((deckPage > 0 && deckPage < (numberOfPages - 1)) || (deckPage == 0))) {
        deckPage = deckPage + 1;
        return AppState::PROFILE_DASHBOARD;
    }

    if(choice != '1' && choice != '2' && choice != '3' && choice != '4' && choice != '5') {
        return AppState::PROFILE_DASHBOARD;
    }
    int choiceToInt = choice - '0';
    activeDeck = &activeProfile->decks[(choiceToInt - 1) + (deckPage * limitPerPage)];
    return AppState::DECK_DASHBOARD;
}

AppState handleAccountSettings(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  ACCOUNT SETTINGS (" << activeUser->accountName << ")\n";
    cout << "===========================================\n\n";

    cout << "[1] Change Account Name\n[2] Change Password\n[3] Delete Account\n[4] Create Backup\n[0] Back\n";

    char choice = getChoice();
    if(choice == '1') return AppState::CHANGE_ACCOUNT_NAME;
    if(choice == '2') return AppState::CHANGE_ACCOUNT_PASSWORD;
    if(choice == '3') return AppState::DELETE_ACCOUNT;
    if(choice == '4') {
        createBackup();
        cout << "Press Enter to go back...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::ACCOUNT_SETTINGS;
    }
    if(choice == '0') return AppState::BACK;

    return AppState::ACCOUNT_SETTINGS;
}

AppState handleChangeAccountName(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H\n";
    string oldUsername = activeUser->accountName;
    cout << "===========================================\n";    
    cout << "  CHANGE ACCOUNT NAME (" << oldUsername << ")\n";
    cout << "===========================================\n";

    cout << "Enter New Username: ";
    string newUsername, password;
    getline(cin >> ws, newUsername);
    while(allAccounts.find(newUsername) != allAccounts.end()) {
        std::cout << "\033[2J\033[1;1H\n";
        cout << "=== CHANGE ACCOUNT NAME ===\n";
        cout << "Account Name: " << newUsername << " Already Exists, Please Try Again!\n";
        cout << "Enter New Username: ";
        getline(cin >> ws, newUsername);
    }
    cout << "Enter Password: ";
    getline(cin >> ws, password);
    while(activeUser->password != password) {
        cout << "Incorrect Password, Please Try Again!\n";
        cout << "Enter Password: ";
        getline(cin >> ws, password);
    }

    activeUser->accountName = newUsername;
    allAccounts[newUsername] = allAccounts[oldUsername];
    allAccounts.erase(oldUsername);
    saveToFile(allAccounts);
    activeUser = &allAccounts[newUsername];

    cout << "Change Account Name Successful, Press any key to continue: ";
    cin.get();
    return AppState::BACK;
}

AppState handleChangeAccountPassword(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  CHANGE ACCOUNT PASSWORD (" << activeUser->accountName << ")\n";
    cout << "===========================================\n";

    cout << "Enter password: ";
    string currentPassword, newPassword, confirmPassword;
    getline(cin >> ws, currentPassword);
    while(currentPassword != activeUser->password) {
        std::cout << "\033[2J\033[1;1H\n";
        cout << "=== CHANGE ACCOUNT PASSWORD ===\n";
        cout << "= Current Account Name: " << activeUser->accountName << "\n\n";
        cout << "Incorrect Password, Please Try Again!\n";
        cout << "Enter password: ";
        getline(cin >> ws, currentPassword);
    }
    cout << "\n";
    cout << "Enter New Password: ";
    getline(cin >> ws, newPassword);
    cout << "Confirm New Password: ";
    getline(cin >> ws, confirmPassword);
    while(newPassword != confirmPassword) {
        std::cout << "\033[2J\033[1;1H\n";
        cout << "Passwords Did Not Match, Please Try Again!\n";
        cout << "\n";
        cout << "Enter New Password: ";
        getline(cin >> ws, newPassword);
        cout << "Confirm New Password: ";
        getline(cin >> ws, confirmPassword);
    }
    allAccounts[activeUser->accountName].password = newPassword;
    saveToFile(allAccounts);
    activeUser = nullptr;
    cout << "Password Change Successful, Please Log In Again\nEnter any key to proceed...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  
    return AppState::BACK;
}

AppState handleDeleteAccount(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  DELETE ACCOUNT (" << activeUser->accountName << ")\n";
    cout << "===========================================\n";

    char confirm;
    cout << "Are you sure you want to delete your account? [Y/n]: ";
    cin >> confirm;
    if(confirm == 'Y') {
        string confirmString;
        cout << "\nPlease type your username enclosed in square brackets to confirm account deletion: \n";
        getline(cin >> ws, confirmString);
        if(confirmString == ("[" + activeUser->accountName + "]")) {
            allAccounts.erase(activeUser->accountName);
            saveToFile(allAccounts);
            activeUser = nullptr;
            std::cout << "\033[2J\033[1;1H\n";
            cout << "Account Deletion Successful\nEnter any key to go back...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return AppState::BACK;
        }
    } else if(confirm == 'n') return AppState::ACCOUNT_SETTINGS;

    return AppState::DELETE_ACCOUNT;
}

AppState handleSettings(vector<AppState> &breadcrumbs) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "                SETTINGS\n";
    cout << "===========================================\n\n";

    if(breadcrumbs.rbegin()[1] == breadcrumbs.rbegin()[0]) breadcrumbs.pop_back();

    if(breadcrumbs.rbegin()[1] == AppState::PROFILE_DASHBOARD) {
        cout << "[1] Profile Settings\n[2] Account Settings\n[0] Back\n";
        char choice;
        choice = getChoice();
        if(choice == '1') return AppState::PROFILE_SETTINGS;
        else if(choice == '2') return AppState::ACCOUNT_SETTINGS;
        else if(choice == '0') return AppState::BACK;
    } else if(breadcrumbs.rbegin()[1] == AppState::DECK_DASHBOARD || breadcrumbs.rbegin()[1] == AppState::SHOW_SUBDECKS) {
        cout << "[1] Deck Settings\n[2] Profile Settings\n[3] Account Settings\n[0] Back\n";
        char choice;
        choice = getChoice();
        if(choice == '1') return AppState::DECK_SETTINGS;
        else if(choice == '2') return AppState::PROFILE_SETTINGS;
        else if(choice == '3') return AppState::ACCOUNT_SETTINGS;
        else if(choice == '0') return AppState::BACK;
    }

    return AppState::SETTINGS;
}

AppState handleProfileSettings(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  PROFILE SETTINGS (" << activeProfile->profileName << ")\n";
    cout << "===========================================\n\n";
    
    cout << "[1] Change Profile Name\n[2] Delete Profile\n[0] Back\n";

    char choice = getChoice();
    if(choice == '1') return AppState::CHANGE_PROFILE_NAME;
    if(choice == '2') return AppState::DELETE_PROFILE;
    if(choice == '0') return AppState::BACK;

    return AppState::PROFILE_SETTINGS;
}

AppState handleChangeProfileName(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  CHANGE PROFILE NAME (" << activeProfile->profileName << ")\n";
    cout << "===========================================\n";

    string oldProfileName = activeProfile->profileName;
    cout << "= Current Profile Name: " << oldProfileName << "\n";
    cout << "Enter New Profile Name: ";
    string newProfileName, password;
    bool profileNameExists = true;
    getline(cin >> ws, newProfileName);
    while(profileNameExists == true) {
        profileNameExists = false;
        for(int i = 0; i < activeUser->profiles.size(); i++) {
            if(newProfileName == activeUser->profiles[i].profileName) {
                std::cout << "\033[2J\033[1;1H\n";
                cout << "=== CHANGE PROFILE NAME ===\n";
                cout << "Profile Name: " << oldProfileName << " Already Exists, Please Try Again!\n";
                cout << "Enter New Profile Name: ";
                getline(cin >> ws, newProfileName);
                profileNameExists = true;
                break;
            }
        }
    }
    cout << "Enter Password: ";
    getline(cin >> ws, password);
    while(activeUser->password != password) {
        cout << "Incorrect Password, Please Try Again!\n";
        cout << "Enter Password: ";
        getline(cin >> ws, password);
    }

    activeProfile->profileName = newProfileName;
    allAccounts[activeUser->accountName].profiles = activeUser->profiles;
    saveToFile(allAccounts);
    return AppState::BACK;
}

AppState handleDeleteProfile(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  DELETE PROFILE (" << activeProfile->profileName << ")\n";
    cout << "===========================================\n\n";

    char confirm;
    cout << "Are you sure you want to delete this profile? [Y/n]: ";
    cin >> confirm;
    if(confirm == 'Y') {
        string confirmString;
        cout << "\nPlease type your profile name enclosed in square brackets to confirm account deletion: \n";
        getline(cin >> ws, confirmString);
        if(confirmString == ("[" + activeProfile->profileName + "]")) {
            int index = 0;
            while(index < activeUser->profiles.size()) {
                if(activeUser->profiles[index].profileName == activeProfile->profileName) break;
                index++;
            }
            activeUser->profiles.erase(activeUser->profiles.begin() + index);
            allAccounts[activeUser->accountName].profiles = activeUser->profiles;
            saveToFile(allAccounts);
            activeProfile = nullptr;
            std::cout << "\033[2J\033[1;1H\n";
            cout << "Profile Deletion Successful\nEnter any key to go back...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return AppState::PROFILES;
        }
    } else if(confirm == 'n') return AppState::PROFILE_SETTINGS;

    return AppState::DELETE_PROFILE;
}

AppState handleDeckDashboard(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  DECK DASHBOARD (" << activeDeck->deckName<< ")\n";
    cout << "===========================================\n\n";

    cout << "Insert Deck Stats Here\n\n";

    cout << "[1] Review\n[2] Card Management\n[3] Add Deck\n";
    if(activeDeck->subDecks.size() > 0) cout << "[4] Show Subdecks\n";
    cout << "[8] Settings\n[9] Back\n[0] Sign Out\n";

    char choice = getChoice();
    if(choice == '0') {
        activeUser = nullptr;
        return AppState::BACK;
    } else if(choice == '9') {
        activeDeck = nullptr; 
        return AppState::BACK;
    } else if(choice == '8') {
        return AppState::SETTINGS;
    } else if(choice == '1') {
        return AppState::REVIEW_CARDS;
    } else if(choice == '2') {
        return AppState::CARD_MANAGEMENT;
    } else if(choice == '3') {
        return AppState::CREATE_DECK;
    } else if((choice == '4') && (activeDeck->subDecks.size() > 0)) {
        return AppState::SHOW_SUBDECKS;
    }

    return AppState::DECK_DASHBOARD;
}

AppState handleShowSubDecks(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, int &deckPage, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  SUBDECKS (" << activeDeck->deckName << ")\n";
    cout << "===========================================\n\n";

    const int limitPerPage = 5;
    int decksVectorSize = activeDeck->subDecks.size();
    int runningSize = (decksVectorSize<= limitPerPage) ? decksVectorSize : ((deckPage + 1) * limitPerPage) - ((decksVectorSize/ limitPerPage) >= (deckPage + 1) ? 0 : limitPerPage - (decksVectorSize% limitPerPage));
    for(int i = (5 * deckPage); i < runningSize; i++) {
        int index = (i + 1) - (deckPage * limitPerPage);
        cout << "[" << index << "]: " << activeDeck->subDecks[i].deckName << "\n";
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
    cout << "[7] Create Deck\n[8] Settings\n[9] Back\n[0] Sign Out\n";

    char choice = getChoice();

    if(choice == '0') {
        activeUser = nullptr;
        return AppState::BACK;
    } else if(choice == '9') {
        return AppState::BACK;
    } else if(choice == '8') {
        return AppState::SETTINGS;
    } else if(choice == '7') {
        return AppState::CREATE_DECK;
    } else if(choice == '<' && numberOfPages > 1 && ((deckPage > 0 && deckPage < (numberOfPages - 1)) || (deckPage == (numberOfPages - 1)))) {
        deckPage = deckPage - 1;
        return AppState::SHOW_SUBDECKS;
    } else if(choice == '>' && numberOfPages > 1 && ((deckPage > 0 && deckPage < (numberOfPages - 1)) || (deckPage == 0))) {
        deckPage = deckPage + 1;
        return AppState::SHOW_SUBDECKS;
    }

    if(choice != '1' && choice != '2' && choice != '3' && choice != '4' && choice != '5') {
        return AppState::SHOW_SUBDECKS;
    }
    int choiceToInt = choice - '0';
    activeDeck = &activeDeck->subDecks[(choiceToInt - 1) + (deckPage * limitPerPage)];
    return AppState::DECK_DASHBOARD;
}

AppState handleCreateDeck(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    string fullDeckInput;
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "               CREATE DECK\n";
    cout << "===========================================\n";

    vector<Deck*> addToDeckBreadcrumbs;
    Deck* selectedDeck;
    vector<Deck>* currentDeckVector = nullptr;
    int startingLevel = 0;
    if (activeDeck != nullptr) {
        std::cout << "Creating a subdeck inside active deck: [ " << activeDeck->deckName << " ]\n";
        std::cout << "Use '::' for deeper nesting (e.g., Object-Oriented Programming::Inheritance)\n\n";

        currentDeckVector = &(activeDeck->subDecks);
        startingLevel = activeDeck->level + 1;
    } else {
        std::cout << "Creating a root-level deck.\n";
        std::cout << "Use '::' for subdecks (e.g., CPE103L::Object-Oriented Programming::Inheritance)\n\n";

        currentDeckVector = &(activeProfile->decks);
    }

    cout << "Enter Deck Name: ";
    getline(cin >> ws, fullDeckInput);

    vector<string> deckNames = splitString(fullDeckInput, "::");
    
    if (deckNames.empty() || deckNames[0].empty()) {
        cout << "\nError: Invalid deck name layout.\n";
        cin.get();
        return AppState::CREATE_DECK;
    }

    for (size_t i = 0; i < deckNames.size(); ++i) {
        string targetName = deckNames[i];
        bool deckFound = false;
        size_t foundIndex = 0;

        int calculatedLevel = startingLevel + static_cast<int>(i);

        for (size_t j = 0; j < currentDeckVector->size(); ++j) {
            if ((*currentDeckVector)[j].deckName == targetName) {
                deckFound = true;
                foundIndex = j;
                break;
            }
        }

        if (deckFound && i == deckNames.size() - 1) {
            cout << "\nError: A deck named '" << targetName << "' already exists at this level!\n";
            cout << "Creation aborted. Press Enter to return...";
            cin.get();
            return AppState::CREATE_DECK;
        }

        if (!deckFound) {
            Deck newLevelDeck(targetName, calculatedLevel);
            currentDeckVector->push_back(newLevelDeck);
            foundIndex = currentDeckVector->size() - 1;
        }

        selectedDeck = &((*currentDeckVector)[foundIndex]);
        addToDeckBreadcrumbs.push_back(selectedDeck);
        
        currentDeckVector = &((*currentDeckVector)[foundIndex].subDecks);
    }

    saveToFile(allAccounts);

    cout << "\nDeck structure '" << fullDeckInput << "' processed successfully!\n";
    cout << "[1] Open " << fullDeckInput << "\n";
    cout << "[2] Create Another Deck\n";
    cout << "[9] Back\n";
    cout << "[0] Sign Out\n";
    char choice = getChoice();

    if(choice == '0') {
        activeUser = nullptr;
        return AppState::BACK;
    } else if(choice == '9') {
        return AppState::BACK;
    } else if(choice == '2') {
        return AppState::CREATE_DECK;
    } else if(choice == '1') {
        for(size_t i = 0; i < addToDeckBreadcrumbs.size() - 1; ++i) deckBreadcrumbs.push_back(addToDeckBreadcrumbs[i]);
        activeDeck = selectedDeck;
        return AppState::DECK_DASHBOARD;
    }

    return AppState::BACK;
}

AppState handleDeckSettings(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  DECK SETTINGS (" << activeDeck->deckName << ")\n";
    cout << "===========================================\n\n";

    cout << "[1] Change Deck Name\n[2] Manage Cards\n[3] Delete Deck\n[9] Back\n[0] Sign Out\n";

    char choice = getChoice();
    if(choice == '1') return AppState::CHANGE_DECK_NAME;
    if(choice == '2') return AppState::CARD_MANAGEMENT;
    if(choice == '3') return AppState::DELETE_DECK;
    if(choice == '9') return AppState::BACK;
    if(choice == '0') {
        activeUser = nullptr;
        return AppState::BACK;
    }

    return AppState::DECK_SETTINGS;
}

AppState handleChangeDeckName(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H\n";
    string oldDeckName = activeDeck->deckName;
    cout << "===========================================\n";    
    cout << "  CHANGE DECK NAME (" << oldDeckName << ")\n";
    cout << "===========================================\n\n";

    int level = activeDeck->level;
    vector<int> runningIndex;
    for(size_t i = 0; i < activeProfile->decks.size(); ++i) {
        if(activeProfile->decks[i].deckName == deckBreadcrumbs[0]->deckName) {
            runningIndex.push_back(i);
            break;
        }
    }
    Deck* runningDeck = &activeProfile->decks[runningIndex[0]];
    vector<Deck> runningVectorDeck;

    if(level == 0) {
        runningVectorDeck = activeProfile->decks;
    } else if(level >= 1) {
        bool early = false;
        for(size_t i = 1; i < deckBreadcrumbs.size() - 1; i++) {
            for(size_t j = 0; runningDeck->subDecks.size(); j++) {
                if(deckBreadcrumbs[i]->deckName == runningDeck->subDecks[j].deckName) {
                    runningIndex.push_back(static_cast<int>(j));
                    runningDeck = &runningDeck->subDecks[runningIndex[i]];
                    early = true;
                    break;
                }
            }
            if(early) break;
        }

        runningVectorDeck = runningDeck->subDecks;
    }

    cout << "Enter New Deck Name: ";
    string newDeckName;
    bool deckNameExists = true;
    getline(cin >> ws, newDeckName);

    while(deckNameExists) {
        deckNameExists = false;
        for(size_t i = 0; i < runningVectorDeck.size(); ++i) {
            if(newDeckName == runningVectorDeck[i].deckName) {
                std::cout << "\033[2J\033[1;1H\n";
                cout << "===========================================\n";    
                cout << "  CHANGE DECK NAME (" << oldDeckName << ")\n";
                cout << "===========================================\n";
                cout << "Previous attempt already existed\n\n";
                cout << "Enter New Deck Name: ";
                getline(cin >> ws, newDeckName);
                deckNameExists = true;
                break;
            }
        }
    }

    activeDeck->deckName = newDeckName;
    saveToFile(allAccounts);

    cout << "Change Deck Name Successful, please press enter to continue";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return AppState::BACK;
}

AppState handleDeleteDeck(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    int level = activeDeck->level;
    vector<int> runningIndex;
    for(size_t i = 0; i < activeProfile->decks.size(); ++i) {
        if(activeProfile->decks[i].deckName == deckBreadcrumbs[0]->deckName) {
            runningIndex.push_back(i);
            break;
        }
    }
    Deck* runningDeck = &activeProfile->decks[runningIndex[0]];

    std::cout << "\033[2J\033[1;1H\n";
    cout << "===========================================\n";    
    cout << "  DELETE DECK (" << activeDeck->deckName<< ")\n";
    cout << "===========================================\n\n";

    cout << "Are you sure you want to delete this Deck and all its Sub Decks and Cards? [Y/n]: ";
    
    char confirm;
    cin >> confirm;
    if(confirm == 'Y') {
        string confirmString;
        cout << "\nPlease type your deck name enclosed in square brackets to confirm account deletion: \n";
        getline(cin >> ws, confirmString);
        if(confirmString == ("[" + activeDeck->deckName + "]")) {
            // while(runningIndex[0] < runningDecks.size()) {
            //     if(deckBreadcrumbs[0]->deckName == runningDecks[runningIndex[0]].deckName) break;
            //     runningIndex[0]++;
            // }

            if(level == 0) {
                activeProfile->decks.erase(activeProfile->decks.begin() + runningIndex[0]);
            } else if(level >= 1) {
                bool early = false;
                for(size_t i = 1; i < deckBreadcrumbs.size() - 1; i++) {
                    for(size_t j = 0; runningDeck->subDecks.size(); j++) {
                        if(deckBreadcrumbs[i]->deckName == runningDeck->subDecks[j].deckName) {
                            runningIndex.push_back(static_cast<int>(j));
                            runningDeck = &runningDeck->subDecks[runningIndex[i]];
                            early = true;
                            break;
                        }
                    }
                    if(early) break;
                }

                for(size_t i = 0; i < runningDeck->subDecks.size(); ++i) {
                    if(runningDeck->subDecks[i].deckName == activeDeck->deckName) {
                        runningIndex.push_back(static_cast<int>(i));
                        break;
                    }
                }

                runningDeck->subDecks.erase(runningDeck->subDecks.begin() + runningIndex.back());
            }

            saveToFile(allAccounts);
            activeDeck = nullptr;
            cout << "Deleted Successfully\n";
            cin.get();

            return AppState::BACK;
        }
    }

    return AppState::BACK;
}

// =========================================================================
// PRODUCTION CARD MANAGEMENT - TARGETING REAL LIVE STRUCTURE DATA POINTERS
// =========================================================================
AppState handleCardManagement(unordered_map<string, Account> &allAccounts, Deck* activeDeck) {
    if (!activeDeck) {
        cout << "Error: No active deck selected! Returning to Dashboard...\n";
        cout << "Press Enter to continue...";
        // cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return AppState::BACK;
    }

    int choice = -1;
    while (choice != 0) {
        std::cout << "\033[2J\033[1;1H\n"; 
        cout << "===========================================\n";
        cout << "  MANAGING CARDS PANEL (" << activeDeck->deckName << ")\n";
        cout << "===========================================\n";
        cout << "[1] Create (Add Card & Tag Type Layout)\n";
        cout << "[2] Read (View Existing Cards)\n";
        cout << "[3] Update (Edit Card Content)\n";
        cout << "[4] Delete (Remove Card From Deck Collection)\n";
        cout << "[5] Manage Tags\n";
        cout << "[6] Toggle Flag\n";
        cout << "[7] Search by Tag\n";
        cout << "[8] Show Flagged Cards\n";
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
            std::cout << "\033[2J\033[1;1H\n";
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

            // === UPDATED SECTION ===
            if (typeChoice == 1) {
                activeDeck->cards.push_back(Card(front, back, 1)); 
                activeDeck->cards.push_back(Card(back, front, 1)); 
                cout << "\nSuccess: 2 Cards generated (Forward & Reversed)!";
            } else {
                activeDeck->cards.push_back(Card(front, back, typeChoice));
                cout << "\nSuccess: Card added and synchronized directly!";
            }
            // =======================

            saveToFile(allAccounts); // Instantly commits updates to data.json

            cout << " Press Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
        // --- READ ---
        else if (choice == 2) {
            std::cout << "\033[2J\033[1;1H\n";
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
                
                    cout << "    Flagged: " << (activeDeck->cards[i].flagged ? "Yes" : "No") << "\n";
                    cout << "    Tags: ";
                    for (const auto& tag : activeDeck->cards[i].tags) {
                        cout << tag << " ";
                }
                    cout << "\n";
                    cout << "===========================================\n";
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
            // cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Press Enter to continue...";
            cin.get();
        }
        else if(choice == 5) {
            int index;
            cout << "\nCard Index: ";
            cin >> index;

            if(index < 1 || index > activeDeck->cards.size())
                continue;
            index--;

            cout << "\n[1] Add Tag\n";
            cout << "[2] Remove Tag\n";

            int tagChoice;
            cin >> tagChoice;

            if(tagChoice == 1) {
                string tag;
                cout << "Tag Name: ";
                getline(cin >> ws, tag);
                activeDeck->cards[index].tags.push_back(tag);

            }
            else if(tagChoice == 2) {
                string tag;
                cout << "Tag to Remove: ";
                getline(cin >> ws, tag);

                auto &tags = activeDeck->cards[index].tags;
                tags.erase(remove(tags.begin(), tags.end(), tag), tags.end());
            }
            saveToFile(allAccounts);
        }
        else if(choice == 6) {
            int index;
            cout << "Card Index: ";
            cin >> index;
            if(index < 1 || index > activeDeck->cards.size())
                continue;
            index--;

            activeDeck->cards[index].flagged = !activeDeck->cards[index].flagged;
            saveToFile(allAccounts);

            cout << "Flag status changed.\n";
        }
        else if(choice == 7) {
            string searchTag;
            cout << "Tag: ";
            getline(cin >> ws, searchTag);

            cout << "\n=== RESULTS ===\n";
            for(const auto &card : activeDeck->cards) {
                if(find(card.tags.begin(), card.tags.end(), searchTag) != card.tags.end()) {
                    cout << "Front: " << card.front << "\n";
                    cout << "Back: " << card.back  << "\n\n";
                }
            }
            cin.get();
        }
        else if(choice == 8) {
            cout << "\n=== FLAGGED CARDS ===\n";
            for(const auto &card : activeDeck->cards) {
                if(card.flagged) {
                    cout << "Front: " << card.front << "\n";
                    cout << "Back: " << card.back  << "\n\n";
                }
            }
            cin.ignore();
            cin.get();
        }
    }
    return AppState::BACK; 
}

AppState handleReviewCards(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck, int &reviewIndex) {
    string deckNameString;
    for(size_t i = 0; i < deckBreadcrumbs.size(); ++i) {
        if(i == 0) {
            deckNameString = deckBreadcrumbs[i]->deckName;
        } else {
            deckNameString = deckNameString + "::" + deckBreadcrumbs[i]->deckName;
        }
    }

    vector<Card*> cards = getCardsFromDeck(*activeDeck);

    bool isReviewing = true;
    while(isReviewing && (reviewIndex < cards.size())) {
        if(reviewIndex != 0) cout << "\n...........................................";

        std::cout << "\033[2J\033[1;1H\n";
        cout << "=== REVIEWING DECK: "<< deckNameString << " ===\n\n";
        cout << cards[reviewIndex]->front << "\n\n";

        int cardType = cards[reviewIndex]->type;
        if(cardType == 0 || cardType == 1) {
            cout << "[1] Show Answer\n[8] Edit Card (To follow mamaya para makaproceed yung iba)\n[9] Back/Stop Review\n[0] Sign Out\n";

            char choice = getChoice();
            if(choice == '0') {
                activeUser = nullptr;
                return AppState::BACK;
            } else if(choice == '9') { 
                return AppState::BACK;
            } else if(choice == '8') {
                editCardWhileReview(allAccounts, cards[reviewIndex]);
                return AppState::REVIEW_CARDS;
            } else if(choice == '1') {
                cout << "\n" << cards[reviewIndex]->back << "\n";
            } else {
                return AppState::REVIEW_CARDS;
            }
        } else if(cardType == 2) {
            cout << "To answer the flashcard, type your answer enclosed in square brackets\n\n";
            cout << "[8] Edit Card (To follow mamaya para makaproceed yung iba)\n[9] Back/Stop Review\n[0] Sign Out\n";

            cout << ": "; string input; getline(cin >> ws, input);
            if(input == "0") {
                activeUser = nullptr;
                return AppState::BACK;
            } else if(input == "9") { 
                return AppState::BACK;
            } else if(input == "8") {
                // return AppState::EDIT_CARD;
            } else if((input.front() == '[') && (input.back() == ']')) {
                if(input == "[" + cards[reviewIndex]->back + "]") {
                    cout << "\ntomoh\n";
                } else {
                    cout << "\nntnt\n";
                }
            }
        }

        // rating would go here, input buffer na lang in the meantime
        /*
        cout << "Press any key to proceed to the next card...";
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cin.get();
        
        */
        cout << "Did you get this card correct? [Y/n]: ";
        char grade;
        cin >> grade;

        // Update Stat
        activeProfile->totalStudied++;
        if (grade == 'Y' || grade == 'y') {
            activeProfile->totalCorrect++;
        } else {
                activeProfile->totalWrong++;
        }

        // Save to data.json
        saveToFile(allAccounts);

        cout << "Press Enter to proceed to the next card...";

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();

        reviewIndex++;
    }

    
    for(size_t i = 0; i < cards.size(); ++i) {
        cout << cards[i]->front << "\n";
    }

    cout << "Press Enter to return...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();

    return AppState::BACK;
}

/*
    main Function:
        This is the function that gets called as soon as the program starts.
*/
int main() {
    unordered_map<string, Account> allAccounts = loadFromFile();
    Account* activeUser = nullptr;
    Profile* activeProfile = nullptr;
    Deck* activeDeck = nullptr;
    int profilePage = 0;
    int deckPage = 0;
    int reviewIndex = 0;

    /*
        The code below connects the AppStates to their respective handle Functions
    */
    AppState currentState = AppState::MAIN_MENU;
    while(currentState != AppState::EXIT) {
        if(activeUser == nullptr) {
            profilePage = 0;
            deckPage = 0;
        }
        if(activeProfile == nullptr) {
            deckPage = 0;
        }

        if(currentState != AppState::BACK) {
            if(((breadcrumbs.size() != 0) && (breadcrumbs.back() != currentState)) || (breadcrumbs.size() == 0)) {
                breadcrumbs.push_back(currentState);
            }
        } 
            
        if(activeDeck != nullptr) {
            if(((deckBreadcrumbs.size() != 0) && (deckBreadcrumbs.back() != activeDeck)) || (deckBreadcrumbs.size() == 0)) {
                deckBreadcrumbs.push_back(activeDeck);
            }
        }

        cout << "\n-------------------------------------------";
        switch(currentState) {
            case AppState::MAIN_MENU:
                currentState = handleMainMenu(allAccounts);
                activeUser = nullptr;
                activeProfile = nullptr;
                break;
            case AppState::BACK:
                currentState = handleBack(breadcrumbs, activeUser, activeDeck);
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
                currentState = handleProfileDashboard(allAccounts, activeUser, activeProfile, deckPage, activeDeck);
                break;
            case AppState::ACCOUNT_SETTINGS:
                currentState = handleAccountSettings(allAccounts, activeUser);
                break;
            case AppState::CHANGE_ACCOUNT_NAME:
                currentState = handleChangeAccountName(allAccounts, activeUser);
                break;
            case AppState::CHANGE_ACCOUNT_PASSWORD:
                currentState = handleChangeAccountPassword(allAccounts, activeUser);
                break;
            case AppState::DELETE_ACCOUNT:
                currentState = handleDeleteAccount(allAccounts, activeUser);
                break;
            case AppState::SETTINGS:
                currentState = handleSettings(breadcrumbs);
                break;
            case AppState::PROFILE_SETTINGS:
                currentState = handleProfileSettings(allAccounts, activeUser, activeProfile);
                break;
            case AppState::CHANGE_PROFILE_NAME:
                currentState = handleChangeProfileName(allAccounts, activeUser, activeProfile);
                break;
            case AppState::DELETE_PROFILE:
                currentState = handleDeleteProfile(allAccounts, activeUser, activeProfile);
                break;
            case AppState::CREATE_DECK:
                currentState = handleCreateDeck(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::DECK_DASHBOARD:
                currentState = handleDeckDashboard(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::DECK_SETTINGS:
                currentState = handleDeckSettings(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::CHANGE_DECK_NAME:
                currentState = handleChangeDeckName(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::DELETE_DECK:
                currentState = handleDeleteDeck(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::SHOW_SUBDECKS:
                currentState = handleShowSubDecks(allAccounts, activeUser, activeProfile, deckPage, activeDeck);
                break;
            case AppState::CARD_MANAGEMENT:
                currentState = handleCardManagement(allAccounts, activeDeck);
                break;
            case AppState::REVIEW_CARDS:
                currentState = handleReviewCards(allAccounts, activeUser, activeProfile, activeDeck, reviewIndex);
                break;
        }
    }

    return 0;
}   