#pragma once
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <algorithm>

std::string symbols = "ABCD";
int TASK = 18950; // 18950
std::vector<std::string> BDR{
    "           +---+           ",
    "    .......| A |------+    ",
    "    :      +---+      |    ",
    "  +---+             +---+  ",
    "..| B |--+  ........| B |-+",
    ": +---+  |  :       +---+ |",
    ":        |  :             |",
    ":....    +--:-+       +---+",
    "    :       : |       |    ",
    "  +---+     : |     +---+  ",
    "..| C |--+  : |  ...| C |-+",
    ": +---+  |  : |  :  +---+ |",
    ":        |  : +--:----+   |",
    ":........|..:....:    +---+",
    "    :    |  :         |    ",
    "  +---+  |  :       +---+  ",
    "..| D |--|--:--+ ...| D |-+",
    ": +---+  |  :  | :  +---+ |",
    ":        |-------:--------+",
    ":        |  :  | :         ",
    ":........O..:  +-O         ",
    "         |       |         ",
    "       +---+   +---+       ",
    "       | 0 |   | 1 |       ",
    "       +---+   +---+       ",
};

std::vector<std::string> toBinaryVector(int number, int bitWidth) {
    std::bitset<32> binary(number);

    std::string binaryString = binary.to_string();
    binaryString = binaryString.substr(32 - bitWidth, bitWidth);
    std::vector<std::string> binaryVector;
    for (char bit : binaryString) {
        binaryVector.push_back(std::string(1, bit));
    }
    return binaryVector;
}

std::vector<std::vector<std::string>> generateTable(std::vector<std::string> boolFunc) {
    std::vector<std::vector<std::string>> result = { {"A", "B", "C", "D", "F"} };
    for (int i = 0; i < boolFunc.size(); i++) {
        std::vector<std::string> row = toBinaryVector(i, 4);
        row.push_back(boolFunc[i]);
        result.push_back(row);
    }
    return result;
}

auto boolFunc = toBinaryVector(TASK, 16);
std::vector<std::string> tablePrint() {
    const std::vector<std::vector<std::string>> array2d = generateTable(boolFunc);
    const int width = 3;
    const char p = '+';
    int size = array2d.size();
    std::string gap = p + std::string((width + 1) * 5 - 1, '-') + p;
    std::vector<std::string> result;

    for (const auto& row : array2d) {
        result.push_back(gap);
        std::string str;
        str += "|";
        for (const auto& el : row) {
            std::string elStr = el;
            std::string space = std::string(width - elStr.size() - 1, ' ');
            str += space + elStr + " |";
        }
        result.push_back(str);
    }
    result.push_back(gap);
    return result;
}
std::vector<std::vector<int>> sdnfList;
std::vector<std::string> formaPrint() {
    std::string void_sdnf = "|        ";
    std::string sdnf = "| —ƒÕ‘ = ";
    std::string void_sknf = "|        ";
    std::string sknf = "| — Õ‘ = ";
    sdnfList.clear();

    int countD = 0, countK = 0;


    std::vector<std::string> respD_void, respD_formula;
    std::vector<std::string> respK_void, respK_formula;

    for (int i = 0; i < boolFunc.size(); i++) {
        auto row = toBinaryVector(i, 4);

        if (boolFunc[i] == "0") {
            countK += 1;
            std::string void_temp = " ";
            std::string sknf_temp = "(";

            for (int j = 0; j < row.size(); j++) {
                sknf_temp += symbols[j];
                if (row[j] == "0") {
                    void_temp += "_";
                }
                else {
                    void_temp += " ";
                }

                if (j < row.size() - 1) {
                    sknf_temp += "+";
                    void_temp += " ";
                }
            }
            sknf_temp += ")";

            respK_void.push_back(void_temp + "    ");
            respK_formula.push_back(sknf_temp + (i < boolFunc.size() - 1 ? " * " : "   "));
        }

        else if (boolFunc[i] == "1") {
            countD += 1;
            std::string void_temp = " ";
            std::string sdnf_temp = "";
            std::vector<int> tempPart;

            for (int j = 0; j < row.size(); j++) {
                sdnf_temp += symbols[j];
                if (row[j] == "0") {
                    void_temp += "_";
                    tempPart.push_back(-1);
                }
                else {
                    void_temp += " ";
                    tempPart.push_back(0);
                }
            }
            sdnfList.push_back(tempPart);

            respD_void.push_back(void_temp + "   ");
            respD_formula.push_back(sdnf_temp + (i < boolFunc.size() - 1 ? " + " : "   "));
        }
    }

    std::vector<std::string> response;
    std::string gap(44, '-');

    response.push_back("+" + gap + "+");

    int max_lines = max(respK_formula.size(), respD_formula.size());
    for (int line = 0; line < max_lines; line++) {
        std::string void_line = "|        ";
        std::string formula_line = "| — Õ‘ = ";

        if (line < respK_formula.size()) {
            void_line += respK_void[line];
            formula_line += respK_formula[line];
        }

        while (void_line.length() < gap.length() + 1) void_line += " ";
        while (formula_line.length() < gap.length() + 1) formula_line += " ";

        void_line += "|";
        formula_line += "|";

        response.push_back(void_line);
        response.push_back(formula_line);
    }

    response.push_back("+" + gap + "+");

    // —ƒÕ‘ ·ÎÓÍ
    for (int line = 0; line < max_lines; line++) {
        std::string void_line = "|        ";
        std::string formula_line = "| —ƒÕ‘ = ";

        if (line < respD_formula.size()) {
            void_line += respD_void[line];
            formula_line += respD_formula[line];
        }
        while (void_line.length() < gap.length() + 1) void_line += " ";
        while (formula_line.length() < gap.length() + 1) formula_line += " ";

        void_line += "|";
        formula_line += "|";

        response.push_back(void_line);
        response.push_back(formula_line);
    }

    response.push_back("+" + gap + "+");

    return response;
}

std::vector<std::vector<int>> polyList;
std::vector<std::string> polyPrint() {
    std::vector<std::string> response;
    std::string formula = "∆ = ";
    std::vector<std::string> lastLine = boolFunc;
    polyList.clear();
    for (int i = 0; i < 16; i++) {
        std::string tempLine = lastLine[0];
        std::vector<std::string> newLine;
        for (int j = 1; j < 16 - i; j++) {
            tempLine += " " + lastLine[j];
            newLine.push_back(std::to_string(lastLine[j] != lastLine[j - 1]));
        }
        if (lastLine[0] == "1") {
            std::vector<std::string> row = toBinaryVector(i, 4);
            std::string sub;
            std::vector<int> subList;
            for (int u = 0; u < 4; u++) {
                if (row[u] == "1") {
                    sub += symbols[u];
                    subList.push_back(1);
                }
                else
                    subList.push_back(0);
            }
            formula += sub + " + ";
            polyList.push_back(subList);
        }
        lastLine = newLine;
        response.push_back(tempLine);
    }
    response.push_back("");
    response.push_back("+" + std::string(formula.size() - 1, '-') + "+");
    response.push_back("| " + formula.substr(0, formula.size() - 3) + " |");
    response.push_back("+" + std::string(formula.size() - 1, '-') + "+");

    return response;
}