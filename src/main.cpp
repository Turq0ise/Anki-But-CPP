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
    DECK_SETTINGS,
        CHANGE_DECK_NAME,
        DELETE_DECK,
    SHOW_SUBDECKS,
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

    return AppState::CREATE_PROFILE;
}

AppState handleProfileDashboard(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, int &deckPage, Deck* &activeDeck) {
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
    std::cout << "\033[2J\033[1;1H";
    cout << "=== ACCOUNT SETTINGS: " << activeUser->accountName << " ===\n\n";
    cout << "[1] Change Account Name\n[2] Change Password\n[3] Delete Account\n[0] Back\n";

    char choice = getChoice();
    if(choice == '1') return AppState::CHANGE_ACCOUNT_NAME;
    if(choice == '2') return AppState::CHANGE_ACCOUNT_PASSWORD;
    if(choice == '3') return AppState::DELETE_ACCOUNT;
    if(choice == '0') return AppState::BACK;

    return AppState::ACCOUNT_SETTINGS;
}

AppState handleChangeAccountName(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== CHANGE ACCOUNT NAME ===\n";
    string oldUsername = activeUser->accountName;
    cout << "= Current Account Name: " << oldUsername << "\n";
    cout << "Enter New Username: ";
    string newUsername, password;
    getline(cin >> ws, newUsername);
    while(allAccounts.find(newUsername) != allAccounts.end()) {
        std::cout << "\033[2J\033[1;1H";
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
    return AppState::ACCOUNT_SETTINGS;
}

AppState handleChangeAccountPassword(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== CHANGE ACCOUNT PASSWORD ===\n";
    cout << "= Current Account Name: " << activeUser->accountName << "\n\n";
    cout << "Enter password: ";
    string currentPassword, newPassword, confirmPassword;
    getline(cin >> ws, currentPassword);
    while(currentPassword != activeUser->password) {
        std::cout << "\033[2J\033[1;1H";
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
        std::cout << "\033[2J\033[1;1H";
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
    return AppState::LOGIN;
}

AppState handleDeleteAccount(unordered_map<string,Account> &allAccounts, Account* &activeUser) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== DELETE ACCOUNT ===\n";
    cout << "= Current Account: " << activeUser->accountName << "\n\n";
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
            std::cout << "\033[2J\033[1;1H";
            cout << "Account Deletion Successful\nEnter any key to go back...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return AppState::BACK;
        }
    } else if(confirm == 'n') return AppState::ACCOUNT_SETTINGS;

    return AppState::DELETE_ACCOUNT;
}

AppState handleSettings(vector<AppState> &breadcrumbs) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== SETTINGS ===\n\n";

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
    std::cout << "\033[2J\033[1;1H";
    cout << "=== PROFILE SETTINGS: " << activeProfile->profileName << " ===\n\n";
    cout << "[1] Change Profile Name\n[2] Delete Profile\n[0] Back\n";

    char choice = getChoice();
    if(choice == '1') return AppState::CHANGE_PROFILE_NAME;
    if(choice == '2') return AppState::DELETE_PROFILE;
    if(choice == '0') return AppState::BACK;

    return AppState::PROFILE_SETTINGS;
}

AppState handleChangeProfileName(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== CHANGE PROFILE NAME ===\n";
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
                std::cout << "\033[2J\033[1;1H";
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
    std::cout << "\033[2J\033[1;1H";
    cout << "=== DELETE PROFILE ===\n";
    cout << "= Current Profile: " << activeProfile->profileName << "\n\n";
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
            std::cout << "\033[2J\033[1;1H";
            cout << "Profile Deletion Successful\nEnter any key to go back...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return AppState::PROFILES;
        }
    } else if(confirm == 'n') return AppState::PROFILE_SETTINGS;

    return AppState::DELETE_PROFILE;
}

AppState handleDeckDashboard(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== DECK DASHBOARD: " << activeDeck->deckName << " ===\n\n";

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
        // return AppState::REVIEW_DECK;
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
    std::cout << "\033[2J\033[1;1H";
    cout << "=== SUBDECKS OF: " << activeDeck->deckName << " ===\n\n";

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
    activeDeck = &activeProfile->decks[(choiceToInt - 1) + (deckPage * limitPerPage)];
    return AppState::DECK_DASHBOARD;
}

