#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include <clocale>
#include <limits>

struct Note {
    std::string content;
    bool isDone;
};

const std::string CONFIG_FILE = "config.txt";
const char SEPARATOR = '|';
const int MENU_SIZE = 7;
std::string GLOBAL_NOTES_PATH = "";
int GLOBAL_TOTAL_COUNT = 0;
int GLOBAL_DONE_COUNT = 0;

//----------------------------------------------------------------

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

std::string getCurrentTimes() {
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "(%d.%m.%Y %H:%M)", ltm);
    return std::string(buffer);
}

void updateConfigStats(const std::vector<Note>& notes) {
    int total = (int)notes.size();
    int done = 0;
    for (size_t i = 0; i < notes.size(); ++i) if (notes[i].isDone) done++;

    GLOBAL_TOTAL_COUNT = total;
    GLOBAL_DONE_COUNT = done;
    std::ofstream cfg(CONFIG_FILE, std::ios::trunc);
    if (cfg.is_open()) {
        cfg << GLOBAL_NOTES_PATH << "\n";
        cfg << GLOBAL_TOTAL_COUNT << "\n";
        cfg << GLOBAL_DONE_COUNT << "\n";
        cfg.close();
    }
}

void InitializeStorage() {
    std::ifstream cfg(CONFIG_FILE);
    if (cfg.is_open()) {
        std::getline(cfg, GLOBAL_NOTES_PATH);

        std::string totalStr, doneStr;
        if (std::getline(cfg, totalStr)) GLOBAL_TOTAL_COUNT = std::stoi(totalStr);
        if (std::getline(cfg, doneStr)) GLOBAL_DONE_COUNT = std::stoi(doneStr);
        cfg.close();
        return;
    }
    system("cls");
    printf("============================================================\n");
    printf("                ПЕРВИЧНАЯ НАСТРОЙКА                        \n");
    printf("------------------------------------------------------------\n");
    printf(" Файл с заметками не найден. Выберите место хранения:\n\n");
    printf(" 1. В папке с программой (по умолчанию)\n");
    printf(" 2. Указать свой путь к папке\n\n");
    printf(" Ваш выбор: ");

    char choice = _getch();
    printf("\n");

    if (choice == '2') {
        printf("\n Введите путь к папке (например, D:\\MyNotes или C:/Data):\n ");
        std::string userDir;
        std::getline(std::cin >> std::ws, userDir);
        if (!userDir.empty() && userDir.back() != '\\' && userDir.back() != '/') {
            userDir += "\\";
        }
        GLOBAL_NOTES_PATH = userDir + "notes.txt";
    }
    else {
        GLOBAL_NOTES_PATH = "notes.txt";
    }
    std::vector<Note> emptyNotes;
    updateConfigStats(emptyNotes);

    printf("\n Настройка сохранена. Путь: %s\n", GLOBAL_NOTES_PATH.c_str());
    printf(" Нажмите любую клавишу для запуска программы...");
    _getch();
}

//----------------------------------------------------------------

std::vector<Note> loadNotes() {
    std::vector<Note> notes;
    std::ifstream file(GLOBAL_NOTES_PATH);
    if (!file.is_open()) return notes;
    std::string line;
    while (std::getline(file, line)) {
        if (line.length() < 3) continue;
        size_t sepPos = line.find(SEPARATOR);
        if (sepPos != std::string::npos) {
            Note note;
            note.isDone = (line[0] == '1');
            note.content = line.substr(sepPos + 1);
            notes.push_back(note);
        }
    }
    file.close();
    return notes;
}

void saveNotes(const std::vector<Note>& notes) {
    std::ofstream file(GLOBAL_NOTES_PATH, std::ios::trunc);
    if (!file.is_open()) return;
    for (size_t i = 0; i < notes.size(); ++i) {
        file << (notes[i].isDone ? "1" : "0") << SEPARATOR << notes[i].content << "\n";
    }
    file.close();
    updateConfigStats(notes);
}

int getValidNoteIndex(int totalNotes) {
    while (true) {
        printf("\nВведите номер заметки (1-%d): ", totalNotes);
        int num;
        if (!(std::cin >> num)) {
            printf("\nОШИБКА: Нужно вводить только цифры!\n");
            clearInputBuffer();
        }
        else if (num < 1 || num > totalNotes) {
            printf("\nОШИБКА: Заметки под номером %d не существует!\n", num);
        }
        else {
            clearInputBuffer();
            return num - 1;
        }
        printf("Попробовать снова? (y - да, любая другая клавиша - выход в меню): ");
        char choice = _getch();
        printf("\n\n");
        if (choice != 'y' && choice != 'Y' && choice != 'н' && choice != 'Н') {
            return -1;
        }
    }
}

bool isInvalidString(const std::string& str) {
    if (str.length() == 0) return true;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] != ' ' && str[i] != '\t') {
            return false;
        }
    }
    return true;
}

//----------------------------------------------------------------

