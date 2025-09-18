#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cstdlib>
#include <cstdio>
#include <queue>
#include <unordered_set>

using namespace std;

const string BOOKS_FILE = "books.txt";
const string USERS_FILE = "users.txt";
const string ISSUES_FILE = "issues.txt";
const int SUGGESTION_COUNT = 3;

string getCurrentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << 1900 + ltm->tm_year << "-" 
       << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
       << setw(2) << setfill('0') << ltm->tm_mday;
    return ss.str();
}

string getDueDate(int days = 14) {
    time_t now = time(0) + days * 24 * 3600;
    tm* ltm = localtime(&now);
    stringstream ss;
    ss << 1900 + ltm->tm_year << "-"
       << setw(2) << setfill('0') << 1 + ltm->tm_mon << "-"
       << setw(2) << setfill('0') << ltm->tm_mday;
    return ss.str();
}

string hashPassword(const string& password) {
    hash<string> hasher;
    return to_string(hasher(password));
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

class Book {
public:
    string ISBN;
    string title;
    string author;
    string genre;
    int totalQuantity;
    int availableQuantity;

    Book(string isbn = "", string t = "", string a = "", string g = "", 
        int tq = 0, int aq = 0)
        : ISBN(isbn), title(t), author(a), genre(g), 
          totalQuantity(tq), availableQuantity(aq) {}
};

struct IssueRecord {
    string ISBN;
    string username;
    string issueDate;
    string dueDate;
    bool returned;
    string bookTitle;
};

class User {
public:
    string username;
    string passwordHash;
    string role;
    string name;
    string email;

    User(string u = "", string p = "", string r = "", 
        string n = "", string e = "")
        : username(u), passwordHash(p), role(r), name(n), email(e) {}
};

class UserManager {
private:
    vector<User> users;

    void loadUsers() {
        ifstream file(USERS_FILE);
        string line;
        while (getline(file, line)) {
            vector<string> fields;
            stringstream ss(line);
            string field;
            
            while (getline(ss, field, '|')) {
                fields.push_back(field);
            }
            
            if (fields.size() >= 5) {
                users.emplace_back(fields[0], fields[1], fields[2], fields[3], fields[4]);
            }
        }
        file.close();
    }

    void saveUsers() {
        ofstream file(USERS_FILE);
        for (const auto& user : users) {
            file << user.username << "|" << user.passwordHash << "|"
                 << user.role << "|" << user.name << "|" << user.email << "\n";
        }
        file.close();
    }

public:
    UserManager() {
        loadUsers();
        if (users.empty()) {
            users.emplace_back("admin", hashPassword("admin123"), "manager", "Admin", "admin@library.com");
            saveUsers();
        }
    }

    bool authenticate(string username, string password, string& role) {
        string hashed = hashPassword(password);
        for (auto& user : users) {
            if (user.username == username && user.passwordHash == hashed) {
                role = user.role;
                return true;
            }
        }
        return false;
    }

    void createUser(string username, string password, string role, string name, string email) {
        for (auto& user : users) {
            if (user.username == username) {
                cout << "User already exists!\n";
                return;
            }
        }
        users.emplace_back(username, hashPassword(password), role, name, email);
        saveUsers();
    }

    bool resetPassword(const string& username, const string& newPassword) {
        for (auto& user : users) {
            if (user.username == username) {
                user.passwordHash = hashPassword(newPassword);
                saveUsers();
                return true;
            }
        }
        return false;
    }

    User* getUser(const string& username) {
        for (auto& user : users) {
            if (user.username == username) return &user;
        }
        return nullptr;
    }

    void listUsers() {
        cout << "\n===== USER LIST =====\n";
        cout << left << setw(15) << "Username" << setw(20) << "Name" 
             << setw(25) << "Email" << setw(10) << "Role" << endl;
        cout << "-----------------------------------------------------------\n";
        for (const auto& user : users) {
            cout << left << setw(15) << user.username << setw(20) << user.name
                 << setw(25) << user.email << setw(10) << user.role << endl;
        }
    }
};

class Library {
private:
vector<Book> books;
unordered_map<string, int> isbnIndex;
vector<IssueRecord> issues;
unordered_map<string, vector<Book*>> genreMap;
unordered_map<string, vector<Book*>> authorMap;

void merge(int left, int mid, int right) {
    vector<Book> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right) {
        if (books[i].ISBN < books[j].ISBN) temp[k++] = books[i++];
        else temp[k++] = books[j++];
    }

    while (i <= mid) temp[k++] = books[i++];
    while (j <= right) temp[k++] = books[j++];

    for (int x = 0; x < k; x++) {
        books[left + x] = temp[x];
    }
}

void mergeSort(int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(left, mid);
        mergeSort(mid + 1, right);
        merge(left, mid, right);
    }
}

    void loadBooks() {
        ifstream file(BOOKS_FILE);
        string line;
        while (getline(file, line)) {
            vector<string> fields;
            stringstream ss(line);
            string field;
            
            while (getline(ss, field, '|')) {
                fields.push_back(field);
            }
            
            if (fields.size() >= 6) {
                try {
                    books.emplace_back(
                        fields[0], fields[1], fields[2], fields[3],
                        stoi(fields[4]), stoi(fields[5])
                    );
                    Book& newBook = books.back();
                    genreMap[newBook.genre].push_back(&newBook);
                    authorMap[newBook.author].push_back(&newBook);
                } catch (...) {
                    cerr << "Error parsing book: " << line << endl;
                }
            }
        }
        file.close();
        if (!books.empty()) {
            mergeSort(0, books.size() - 1);
        }
        updateIndex();
    }

    void updateIndex() {
        isbnIndex.clear();
        for (size_t i = 0; i < books.size(); i++) {
            isbnIndex[books[i].ISBN] = i;
        }
    }

    void saveBooks() {
        ofstream file(BOOKS_FILE);
        for (auto& book : books) {
            file << book.ISBN << "|" << book.title << "|" 
                 << book.author << "|" << book.genre << "|"
                 << book.totalQuantity << "|"
                 << book.availableQuantity << "\n";
        }
        file.close();
    }

    void loadIssues() {
        ifstream file(ISSUES_FILE);
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            IssueRecord record;
            getline(ss, record.ISBN, '|');
            getline(ss, record.username, '|');
            getline(ss, record.issueDate, '|');
            getline(ss, record.dueDate, '|');
            string returned;
            getline(ss, returned, '|');
            getline(ss, record.bookTitle, '|');
            record.returned = (returned == "1");
            issues.push_back(record);
        }
        file.close();
    }

    void saveIssues() {
        ofstream file(ISSUES_FILE);
        for (const auto& record : issues) {
            file << record.ISBN << "|" << record.username << "|"
                 << record.issueDate << "|" << record.dueDate << "|"
                 << (record.returned ? "1" : "0") << "|"
                 << record.bookTitle << "\n";
        }
        file.close();
    }
    void addBookInternal(const Book& book) {
        // Check if book already exists
        if (isbnIndex.count(book.ISBN)) {
            int idx = isbnIndex[book.ISBN];
            books[idx].totalQuantity += book.totalQuantity;
            books[idx].availableQuantity += book.availableQuantity;
        } else {
            books.push_back(book);
            Book& newBook = books.back();
            genreMap[newBook.genre].push_back(&newBook);
            authorMap[newBook.author].push_back(&newBook);
        }
        updateIndex();
    }
