#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#endif
#include "Module.h"

struct ConsoleSize {
    int width;
    int height;
};

ConsoleSize getConsoleSize() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return {
            csbi.srWindow.Right - csbi.srWindow.Left + 1,
            csbi.srWindow.Bottom - csbi.srWindow.Top + 1
        };
    }
    return { 80, 25 };
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return { 80, 25 };
    }
    return { ws.ws_col, ws.ws_row };
#endif
}

void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H" << std::flush;
#endif
}

#ifndef _WIN32
class TerminalRawGuard {
public:
    TerminalRawGuard() {
        if (tcgetattr(STDIN_FILENO, &original_) == -1) {
            active_ = false;
            return;
        }
        termios raw = original_;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == -1) {
            active_ = false;
            return;
        }
        active_ = true;
    }

    ~TerminalRawGuard() {
        if (active_) {
            tcsetattr(STDIN_FILENO, TCSANOW, &original_);
        }
    }

private:
    termios original_{};
    bool active_{ false };
};

bool readCharWithTimeout(char& ch, int timeoutMs = 50) {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    timeval tv;
    tv.tv_sec = timeoutMs / 1000;
    tv.tv_usec = (timeoutMs % 1000) * 1000;
    int result = select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv);
    if (result > 0) {
        return read(STDIN_FILENO, &ch, 1) == 1;
    }
    return false;
}

enum class KeyPress {
    Escape,
    ArrowUp,
    ArrowDown,
    Enter,
    Digit0,
    Digit1,
    Unknown
};

KeyPress readLinuxKey() {
    char ch;
    if (read(STDIN_FILENO, &ch, 1) != 1) {
        return KeyPress::Unknown;
    }
    if (ch == '\033') {
        char next;
        if (!readCharWithTimeout(next, 30)) {
            return KeyPress::Escape;
        }
        if (next == '[') {
            char arrow;
            if (!readCharWithTimeout(arrow, 30)) {
                return KeyPress::Escape;
            }
            if (arrow == 'A') {
                return KeyPress::ArrowUp;
            }
            if (arrow == 'B') {
                return KeyPress::ArrowDown;
            }
        }
        return KeyPress::Escape;
    }
    if (ch == '\r' || ch == '\n') {
        return KeyPress::Enter;
    }
    if (ch == '0') {
        return KeyPress::Digit0;
    }
    if (ch == '1') {
        return KeyPress::Digit1;
    }
    return KeyPress::Unknown;
}
#endif


void setTextColor(int textColor, int bgColor) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
#else
    int fg = (textColor % 8) + (textColor >= 8 ? 90 : 30);
    int bg = (bgColor % 8) + (bgColor >= 8 ? 100 : 40);
    std::cout << "\033[" << fg << ";" << bg << "m";
#endif
}

void resetColors() {
#ifdef _WIN32
    setTextColor(7, 0);
#else
    std::cout << "\033[0m";
#endif
}

