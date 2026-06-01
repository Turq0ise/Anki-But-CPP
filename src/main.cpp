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
    CARD_MANAGEMENT,
    DECK_SETTINGS,
    
    BACK,
    EXIT
};

vector<AppState> breadcrumbs;

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

AppState handleBack(vector<AppState> &breadcrumbs, Account* &activeUser) {
    if(activeUser == nullptr) {
        breadcrumbs.clear();
        breadcrumbs.push_back(AppState::MAIN_MENU);
        return breadcrumbs.back();
    }

    if(breadcrumbs.rbegin()[1] == breadcrumbs.rbegin()[0]) breadcrumbs.pop_back();
    breadcrumbs.pop_back();
    return breadcrumbs.back();
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
    cout << "[7] Create Deck\n[8] Settings\n[9] Back\n[0] Sign Out";

    
    char choice = getChoice();
    if(choice == '0') {
        activeUser = nullptr;
        return AppState::BACK;
    } else if(choice == '9') { 
        return AppState::BACK;
    } else if(choice == '8') {
        // previousState = AppState::PROFILE_DASHBOARD;
        return AppState::SETTINGS;
    } else if(choice == '7') {
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
    } else if(breadcrumbs.rbegin()[1] == AppState::DECK_DASHBOARD) {
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
        for(int i = 0; i < activeUser->profiles.size(); i++) {
            if(newProfileName == activeUser->profiles[i].profileName) {
                std::cout << "\033[2J\033[1;1H";
                cout << "=== CHANGE PROFILE NAME ===\n";
                cout << "Profile Name: " << oldProfileName << " Already Exists, Please Try Again!\n";
                cout << "Enter New Profile Name: ";
                getline(cin >> ws, newProfileName);
                break;
            }
        }
        profileNameExists = false;
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
        if(activeUser == nullptr) {
            profilePage = 0;
            deckPage = 0;
        }
        if(activeProfile == nullptr) {
            deckPage = 0;
        }

        if( currentState != AppState::BACK &&
            currentState != AppState::SIGNUP &&
            currentState != AppState::CHANGE_ACCOUNT_NAME &&
            currentState != AppState::CHANGE_ACCOUNT_PASSWORD &&
            currentState != AppState::DELETE_ACCOUNT &&
            currentState != AppState::CREATE_PROFILE &&
            currentState != AppState::CHANGE_PROFILE_NAME &&
            currentState != AppState::DELETE_PROFILE &&
            currentState != AppState::CREATE_DECK)
            breadcrumbs.push_back(currentState);

        switch(currentState) {
            case AppState::MAIN_MENU:
                currentState = handleMainMenu();
                activeUser = nullptr;
                activeProfile = nullptr;
                break;
            case AppState::BACK:
                currentState = handleBack(breadcrumbs, activeUser);
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
            case AppState::PROFILES:
                currentState = handleProfiles(allAccounts, activeUser, profilePage, activeProfile);
                break;
            case AppState::CREATE_PROFILE:
                currentState = handleCreateProfile(allAccounts, activeUser, activeProfile);
                break;
            case AppState::PROFILE_DASHBOARD:
                currentState = handleProfileDashboard(allAccounts, activeUser, activeProfile, deckPage, selectedDeck);
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
        }
    }

    return 0;
}   