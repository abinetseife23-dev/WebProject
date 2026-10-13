#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <limits>

using namespace std;

// ======== Custom Exceptions ========
class InvalidInputException : public runtime_error {
public:
    InvalidInputException(const string& msg) : runtime_error(msg) {}
};

class AccessDeniedException : public runtime_error {
public:
    AccessDeniedException(const string& msg) : runtime_error(msg) {}
};

class BookNotFoundException : public runtime_error {
public:
    BookNotFoundException(const string& msg) : runtime_error(msg) {}
};

class StudentNotFoundException : public runtime_error {
public:
    StudentNotFoundException(const string& msg) : runtime_error(msg) {}
};

class BorrowLimitException : public runtime_error {
public:
    BorrowLimitException(const string& msg) : runtime_error(msg) {}
};

// ======== Node for Linked List (Books) ========
struct Book {
    int id;
    string title;
    string category;
    int totalCopies;
    int availableCopies;
    Book* next;

    Book(int i, string t, string c, int copies)
        : id(i), title(t), category(c), totalCopies(copies), availableCopies(copies), next(nullptr) {}
};

// ======== Node for Queue (Waiting Students) ========
struct QueueNode {
    int studentId;
    int bookId;
    QueueNode* next;

    QueueNode(int sid, int bid) : studentId(sid), bookId(bid), next(nullptr) {}
};

// ======== Node for Stack (Returned Books) ========
struct StackNode {
    int bookId;
    string title;
    StackNode* next;

    StackNode(int bid, string t) : bookId(bid), title(t), next(nullptr) {}
};

// ======== Student Structure ========
struct Student {
    int id;
    string name;
    string gender;
    string role;
    int borrowedCount;
};

// ======== Helper: Read a validated integer from cin ========
int readInt(const string& prompt) {
    int value;
    cout << prompt;
    if (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        throw InvalidInputException("Invalid input: expected a whole number.");
    }
    return value;
}

// ======== Helper: Read a validated positive integer ========
int readPositiveInt(const string& prompt) {
    int value = readInt(prompt);
    if (value <= 0) {
        throw InvalidInputException("Invalid input: value must be a positive number (greater than 0).");
    }
    return value;
}

// ======== Helper: Read a non-empty string ========
string readNonEmptyString(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    if (value.empty()) {
        throw InvalidInputException("Invalid input: text cannot be empty.");
    }
    return value;
}

// ======== Library Class ========
class Library {
private:
    string name;
    Book* bookList;
    QueueNode* queueFront;
    QueueNode* queueRear;
    StackNode* returnStack;

    void processWaitingQueue(int bookId, vector<Student>& students) {
        if (!queueFront) return;
        if (queueFront->bookId == bookId) {
            int studentId = queueFront->studentId;
            for (auto& s : students) {
                if (s.id == studentId) {
                    if (s.borrowedCount < 3) {
                        s.borrowedCount++;
                        Book* book = searchBook(bookId);
                        if (book) {
                            book->availableCopies--;
                            cout << "Waiting student ID " << studentId << " got the book: " << book->title << endl;
                        }
                        dequeue();
                    } else {
                        cout << "Waiting student ID " << studentId << " reached limit. Removed from queue.\n";
                        dequeue();
                    }
                    break;
                }
            }
        }
    }

public:
    Library(string libName) : name(libName), bookList(nullptr), queueFront(nullptr), queueRear(nullptr), returnStack(nullptr) {}

    string getName() { return name; }

    void addBook(int id, string title, string category, int copies, bool silent = false) {
        if (id <= 0)
            throw InvalidInputException("Book ID must be a positive number.");
        if (title.empty())
            throw InvalidInputException("Book title cannot be empty.");
        if (category.empty())
            throw InvalidInputException("Book category cannot be empty.");
        if (copies <= 0)
            throw InvalidInputException("Number of copies must be greater than 0.");

        Book* newBook = new Book(id, title, category, copies);
        if (!bookList) {
            bookList = newBook;
        } else {
            Book* temp = bookList;
            while (temp->next) temp = temp->next;
            temp->next = newBook;
        }
        if (!silent) {
            cout << "Book added successfully to " << name << "!\n";
        }
    }

    Book* searchBook(int id = -1, string title = "") {
        Book* temp = bookList;
        while (temp) {
            if ((id != -1 && temp->id == id) ||
                (!title.empty() && temp->title.find(title) != string::npos)) {
                return temp;
            }
            temp = temp->next;
        }
        return nullptr;
    }