public:
    Library() {
        loadBooks();
        loadIssues();
    }

    vector<Book> suggestSimilarBooks(const string& isbn) {
        vector<Book> suggestions;
        priority_queue<pair<int, Book*>> pq;

        if (!isbnIndex.count(isbn)) return suggestions;

        Book& target = books[isbnIndex[isbn]];
        
        for (Book* book : genreMap[target.genre]) {
            if (book->ISBN != isbn) {
                int score = 2;
                if (book->author == target.author) score += 1;
                pq.push({score, book});
            }
        }

        for (Book* book : authorMap[target.author]) {
            if (book->ISBN != isbn) {
                pq.push({1, book});
            }
        }

        unordered_set<string> added;
        while (!pq.empty() && suggestions.size() < SUGGESTION_COUNT) {
            auto entry = pq.top();
            pq.pop();
            if (added.find(entry.second->ISBN) == added.end()) {
                suggestions.push_back(*entry.second);
                added.insert(entry.second->ISBN);
            }
        }

        return suggestions;
    }

    void displayBookList(bool showAvailability = true) {
        cout << "\n===== BOOK LIST =====\n";
        cout << left << setw(15) << "ISBN" << setw(30) << "Title" 
             << setw(20) << "Author" << setw(15) << "Genre" 
             << setw(10) << "Total" << setw(15) << "Available" << endl;
        cout << "-------------------------------------------------------------------------\n";
        
        for (const auto& book : books) {
            cout << left << setw(15) << book.ISBN << setw(30) << book.title 
                 << setw(20) << book.author << setw(15) << book.genre 
                 << setw(10) << book.totalQuantity;
            
            if (showAvailability) {
                cout << setw(15) << (book.availableQuantity > 0 ? 
                    to_string(book.availableQuantity) + " available" : "Unavailable");
            }
            cout << endl;
        }
    }
    void addBook(const Book& book) {
        addBookInternal(book);
        mergeSort(0, books.size() - 1);
        updateIndex();
        saveBooks();
        cout << "Book added successfully!\n";
    }

    bool issueBook(const string& isbn, const string& username) {
        if (isbnIndex.count(isbn)) {
            Book& book = books[isbnIndex[isbn]];
            if (book.availableQuantity > 0) {
                book.availableQuantity--;
                
                IssueRecord record;
                record.ISBN = isbn;
                record.username = username;
                record.issueDate = getCurrentDate();
                record.dueDate = getDueDate();
                record.returned = false;
                record.bookTitle = book.title;
                
                issues.push_back(record);
                saveBooks();
                saveIssues();
                return true;
            }
        }
        return false;
    }

    bool returnBook(const string& isbn, const string& username) {
        for (auto& record : issues) {
            if (record.ISBN == isbn && record.username == username && !record.returned) {
                record.returned = true;
                if (isbnIndex.count(isbn)) {
                    books[isbnIndex[isbn]].availableQuantity++;
                }
                saveBooks();
                saveIssues();
                return true;
            }
        }
        return false;
    }

    void displayUserBorrowedBooks(const string& username) {
        cout << "\n===== BORROWED BOOKS =====\n";
        cout << left << setw(15) << "ISBN" << setw(30) << "Title" 
             << setw(15) << "Issue Date" << setw(15) << "Due Date" << endl;
        cout << "-------------------------------------------------------------------------\n";
        
        bool found = false;
        for (const auto& record : issues) {
            if (record.username == username && !record.returned) {
                cout << left << setw(15) << record.ISBN << setw(30) << record.bookTitle
                     << setw(15) << record.issueDate << setw(15) << record.dueDate << endl;
                found = true;
            }
        }
        if (!found) cout << "No borrowed books found.\n";
    }

    void displayAllIssues() {
        cout << "\n===== ALL ISSUES =====\n";
        cout << left << setw(15) << "ISBN" << setw(30) << "Title" 
             << setw(15) << "Student ID" << setw(15) << "Issue Date" 
             << setw(15) << "Due Date" << setw(10) << "Status" << endl;
        cout << "-------------------------------------------------------------------------\n";
        
        for (const auto& record : issues) {
            string status = record.returned ? "Returned" : "Issued";
            cout << left << setw(15) << record.ISBN << setw(30) << record.bookTitle
                 << setw(15) << record.username << setw(15) << record.issueDate
                 << setw(15) << record.dueDate << setw(10) << status << endl;
        }
    }

    void displayOverdueBooks() {
        string today = getCurrentDate();
        cout << "\n===== OVERDUE BOOKS =====\n";
        cout << left << setw(15) << "ISBN" << setw(30) << "Title" 
             << setw(15) << "Student ID" << setw(15) << "Due Date" << endl;
        cout << "-------------------------------------------------------------------------\n";
        
        bool found = false;
        for (const auto& record : issues) {
            if (!record.returned && record.dueDate < today) {
                cout << left << setw(15) << record.ISBN << setw(30) << record.bookTitle
                     << setw(15) << record.username << setw(15) << record.dueDate << endl;
                found = true;
            }
        }
        if (!found) cout << "No overdue books found.\n";
    }

    void searchBooks(const string& query) {
        vector<int> results;
        string searchTerm = query;
        transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(), ::tolower);
        
        for (size_t i = 0; i < books.size(); i++) {
            string title = books[i].title;
            string author = books[i].author;
            string isbn = books[i].ISBN;
            
            transform(title.begin(), title.end(), title.begin(), ::tolower);
            transform(author.begin(), author.end(), author.begin(), ::tolower);
            
            if (title.find(searchTerm) != string::npos || 
                author.find(searchTerm) != string::npos ||
                isbn.find(searchTerm) != string::npos) {
                results.push_back(i);
            }
        }
        displayBookList(results);
    }

