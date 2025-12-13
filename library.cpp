#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <map>
#include <stdexcept>

// --------------------- БАЗОВЫЙ КЛАСС КНИГИ ---------------------
class Book {
protected:
    std::string title;
    std::string author;
    int year;
    bool isBorrowed;

public:
    Book(const std::string& t, const std::string& a, int y)
        : title(t), author(a), year(y), isBorrowed(false) {}

    virtual ~Book() {}

    virtual void print() const {
        std::cout << "Название: " << title
                  << ", Автор: " << author
                  << ", Год: " << year
                  << ", Взята: " << (isBorrowed ? "Да" : "Нет") << std::endl;
    }

    virtual std::string serialize() const = 0;

    const std::string& getTitle() const { return title; }
    const std::string& getAuthor() const { return author; }
    int getYear() const { return year; }

    void borrow() { isBorrowed = true; }
    void returnBook() { isBorrowed = false; }
    bool borrowed() const { return isBorrowed; }
};

// --------------------- НАУЧНАЯ КНИГА ---------------------
class ScienceBook : public Book {
    std::string field;

public:
    ScienceBook(const std::string& t, const std::string& a, int y, const std::string& f)
        : Book(t, a, y), field(f) {}

    void print() const override {
        Book::print();
        std::cout << "Область науки: " << field << std::endl;
    }

    std::string serialize() const override {
        return "ScienceBook;" + title + ";" + author + ";" + std::to_string(year) + ";" + field + ";" + (borrowed() ? "1" : "0");
    }
};

// --------------------- ХУДОЖЕСТВЕННАЯ КНИГА ---------------------
class FictionBook : public Book {
    std::string genre;

public:
    FictionBook(const std::string& t, const std::string& a, int y, const std::string& g)
        : Book(t, a, y), genre(g) {}

    void print() const override {
        Book::print();
        std::cout << "Жанр: " << genre << std::endl;
    }

    std::string serialize() const override {
        return "FictionBook;" + title + ";" + author + ";" + std::to_string(year) + ";" + genre + ";" + (borrowed() ? "1" : "0");
    }
};

// --------------------- ПОЛЬЗОВАТЕЛЬ ---------------------
class User {
    std::string name;
    int id;
    std::vector<std::shared_ptr<Book>> borrowedBooks;

public:
    User(std::string n, int i) : name(n), id(i) {}

    void borrowBook(const std::shared_ptr<Book>& book) {
        if (book->borrowed()) {
            throw std::runtime_error("Книга уже взята!");
        }
        book->borrow();
        borrowedBooks.push_back(book);
    }

    void returnBook(const std::string& title) {
        auto it = std::find_if(borrowedBooks.begin(), borrowedBooks.end(), [&](const std::shared_ptr<Book>& b) {
            return b->getTitle() == title;
        });
        if (it != borrowedBooks.end()) {
            (*it)->returnBook();
            borrowedBooks.erase(it);
        } else {
            throw std::runtime_error("Пользователь не брал эту книгу!");
        }
    }

    void printBorrowed() const {
        std::cout << "Пользователь: " << name << ", ID: " << id << "\nВзятые книги:\n";
        for (const auto& book : borrowedBooks) {
            std::cout << "  - " << book->getTitle() << std::endl;
        }
    }

    const std::string& getName() const { return name; }
    int getId() const { return id; }
};

// --------------------- БИБЛИОТЕКА ---------------------
class Library {
    std::vector<std::shared_ptr<Book>> books;
    std::map<int, std::shared_ptr<User>> users;

public:
    void addBook(const std::shared_ptr<Book>& book) {
        books.push_back(book);
    }

    void addUser(const std::shared_ptr<User>& user) {
        users[user->getId()] = user;
    }

    void showAllBooks() const {
        for (const auto& book : books) {
            book->print();
            std::cout << "---------------------\n";
        }
    }

    void showAllUsers() const {
        for (const auto& [id, user] : users) {
            user->printBorrowed();
            std::cout << "---------------------\n";
        }
    }