    bool borrowBook(int studentId, int bookId, vector<Student>& students) {
        if (studentId <= 0)
            throw InvalidInputException("Student ID must be a positive number.");
        if (bookId <= 0)
            throw InvalidInputException("Book ID must be a positive number.");

        Student* borrower = nullptr;
        for (auto& s : students) {
            if (s.id == studentId) {
                borrower = &s;
                break;
            }
        }
        if (!borrower)
            throw StudentNotFoundException("Student ID " + to_string(studentId) + " not found.");

        if (name == "Female Library" && borrower->gender == "Male")
            throw AccessDeniedException("Access denied: Male students cannot use the Female Library.");

        Book* book = searchBook(bookId);
        if (!book)
            throw BookNotFoundException("Book ID " + to_string(bookId) + " not found in " + name + ".");

        if (book->availableCopies <= 0) {
            cout << "Book unavailable. Added to waiting queue.\n";
            enqueue(studentId, bookId);
            return false;
        }

        int limit = (borrower->role == "Teacher") ? 100 : 3;
        if (borrower->borrowedCount >= limit)
            throw BorrowLimitException("Borrow limit reached (" + to_string(limit) + " books).");

        book->availableCopies--;
        borrower->borrowedCount++;
        cout << "Book borrowed successfully. Due in 7 days.\n";
        return true;
    }

    void returnBook(int bookId, vector<Student>& students) {
        if (bookId <= 0)
            throw InvalidInputException("Book ID must be a positive number.");

        Book* book = searchBook(bookId);
        if (!book)
            throw BookNotFoundException("Book ID " + to_string(bookId) + " not found in " + name + ".");

        book->availableCopies++;
        StackNode* newNode = new StackNode(bookId, book->title);
        newNode->next = returnStack;
        returnStack = newNode;
        cout << "Book returned. Checking waiting list...\n";
        processWaitingQueue(bookId, students);
    }

    void enqueue(int studentId, int bookId) {
        QueueNode* newNode = new QueueNode(studentId, bookId);
        if (!queueRear) {
            queueFront = queueRear = newNode;
        } else {
            queueRear->next = newNode;
            queueRear = newNode;
        }
        cout << "Student " << studentId << " added to waiting queue for book " << bookId << endl;
    }

    void dequeue() {
        if (!queueFront) return;
        QueueNode* temp = queueFront;
        queueFront = queueFront->next;
        if (!queueFront) queueRear = nullptr;
        delete temp;
    }

    void viewQueue() {
        if (!queueFront) {
            cout << "No students waiting in " << name << "\n";
            return;
        }
        cout << "Waiting queue for " << name << "\n";
        QueueNode* temp = queueFront;
        while (temp) {
            cout << "Student ID: " << temp->studentId << " wants Book ID: " << temp->bookId << endl;
            temp = temp->next;
        }
    }

    void shelfReturnedBook() {
        if (!returnStack) {
            cout << "No returned books to shelve.\n";
            return;
        }
        StackNode* temp = returnStack;
        cout << "Shelving book: " << temp->title << " (ID: " << temp->bookId << ")\n";
        returnStack = returnStack->next;
        delete temp;
    }

    void displayBooks() {
        cout << "Books in " << name << " Library:\n";
        Book* temp = bookList;
        if (!temp) {
            cout << " None.\n";
            return;
        }
        while (temp) {
            cout << " ID: " << temp->id << " | " << temp->title << " | Category: " << temp->category
                 << " | Available: " << temp->availableCopies << "/" << temp->totalCopies << endl;
            temp = temp->next;
        }
    }
};

// ====================== Login ======================
bool login(string& role) {
    unordered_map<string, string> credentials = {
        {"admin", "libadmin"},
        {"librarian", "lib123"},
        {"student", "stud2025"}
    };
    try {
        string username, password;
        cout << "Username: ";
        if (!(cin >> username) || username.empty())
            throw InvalidInputException("Username cannot be empty.");
        cout << "Password: ";
        if (!(cin >> password) || password.empty())
            throw InvalidInputException("Password cannot be empty.");

        if (credentials.find(username) != credentials.end() && credentials[username] == password) {
            if (username == "admin") role = "Administrator";
            else if (username == "librarian") role = "Librarian";
            else role = "Student";
            cout << "Login successful! Role: " << role << "\n\n";
            return true;
        }
        throw InvalidInputException("Invalid username or password.");
    } catch (const InvalidInputException& e) {
        cout << "[Login Error] " << e.what() << " Exiting.\n";
        return false;
    }
}

// ====================== Add default books (silently) ======================
void addDefaultBooks(Library& gebisa, Library& law, Library& female) {
    gebisa.addBook(101, "Introduction to Algorithms", "Computer Science", 3, true);
    gebisa.addBook(102, "Clean Code", "Programming", 2, true);
    gebisa.addBook(103, "The C++ Programming Language", "Programming", 4, true);
    gebisa.addBook(104, "Artificial Intelligence: A Modern Approach", "AI", 2, true);

    law.addBook(201, "Constitutional Law of Ethiopia", "Law", 5, true);
    law.addBook(202, "International Human Rights Law", "Law", 3, true);
    law.addBook(203, "Criminal Procedure Code", "Law", 4, true);
    law.addBook(204, "Contract Law: Principles", "Law", 2, true);

    female.addBook(301, "Becoming", "Biography", 3, true);
    female.addBook(302, "Women in Science", "History", 2, true);
}