AppState handleCreateDeck(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    if (activeUser == nullptr) return AppState::MAIN_MENU;

    string fullDeckInput;
    std::cout << "\033[2J\033[1;1H";
    cout << "=== CREATE DECK ===\n";
    cout << "Use '::' for subdecks (e.g., Medicine::Anatomy::Bones)\n\n";
    cout << "Enter Deck Name: ";
    getline(cin >> ws, fullDeckInput);

    vector<string> deckNames = splitString(fullDeckInput, "::");
    
    if (deckNames.empty() || deckNames[0].empty()) {
        cout << "\nError: Invalid deck name layout.\n";
        cin.get();
        return AppState::CREATE_DECK;
    }

    vector<Deck>* currentDeckVector = &(activeProfile->decks);

    for (size_t currentLevel = 0; currentLevel < deckNames.size(); ++currentLevel) {
        string targetName = deckNames[currentLevel];
        bool deckFound = false;
        size_t foundIndex = 0;

        for (size_t i = 0; i < currentDeckVector->size(); ++i) {
            if ((*currentDeckVector)[i].deckName == targetName) {
                deckFound = true;
                foundIndex = i;
                break;
            }
        }

        if (deckFound && currentLevel == deckNames.size() - 1) {
            cout << "\nError: A deck named '" << targetName << "' already exists at this level!\n";
            cout << "Creation aborted. Press Enter to return...";
            cin.get();
            return AppState::CREATE_DECK;
        }

        if (!deckFound) {
            Deck newLevelDeck(targetName, static_cast<int>(currentLevel));
            currentDeckVector->push_back(newLevelDeck);
            foundIndex = currentDeckVector->size() - 1;
        }

        currentDeckVector = &((*currentDeckVector)[foundIndex].subDecks);
    }

    saveToFile(allAccounts);

    cout << "\nDeck structure '" << fullDeckInput << "' processed successfully!\n";
    cout << "Press Enter to return...";
    cin.get();

    return AppState::BACK;
}

AppState handleDeckSettings(unordered_map<string,Account> &allAccounts, Account* &activeUser, Profile* &activeProfile, Deck* &activeDeck) {
    std::cout << "\033[2J\033[1;1H";
    cout << "=== PROFILE SETTINGS: " << activeProfile->profileName << " ===\n\n";
    cout << "[1] Manage Cards\n[0] Back\n";

    char choice = getChoice();
    if(choice == '1') return AppState::CARD_MANAGEMENT;
    if(choice == '0') return AppState::BACK;

    return AppState::DECK_SETTINGS;
}

// =========================================================================
// PRODUCTION CARD MANAGEMENT - TARGETING REAL LIVE STRUCTURE DATA POINTERS
// =========================================================================
AppState handleCardManagement(unordered_map<string, Account> &allAccounts, Deck* activeDeck) {
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
            // cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Press Enter to continue...";
            cin.get();
        }
    }
    return AppState::DECK_DASHBOARD; 
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
            case AppState::PROFILES:
                currentState = handleProfiles(allAccounts, activeUser, profilePage, activeProfile);
                break;
            case AppState::CREATE_PROFILE:
                currentState = handleCreateProfile(allAccounts, activeUser, activeProfile);
                break;
            case AppState::PROFILE_DASHBOARD:
                currentState = handleProfileDashboard(allAccounts, activeUser, activeProfile, deckPage, activeDeck);
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
            case AppState::CREATE_DECK:
                currentState = handleCreateDeck(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::DECK_DASHBOARD:
                currentState = handleDeckDashboard(allAccounts, activeUser, activeProfile, activeDeck);
                break;
            case AppState::SHOW_SUBDECKS:
                currentState = handleShowSubDecks(allAccounts, activeUser, activeProfile, deckPage, activeDeck);
            case AppState::CARD_MANAGEMENT:
                currentState = handleCardManagement(allAccounts, activeDeck);
                break;
        }
    }

    return 0;
}