    std::shared_ptr<Book> findBook(const std::string& title) const {
        for (const auto& book : books) {
            if (book->getTitle() == title) return book;
        }
        return nullptr;
    }

    void borrowBook(int userId, const std::string& title) {
        auto userIt = users.find(userId);
        if (userIt == users.end()) throw std::runtime_error("Пользователь не найден!");
        auto book = findBook(title);
        if (!book) throw std::runtime_error("Книга не найдена!");
        userIt->second->borrowBook(book);
    }

    void returnBook(int userId, const std::string& title) {
        auto userIt = users.find(userId);
        if (userIt == users.end()) throw std::runtime_error("Пользователь не найден!");
        userIt->second->returnBook(title);
    }

    void sortBooksByTitle() {
        std::sort(books.begin(), books.end(), [](const std::shared_ptr<Book>& a, const std::shared_ptr<Book>& b) {
            return a->getTitle() < b->getTitle();
        });
    }

    void saveToBinaryFile(const std::string& filename) {
        std::ofstream out(filename, std::ios::binary);
        if (!out) throw std::runtime_error("Ошибка при открытии файла для записи");

        size_t count = books.size();
        out.write(reinterpret_cast<char*>(&count), sizeof(count));

        for (auto& book : books) {
            std::string serialized = book->serialize();
            size_t len = serialized.size();
            out.write(reinterpret_cast<char*>(&len), sizeof(len));
            out.write(serialized.c_str(), len);
        }

        out.close();
    }

    void loadFromBinaryFile(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in) throw std::runtime_error("Ошибка при открытии файла для чтения");

        books.clear();

        size_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));

        for (size_t i = 0; i < count; ++i) {
            size_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            std::string serialized(len, '\0');
            in.read(&serialized[0], len);

            std::vector<std::string> parts;
            size_t pos = 0;
            std::string temp = serialized;
            while ((pos = temp.find(';')) != std::string::npos) {
                parts.push_back(temp.substr(0, pos));
                temp.erase(0, pos + 1);
            }
            parts.push_back(temp);

            if (parts[0] == "ScienceBook" && parts.size() == 6) {
                auto b = std::make_shared<ScienceBook>(parts[1], parts[2], std::stoi(parts[3]), parts[4]);
                if (parts[5] == "1") b->borrow();
                books.push_back(b);
            } else if (parts[0] == "FictionBook" && parts.size() == 6) {
                auto b = std::make_shared<FictionBook>(parts[1], parts[2], std::stoi(parts[3]), parts[4]);
                if (parts[5] == "1") b->borrow();
                books.push_back(b);
            }
        }

        in.close();
    }
};

// --------------------- ГЛАВНАЯ ФУНКЦИЯ ---------------------
int main() {
    Library lib;

    // Добавляем книги
    lib.addBook(std::make_shared<ScienceBook>("Физика для всех", "Иванов", 2010, "Физика"));
    lib.addBook(std::make_shared<FictionBook>("Приключения в лесу", "Петров", 2015, "Приключения"));
    lib.addBook(std::make_shared<ScienceBook>("Химия и жизнь", "Сидоров", 2012, "Химия"));
    lib.addBook(std::make_shared<FictionBook>("Мир фантазий", "Иванов", 2020, "Фэнтези"));

    // Добавляем пользователей
    lib.addUser(std::make_shared<User>("Алексей", 1));
    lib.addUser(std::make_shared<User>("Мария", 2));

    std::cout << "Все книги:\n";
    lib.showAllBooks();

    // Пользователи берут книги
    try {
        lib.borrowBook(1, "Физика для всех");
        lib.borrowBook(2, "Мир фантазий");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::cout << "\nСостояние пользователей после выдачи книг:\n";
    lib.showAllUsers();

    // Сохраняем библиотеку в бинарный файл
    try {
        lib.saveToBinaryFile("library.dat");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Загружаем из бинарного файла в новую библиотеку
    Library lib2;
    try {
        lib2.loadFromBinaryFile("library.dat");
        std::cout << "\nЗагруженная библиотека:\n";
        lib2.showAllBooks();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