private:
    void displayBookList(const vector<int>& indices) {
        cout << "\n===== SEARCH RESULTS =====\n";
        cout << left << setw(15) << "ISBN" << setw(30) << "Title" 
             << setw(20) << "Author" << setw(15) << "Genre" 
             << setw(10) << "Total" << setw(15) << "Available" << endl;
        cout << "-------------------------------------------------------------------------\n";
        
        for (int idx : indices) {
            const Book& book = books[idx];
            cout << left << setw(15) << book.ISBN << setw(30) << book.title 
                 << setw(20) << book.author << setw(15) << book.genre 
                 << setw(10) << book.totalQuantity 
                 << setw(15) << (book.availableQuantity > 0 ? 
                    to_string(book.availableQuantity) + " available" : "Unavailable") << endl;
        }
    }
};

bool handleForgotPassword(UserManager& um) {
    string username, email;
    cout << "\n===== PASSWORD RESET =====\n";
    cout << "Enter username: ";
    getline(cin, username);

    User* user = um.getUser(username);
    if (!user) {
        cout << "User not found!\n";
        return false;
    }

    cout << "Enter registered email: ";
    getline(cin, email);

    if (email != user->email) {
        cout << "Email doesn't match!\n";
        return false;
    }

    srand(time(0));
    int otp = 1000 + rand() % 9000;
    string command = "python3 send_email.py \"" + user->email + "\" " + to_string(otp);
    
    if (system(command.c_str()) != 0) {
        cout << "Failed to send OTP!\n";
        return false;
    }

    int attempts = 3;
    while (attempts-- > 0) {
        string input;
        cout << "Enter OTP (" << attempts + 1 << " attempts left): ";
        getline(cin, input);

        if (input == to_string(otp)) {
            string newPass;
            cout << "Enter new password: ";
            getline(cin, newPass);
            
            if (um.resetPassword(username, newPass)) {
                cout << "Password updated successfully!\n";
                return true;
            }
            return false;
        }
        cout << "Invalid OTP!\n";
    }
    cout << "Too many failed attempts!\n";
    return false;
}