// ====================== Library selector helper ======================
Library* selectLibrary(int choice, Library& gebisa, Library& law, Library& female) {
    if (choice == 1) return &gebisa;
    if (choice == 2) return &law;
    if (choice == 3) return &female;
    throw InvalidInputException("Invalid library choice. Please enter 1, 2, or 3.");
}

// ============ Main ============
int main() {
    cout << "========================================\n";
    cout << "HARAMAYA UNIVERSITY LIBRARY MANAGEMENT SYSTEM\n";
    cout << "========================================\n";

    string userRole;
    if (!login(userRole)) return 1;

    Library gebisa("Gebisa & Ejeta Library");
    Library law("Law Library");
    Library female("Female Library");
    addDefaultBooks(gebisa, law, female);

    vector<Student> students = {
        {1001, "Abel Kebede",      "Male",   "Student", 0},
        {1002, "Abebeche Tadesse", "Female", "Student", 0},
        {1003, "Dr. Mohammed",     "Male",   "Teacher", 0},
        {1004, "Selam Girma",      "Female", "Student", 0}
    };

    Library* currentLib = nullptr;

    while (true) {
        try {
            int choice = readInt(
                "\n1. Add Book\n2. Search Book\n3. Borrow Book\n4. Return Book\n"
                "5. View Waiting Queue\n6. Shelf Returned Books\n7. Display All Books\n8. Exit\nChoice: "
            );

            if (choice == 8) break;

            if (choice < 1 || choice > 8)
                throw InvalidInputException("Invalid menu choice. Please enter a number between 1 and 8.");

            // Select library for options 2â€“6
            if (choice >= 2 && choice <= 6) {
                int libChoice = readInt("Library: 1=Gebisa, 2=Law, 3=Female: ");
                currentLib = selectLibrary(libChoice, gebisa, law, female);
            }

            switch (choice) {
                case 1: { // Add Book
                    if (userRole != "Administrator" && userRole != "Librarian")
                        throw AccessDeniedException("Access denied: only Administrators or Librarians can add books.");

                    int bookId    = readPositiveInt("Book ID: ");
                    cin.ignore();
                    string title    = readNonEmptyString("Title: ");
                    string category = readNonEmptyString("Category: ");
                    int copies    = readPositiveInt("Copies: ");
                    int libChoice = readInt("Add to library (1=Gebisa, 2=Law, 3=Female): ");
                    Library* target = selectLibrary(libChoice, gebisa, law, female);
                    target->addBook(bookId, title, category, copies);
                    break;
                }

                case 2: { // Search Book
                    int bookId = readInt("Book ID (-1 for title search): ");
                    if (bookId == -1) {
                        cin.ignore();
                        string title = readNonEmptyString("Title: ");
                        Book* b = currentLib->searchBook(-1, title);
                        if (b) cout << "Found: " << b->title << " (Available: " << b->availableCopies << ")\n";
                        else   cout << "Book not found.\n";
                    } else {
                        if (bookId <= 0)
                            throw InvalidInputException("Book ID must be a positive number or -1 for title search.");
                        Book* b = currentLib->searchBook(bookId);
                        if (b) cout << "Found: " << b->title << " (Available: " << b->availableCopies << ")\n";
                        else   cout << "Book not found.\n";
                    }
                    break;
                }

                case 3: { // Borrow Book
                    int studentId = readPositiveInt("Student ID: ");
                    int bookId    = readPositiveInt("Book ID: ");
                    currentLib->borrowBook(studentId, bookId, students);
                    break;
                }

                case 4: { // Return Book
                    int bookId = readPositiveInt("Book ID: ");
                    currentLib->returnBook(bookId, students);
                    break;
                }

                case 5: { // View Waiting Queue
                    currentLib->viewQueue();
                    break;
                }

                case 6: { // Shelf Returned Books
                    if (userRole != "Administrator" && userRole != "Librarian")
                        throw AccessDeniedException("Access denied: only Administrators or Librarians can shelve books.");
                    currentLib->shelfReturnedBook();
                    break;
                }

                case 7: { // Display All Books
                    gebisa.displayBooks();
                    law.displayBooks();
                    female.displayBooks();
                    break;
                }
            }

        } catch (const AccessDeniedException& e) {
            cout << "[Access Denied] " << e.what() << "\n";
        } catch (const BookNotFoundException& e) {
            cout << "[Book Error] " << e.what() << "\n";
        } catch (const StudentNotFoundException& e) {
            cout << "[Student Error] " << e.what() << "\n";
        } catch (const BorrowLimitException& e) {
            cout << "[Borrow Limit] " << e.what() << "\n";
        } catch (const InvalidInputException& e) {
            cout << "[Input Error] " << e.what() << "\n";
        } catch (const exception& e) {
            cout << "[Unexpected Error] " << e.what() << "\n";
        }
    }

    cout << "Goodbye.\n";
    return 0;
}