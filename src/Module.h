#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>

std::string string = "abcdefgh";
std::string task = "b d g e c/f g h a";
std::unordered_map<char, std::string> base;
std::unordered_map<char, std::string> I_base;
std::vector<std::string> road = { "a" };

void buildBase() {
    size_t i = 0;
    std::string tempTask = task;
    std::string delimiter = " ";
    size_t pos = 0;

    while ((pos = tempTask.find(delimiter)) != std::string::npos) {
        base[string[i]] = tempTask.substr(0, pos);
        tempTask.erase(0, pos + delimiter.length());
        ++i;
    }
    base[string[i]] = tempTask;
}

void buildInvertedBase() {
    for (const auto& pair : base) {
        char key = pair.first;
        const std::string& value = pair.second;

        if (value.size() == 3) {
            I_base[value[0]] = key;
            I_base[value[2]] = key;
        }
        else if (I_base.find(value[0]) != I_base.end()) {
            I_base[value[0]] += key;
        }
        else {
            I_base[value[0]] = key;
        }
    }
}

void buildHasseDiagram() {
    std::string tempVal = base[road.back()[0]];
    while (tempVal != "a") {
        road.push_back(tempVal);
        if (road.back().size() == 2) {
            tempVal = base[road.back()[0]];
        }
        else {
            tempVal = base[road.back()[0]];
        }
    }
}

std::string inputCorrect(const std::string& whiteList, const std::string& comment) {
    std::string ans;
    while (true) {
        try {
            std::getline(std::cin, ans);
            if (ans == "stop") exit(0);

            bool valid = std::all_of(ans.begin(), ans.end(), [&whiteList](char ch) {
                return whiteList.find(ch) != std::string::npos;
                });

            if (ans.size() > 8 && ans[0] != '-' || ans[0] == '-' && ans.size() > 9)
                valid = false;

            if (valid) break;
            else throw std::invalid_argument("Invalid input");
        }
        catch (...) {
            std::cout << comment;
        }
    }
    return ans;
}

std::string textStart() {
    std::cout << "\033[2J\033[1;1H";
    std::cout << "+-----------------------------------+\n";
    std::cout << "| 1. Вывести матрицу суммы          |\n";
    std::cout << "| 2. Вывести матрицу разности       |\n";
    std::cout << "| 3. Вывести матрицу произведения   |\n";
    std::cout << "| 4. Произвести вычисления          |\n";
    std::cout << "| 5. Выход                          |\n";
    std::cout << "+-----------------------------------+\n";
    std::cout << ">>  ";
    return inputCorrect("12345", "Неверный выбор, введите допустимый вариант: ");
}

std::tuple<std::string, std::string, char> textGetVal() {
    std::cout << "\033[2J\033[1;1H";
    std::cout << "Введите первый операнд: ";
    std::string op1 = inputCorrect(string + "-", "Неверный ввод: ");

    std::cout << "Введите второй операнд: ";
    std::string op2 = inputCorrect(string + "-", "Неверный ввод: ");

    std::cout << "Введите операцию (+, -, *, /): ";
    char sign = inputCorrect("*+-/", "Неверная операция: ")[0];

    return { op1, op2, sign };
}

void initialize() {
    buildBase();
    buildInvertedBase();
    buildHasseDiagram();
}
