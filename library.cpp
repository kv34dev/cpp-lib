#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

// --- Базовый класс Книга ---
class Book {
protected:
    std::string title;
    std::string author;
    int year;

public:
    Book(const std::string& t, const std::string& a, int y)
        : title(t), author(a), year(y) {}

    virtual ~Book() {}

    virtual void print() const {
        std::cout << "Название: " << title
                  << ", Автор: " << author
                  << ", Год: " << year << std::endl;
    }

    virtual std::string serialize() const {
        return title + ";" + author + ";" + std::to_string(year);
    }

    const std::string& getTitle() const { return title; }
    const std::string& getAuthor() const { return author; }
};

// --- Класс научной книги ---
class ScienceBook : public Book {
    std::string field; // область науки

public:
    ScienceBook(const std::string& t, const std::string& a, int y, const std::string& f)
        : Book(t, a, y), field(f) {}

    void print() const override {
        Book::print();
        std::cout << "Область науки: " << field << std::endl;
    }

    std::string serialize() const override {
        return "ScienceBook;" + Book::serialize() + ";" + field;
    }
};

// --- Класс художественной книги ---
class FictionBook : public Book {
    std::string genre; // жанр

public:
    FictionBook(const std::string& t, const std::string& a, int y, const std::string& g)
        : Book(t, a, y), genre(g) {}

    void print() const override {
        Book::print();
        std::cout << "Жанр: " << genre << std::endl;
    }

    std::string serialize() const override {
        return "FictionBook;" + Book::serialize() + ";" + genre;
    }
};

// --- Класс библиотеки ---
class Library {
    std::vector<std::shared_ptr<Book>> books;

public:
    void addBook(const std::shared_ptr<Book>& book) {
        books.push_back(book);
    }

    void showAll() const {
        for (const auto& book : books) {
            book->print();
            std::cout << "---------------------\n";
        }
    }

    void searchByAuthor(const std::string& author) const {
        bool found = false;
        for (const auto& book : books) {
            if (book->getAuthor() == author) {
                book->print();
                std::cout << "---------------------\n";
                found = true;
            }
        }
        if (!found) std::cout << "Книг автора \"" << author << "\" не найдено.\n";
    }

    void saveToFile(const std::string& filename) const {
        std::ofstream out(filename);
        if (!out) {
            std::cerr << "Ошибка открытия файла для записи.\n";
            return;
        }
        for (const auto& book : books) {
            out << book->serialize() << "\n";
        }
        out.close();
        std::cout << "Библиотека сохранена в файл \"" << filename << "\"\n";
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream in(filename);
        if (!in) {
            std::cerr << "Ошибка открытия файла для чтения.\n";
            return;
        }

        books.clear(); // очищаем библиотеку перед загрузкой

        std::string line;
        while (std::getline(in, line)) {
            std::vector<std::string> parts;
            size_t pos = 0;
            while ((pos = line.find(';')) != std::string::npos) {
                parts.push_back(line.substr(0, pos));
                line.erase(0, pos + 1);
            }
            parts.push_back(line); // последний элемент

            if (parts.size() < 4) continue;

            if (parts[0] == "ScienceBook" && parts.size() == 5) {
                books.push_back(std::make_shared<ScienceBook>(parts[1], parts[2], std::stoi(parts[3]), parts[4]));
            } else if (parts[0] == "FictionBook" && parts.size() == 5) {
                books.push_back(std::make_shared<FictionBook>(parts[1], parts[2], std::stoi(parts[3]), parts[4]));
            }
        }

        in.close();
        std::cout << "Библиотека загружена из файла \"" << filename << "\"\n";
    }
};

// --- Главная функция ---
int main() {
    Library lib;

    // Добавляем книги
    lib.addBook(std::make_shared<ScienceBook>("Физика для всех", "Иванов", 2010, "Физика"));
    lib.addBook(std::make_shared<FictionBook>("Приключения в лесу", "Петров", 2015, "Приключения"));
    lib.addBook(std::make_shared<ScienceBook>("Химия и жизнь", "Сидоров", 2012, "Химия"));
    lib.addBook(std::make_shared<FictionBook>("Мир фантазий", "Иванов", 2020, "Фэнтези"));

    std::cout << "Все книги в библиотеке:\n";
    lib.showAll();

    std::cout << "\nПоиск книг автора 'Иванов':\n";
    lib.searchByAuthor("Иванов");

    // Сохраняем в файл
    lib.saveToFile("library.txt");

    // Создаем новую библиотеку и загружаем из файла
    Library lib2;
    lib2.loadFromFile("library.txt");
    std::cout << "\nЗагруженная библиотека:\n";
    lib2.showAll();

    return 0;
}
