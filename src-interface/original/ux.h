#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <conio.h>
#include "module.h"


void setTextColor(int textColor, int bgColor) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
}

void resetColors() {
    setTextColor(7, 0);
}

void printCenteredBlock(const std::vector<std::string>& block, int highlightLine = -1) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int consoleHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    int paddingY = (consoleHeight - block.size()) / 2;

    for (int i = 0; i < paddingY; ++i) {
        std::cout << std::endl;
    }
    for (size_t i = 0; i < block.size(); ++i) {
        int paddingX = (consoleWidth - block[i].length()) / 2;
        for (int j = 0; j < paddingX; ++j) {
            std::cout << " ";
        }
        if (static_cast<int>(i) == highlightLine && block[i].length() > 2) {
            for (size_t j = 0; j < block[i].length(); ++j) {
                if (block[i][j] == '|') {
                    std::cout << block[i][j];
                }
                else {
                    setTextColor(0, 15);
                    std::cout << block[i][j];
                    resetColors();
                }
            }
        }
        else {
            std::cout << block[i];
        }
        std::cout << std::endl;
    }
}

void hideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void showCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
int getBDR(int A, int B, int C, int D) {
    if (A)
        if (B)
            if (C)
                if (D)
                    return 0;
                else
                    return 1;
            else
                if (D)
                    return 1;
                else
                    return 0;
        else
            return 0;
    else
        if (B)
            if (D)
                return 0;
            else
                return 1;
        else
            if (C)
                return 0;
            else
                if (D)
                    return 1;
                else
                    return 0;
}

std::string calc(std::vector<int> values, std::string type) {
    int A = values[0];
    int B = values[1];
    int C = values[2];
    int D = values[3];
    int response = NAN;
    if (type == "BDR")
        response = getBDR(A, B, C, D);
    else if (type == "SDNF")
        for (std::vector<int> el : sdnfList) {
            unsigned int tempRes = 1;
            for (int i = 0; i < 4; i++)
                tempRes *= values[i] + el[i];
            if (response == NAN) { response = 0; }
            response = response or tempRes;
        }
    else if (type == "POLY") {
        for (std::vector<int> el : polyList) {
            int tempRes = 1;
            for (int i = 0; i < 4; i++)
                if (el[i]) { tempRes *= values[i]; }
            if (response == NAN) { response = tempRes; }
            response = response xor tempRes;
        }
    }
    return std::to_string(response);
}

void drawCalc(std::vector<int> valuesUser, std::string type, std::vector<std::string> text = BDR) {
    system("cls");
    auto temp = text;
    temp.push_back("");
    temp.push_back("");
    if (valuesUser.size() == 4) {
        temp.push_back("A B C D : F");
        std::string temp_string = " ";
        for (int i = 0; i < 4; i++) {
            if (i < valuesUser.size())
                temp_string += std::to_string(valuesUser[i]) + " ";
            else
                temp_string += "_ ";
        }
        temp.push_back(temp_string + ": " + calc(valuesUser, type) + " ");
    }
    else {
        temp.push_back("¬ведите A B C D");
        std::string temp_string = " ";
        for (int i = 0; i < 4; i++) {
            if (i < valuesUser.size())
                temp_string += std::to_string(valuesUser[i]) + " ";
            else
                temp_string += "_ ";
        }
        temp.push_back(temp_string);
    }
    printCenteredBlock(temp, -1);
}

void choosePrintCenteredBlock(std::vector<std::string> block, int defaultHighLight = 0) {

    int highlightLine = defaultHighLight;
    hideCursor();
    printCenteredBlock(block, highlightLine);
    std::string state = "menu";
    std::vector<int> valuesUser;

    while (true) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == 27) {
                break;
            }
            else if (ch == -32) {
                ch = _getch();
                state = "menu";
                valuesUser.clear();
                if (ch == 72 && highlightLine > 2) {
                    --highlightLine;
                    system("cls");
                    printCenteredBlock(block, highlightLine);
                }
                else if (ch == 80 && highlightLine < block.size() - 2) {
                    ++highlightLine;
                    system("cls");
                    printCenteredBlock(block, highlightLine);
                }
            }
            else if (ch == 13) {
                if (highlightLine == 2) {
                    system("cls");
                    printCenteredBlock(tablePrint(), 1);
                    state = "table";
                }
                else if (highlightLine == 3) {
                    drawCalc(valuesUser, "BDR");
                    state = "diagram";
                }
                else if (highlightLine == 4) {
                    state = "forma";
                    system("cls");
                    drawCalc(valuesUser, "SDNF", formaPrint());
                }
                else if (highlightLine == 5) {
                    state = "polynom";
                    system("cls");
                    drawCalc(valuesUser, "POLY", polyPrint());
                }
            }
            else if (ch == '1' and (state == "diagram" or state == "forma" or state == "polynom")) {
                if (valuesUser.size() == 4) { valuesUser.clear(); }
                valuesUser.push_back(1);
                if (state == "diagram")
                    drawCalc(valuesUser, "BDR");
                if (state == "forma")
                    drawCalc(valuesUser, "SDNF", formaPrint());
                if (state == "polynom")
                    drawCalc(valuesUser, "POLY", polyPrint());
            }
            else if (ch == '0' and (state == "diagram" or state == "forma" or state == "polynom")) {
                if (valuesUser.size() == 4) { valuesUser.clear(); }
                valuesUser.push_back(0);
                if (state == "diagram")
                    drawCalc(valuesUser, "BDR");
                if (state == "forma")
                    drawCalc(valuesUser, "SDNF", formaPrint());
                if (state == "polynom")
                    drawCalc(valuesUser, "POLY", polyPrint());
            }
        }
    }
}