void studentMenu(Library& lib, const string& username) {
    int choice;
    do {
        cout << "\n===== STUDENT MENU =====\n"
             << "1. Search Books\n2. View All Books\n3. Issue Book\n"
             << "4. Return Book\n5. My Borrowed Books\n6. Get Recommendations\n7. Logout\n"
             << "Choice: ";
        cin >> choice;
        clearInputBuffer();

        string query, isbn;
        vector<Book> results;
        switch (choice) {
            case 1:
                cout << "Enter search term: ";
                getline(cin, query);
                lib.searchBooks(query);
                break;
            case 2:
                lib.displayBookList();
                break;
            case 3:
                lib.displayBookList();
                cout << "Enter ISBN to issue: ";
                getline(cin, isbn);
                if (lib.issueBook(isbn, username)) {
                    cout << "Book issued successfully!\n";
                    auto suggestions = lib.suggestSimilarBooks(isbn);
                    if (!suggestions.empty()) {
                        cout << "\nYou might also like:\n";
                        for (const auto& book : suggestions) {
                            cout << " - " << book.title << " by " << book.author 
                                 << " (" << book.ISBN << ")\n";
                        }
                    }
                } else {
                    cout << "Failed to issue book.\n";
                }
                break;
            case 4:
                cout << "Enter ISBN to return: ";
                getline(cin, isbn);
                if (lib.returnBook(isbn, username)) {
                    cout << "Book returned successfully!\n";
                } else {
                    cout << "Failed to return book.\n";
                }
                break;
            case 5:
                lib.displayUserBorrowedBooks(username);
                break;
            case 6: {
                cout << "Enter ISBN of a book you like: ";
                getline(cin, isbn);
                auto suggestions = lib.suggestSimilarBooks(isbn);
                if (!suggestions.empty()) {
                    cout << "\nRecommended books:\n";
                    lib.displayBookList(false);
                } else {
                    cout << "No recommendations found.\n";
                }
                break;
            }
            case 7:
                return;
            default:
                cout << "Invalid choice!\n";
        }
        cout << "Press Enter to continue...";
        cin.get();
    } while (true);
}