void printCenteredBlock(const std::vector<std::string>& block, int highlightLine = -1) {
    ConsoleSize size = getConsoleSize();
    int consoleWidth = size.width;
    int consoleHeight = size.height;
    int paddingY = std::max(0, (consoleHeight - static_cast<int>(block.size())) / 2);

    for (int i = 0; i < paddingY; ++i) {
        std::cout << std::endl;
    }
    for (size_t i = 0; i < block.size(); ++i) {
        int paddingX = std::max(0, (consoleWidth - static_cast<int>(block[i].length())) / 2);
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
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\033[?25l" << std::flush;
#endif
}

void showCursor() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\033[?25h" << std::flush;
#endif
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
    int response = 0;
    if (type == "BDR") {
        response = getBDR(A, B, C, D);
    }
    else if (type == "SDNF") {
        for (const std::vector<int>& el : sdnfList) {
            unsigned int tempRes = 1;
            for (int i = 0; i < 4; i++) {
                tempRes *= values[i] + el[i];
            }
            response = response || tempRes;
        }
    }
    else if (type == "POLY") {
        for (const std::vector<int>& el : polyList) {
            int tempRes = 1;
            for (int i = 0; i < 4; i++) {
                if (el[i]) {
                    tempRes *= values[i];
                }
            }
            response ^= tempRes;
        }
    }
    return std::to_string(response);
}

void drawCalc(std::vector<int> valuesUser, std::string type, std::vector<std::string> text = BDR) {
    clearConsole();
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
        temp.push_back("Введите A B C D");
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

#ifndef _WIN32
    TerminalRawGuard rawGuard;
#endif

#ifdef _WIN32
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
                    clearConsole();
                    printCenteredBlock(block, highlightLine);
                }
                else if (ch == 80 && highlightLine < block.size() - 2) {
                    ++highlightLine;
                    clearConsole();
                    printCenteredBlock(block, highlightLine);
                }
            }
            else if (ch == 13) {
                if (highlightLine == 2) {
                    clearConsole();
                    printCenteredBlock(tablePrint(), 1);
                    state = "table";
                }
                else if (highlightLine == 3) {
                    drawCalc(valuesUser, "BDR");
                    state = "diagram";
                }
                else if (highlightLine == 4) {
                    state = "forma";
                    clearConsole();
                    drawCalc(valuesUser, "SDNF", formaPrint());
                }
                else if (highlightLine == 5) {
                    state = "polynom";
                    clearConsole();
                    drawCalc(valuesUser, "POLY", polyPrint());
                }
            }
            else if (ch == '1' && (state == "diagram" || state == "forma" || state == "polynom")) {
                if (valuesUser.size() == 4) { valuesUser.clear(); }
                valuesUser.push_back(1);
                if (state == "diagram")
                    drawCalc(valuesUser, "BDR");
                if (state == "forma")
                    drawCalc(valuesUser, "SDNF", formaPrint());
                if (state == "polynom")
                    drawCalc(valuesUser, "POLY", polyPrint());
            }
            else if (ch == '0' && (state == "diagram" || state == "forma" || state == "polynom")) {
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
#else
    while (true) {
        KeyPress key = readLinuxKey();
        if (key == KeyPress::Escape) {
            break;
        }
        else if (key == KeyPress::ArrowUp) {
            state = "menu";
            valuesUser.clear();
            if (highlightLine > 2) {
                --highlightLine;
                clearConsole();
                printCenteredBlock(block, highlightLine);
            }
        }
        else if (key == KeyPress::ArrowDown) {
            state = "menu";
            valuesUser.clear();
            if (highlightLine < static_cast<int>(block.size()) - 2) {
                ++highlightLine;
                clearConsole();
                printCenteredBlock(block, highlightLine);
            }
        }
        else if (key == KeyPress::Enter) {
            if (highlightLine == 2) {
                clearConsole();
                printCenteredBlock(tablePrint(), 1);
                state = "table";
            }
            else if (highlightLine == 3) {
                drawCalc(valuesUser, "BDR");
                state = "diagram";
            }
            else if (highlightLine == 4) {
                state = "forma";
                clearConsole();
                drawCalc(valuesUser, "SDNF", formaPrint());
            }
            else if (highlightLine == 5) {
                state = "polynom";
                clearConsole();
                drawCalc(valuesUser, "POLY", polyPrint());
            }
        }
        else if (key == KeyPress::Digit1 && (state == "diagram" || state == "forma" || state == "polynom")) {
            if (valuesUser.size() == 4) { valuesUser.clear(); }
            valuesUser.push_back(1);
            if (state == "diagram")
                drawCalc(valuesUser, "BDR");
            if (state == "forma")
                drawCalc(valuesUser, "SDNF", formaPrint());
            if (state == "polynom")
                drawCalc(valuesUser, "POLY", polyPrint());
        }
        else if (key == KeyPress::Digit0 && (state == "diagram" || state == "forma" || state == "polynom")) {
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
#endif
    showCursor();
}