void DrawUI(const char** menu_items, int selected_point, const std::vector<Note>& notes) {
    printf("============================================================\n");
    printf("                МЕНЕДЖЕР ЛИЧНЫХ ЗАМЕТОК                   \n");
    printf(" Путь: %-50s\n", GLOBAL_NOTES_PATH.c_str());
    printf(" Статистика: Всего: %d | Выполнено: %d \n", GLOBAL_TOTAL_COUNT, GLOBAL_DONE_COUNT);
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < MENU_SIZE; i++) {
        if (selected_point == i) printf("  -> %-50s \n", menu_items[i]);
        else printf("     %-50s \n", menu_items[i]);
    }
    printf("------------------------------------------------------------\n");
    printf(" ПРЕДПРОСМОТР (Первые 10 заметок):                        \n");
    if (notes.empty()) {
        printf("   Список пока пуст...                                    \n");
    }
    else {
        int limit = (notes.size() > 10) ? 10 : (int)notes.size();
        for (int i = 0; i < limit; i++) {
            const char* status = notes[i].isDone ? "[V]" : "[ ]";
            std::string preview = notes[i].content;
            if (preview.length() > 50) preview = preview.substr(0, 47) + "...";
            printf(" %2d. %s %s\n", i + 1, status, preview.c_str());
        }
    }
    printf("------------------------------------------------------------\n");
    printf(" Управление: стрелки - выбор, Enter - ок, Esc - выход      \n");
    printf("============================================================\n");
}

int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    InitializeStorage();

    short is_exit = 0;
    int point = 0;

    const char* list_menu[MENU_SIZE] = {
        "Добавить новую заметку",
        "Отметить как выполненную (Done)",
        "Вернуть в работу (Undone)",
        "Удалить заметку",
        "Очистить все выполненные",
        "Показать полный список",
        "Выход"
    };

    std::vector<Note> notes = loadNotes();
    updateConfigStats(notes);

    while (!is_exit) {
        system("cls");
        DrawUI(list_menu, point, notes);
        int key = _getch();
        if (key == 0 || key == 224) {
            key = _getch();
            if (key == 72) point = (point == 0) ? MENU_SIZE - 1 : point - 1;
            else if (key == 80) point = (point == MENU_SIZE - 1) ? 0 : point + 1;
        }
        else if (key == 13) {
            system("cls");
            switch (point) {
            case 0: {
                bool stop_adding = false;
                while (!stop_adding) {
                    printf("Введите текст заметки: ");
                    std::string text;
                    std::getline(std::cin, text);
                    if (isInvalidString(text)) {
                        printf("\nОшибка: Заметка не может быть пустой!");
                        printf("\nХотите ввести заново? (y - да, др. клавиша - выход в меню): ");
                        char choice = _getch();
                        printf("\n\n");
                        if (choice != 'y' && choice != 'Y' && choice != 'н' && choice != 'Н') {
                            stop_adding = true;
                        }
                    }
                    else {
                        Note new_note;
                        new_note.content = text + " " + getCurrentTimes();
                        new_note.isDone = false;
                        notes.push_back(new_note);
                        saveNotes(notes);
                        printf("\nЗаметка добавлена успешно.\n");
                        stop_adding = true;
                    }
                }
                break;
            }
            case 1: case 2: case 3: {
                if (notes.empty()) {printf("Ошибка: Список пуст.");}
                else {
                    bool normal_input=false;
                    while (!normal_input) {
                        int idx = getValidNoteIndex((int)notes.size());
                        if (idx != -1) {
                            if (point == 1) {
                                if (notes[idx].isDone) printf("\nУпс! Эта заметка уже выполнена.");
                                else {
                                    notes[idx].isDone = true;
                                    saveNotes(notes);
                                    printf("\nЗаметка отмечена как выполненная.");
                                    normal_input = true;
                                    if (GLOBAL_DONE_COUNT == GLOBAL_TOTAL_COUNT) {
                                        printf("Вы огромный молодец!!! Выполнил все текущие дела!");
                                    }
                                }
                            }
                            else if (point == 2) {
                                if (!notes[idx].isDone) printf("\nУпс! Заметка и так в работе.");
                                else {
                                    notes[idx].isDone = false;
                                    saveNotes(notes);
                                    printf("\nЗаметка возвращена в список дел.");
                                    normal_input = true;
                                }
                            }
                            else if (point == 3) { // Delete
                                notes.erase(notes.begin() + idx);
                                saveNotes(notes);
                                printf("\nЗаметка удалена.");
                                normal_input = true;
                            }
                        }
                        else {
                        printf("\nВозврат в главное меню...");
                        normal_input = true;
                        }
                    }
                }
                break;
            }
            case 4: {
                bool hasDone = false;
                for (const Note& n : notes) if (n.isDone) { hasDone = true; break; }
                if (!hasDone) {
                    printf("У вас нет выполненных заметок.");
                }
                else {
                    printf("Удалить ВСЕ выполненные заметки? (y/n): ");
                    char confirm = _getch();
                    if (confirm == 'y' || confirm == 'Y' || confirm == 'н' || confirm == 'Н') {
                        std::vector<Note> active;
                        for (const Note& n : notes) if (!n.isDone) active.push_back(n);
                        notes = active;
                        saveNotes(notes);
                        printf("\n\nОчистка завершена.");
                    }
                    else printf("\n\nОтмена операции.");
                }
                break;
            }
            case 5: {
                if (notes.empty()) printf("Список пуст.");
                else {
                    printf("=== ПОЛНЫЙ СПИСОК ЗАМЕТОК ===\n\n");
                    for (int i = 0; i < (int)notes.size(); i++) {
                        printf("%d. %s %s\n", i + 1, notes[i].isDone ? "[V]" : "[ ]", notes[i].content.c_str());
                    }
                }
                break;
            }
            case 6: { is_exit = 1; 
                updateConfigStats(notes);
                break; }
            }
            if (!is_exit) {
                printf("\n\nНажмите любую клавишу для продолжения...");
                _getch();
            }
        }
        else if (key == 27) is_exit = 1;
    }
    return 0;
}