void managerMenu(Library& lib, UserManager& um) {
    int choice;
    do {
        cout << "\n===== MANAGER MENU =====\n"
             << "1. Add Book\n2. Search Books\n3. View All Books\n"
             << "4. View Issues\n5. Overdue Books\n6. Create User\n"
             << "7. List Users\n8. Logout\nChoice: ";
        cin >> choice;
        clearInputBuffer();

        string isbn, title, author, genre, username, password, name, email;
        int quantity;
        switch (choice) {
            case 1:
                cout << "Enter ISBN: ";
                getline(cin, isbn);
                cout << "Enter Title: ";
                getline(cin, title);
                cout << "Enter Author: ";
                getline(cin, author);
                cout << "Enter Genre: ";
                getline(cin, genre);
                cout << "Enter Total Quantity: ";
                cin >> quantity;
                clearInputBuffer();
                lib.addBook(Book(isbn, title, author, genre, quantity, quantity));
                break;
            case 2:
                cout << "Enter search term: ";
                getline(cin, title);
                lib.searchBooks(title);
                break;
            case 3:
                lib.displayBookList();
                break;
            case 4:
                lib.displayAllIssues();
                break;
            case 5:
                lib.displayOverdueBooks();
                break;
            case 6:
                cout << "Enter username (roll number): ";
                getline(cin, username);
                cout << "Enter password: ";
                getline(cin, password);
                cout << "Enter student name: ";
                getline(cin, name);
                cout << "Enter student email: ";
                getline(cin, email);
                um.createUser(username, password, "student", name, email);
                break;
            case 7:
                um.listUsers();
                break;
            case 8:
                return;
            default:
                cout << "Invalid choice!\n";
        }
        cout << "Press Enter to continue...";
        cin.get();
    } while (true);
}

int main() {
    UserManager um;
    Library lib;
    
    while (true) {
        cout << "\033[2J\033[1;1H";
        cout << "===== LIBRARY MANAGEMENT SYSTEM =====\n\n";
        
        int choice;
        cout << "1. Login\n2. Forgot Password\n3. Exit\nChoice: ";
        cin >> choice;
        clearInputBuffer();

        if (choice == 1) {
            string username, password, role;
            int attempts = 0;
            bool authenticated = false;
            
            while (attempts < 3 && !authenticated) {
                cout << "Username: ";
                getline(cin, username);
                cout << "Password: ";
                getline(cin, password);
                
                if (um.authenticate(username, password, role)) {
                    authenticated = true;
                    if (role == "manager") {
                        managerMenu(lib, um);
                    } else {
                        studentMenu(lib, username);
                    }
                } else {
                    cout << "Invalid credentials!\n";
                    attempts++;
                }
            }
        } else if (choice == 2) {
            handleForgotPassword(um);
        } else if (choice == 3) {
            break;
        }
    }
    return 0;
}