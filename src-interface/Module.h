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
    std::string sdnf = "| СДНФ = ";
    std::string void_sknf = "|        ";
    std::string sknf = "| СКНФ = ";
    sdnfList.clear();

    int countD = 0, countK = 0;

    std::vector<std::string> response, respD, respK;
    for (int i = 0; i < boolFunc.size(); i++) {
        auto row = toBinaryVector(i, 4);
        if (countK == 3) {
            respK.push_back(void_sknf+"|");
            respK.push_back(sknf + "|");
            void_sknf = "|        ";
            sknf = "|      * ";
            countK = 0;
        }
        else if (i == 15) {
            respK.push_back(void_sknf);
            respK.push_back(sknf);
        }
        if (countD == 5) {
            respD.push_back(void_sdnf + " |");
            respD.push_back(sdnf + " |");
            void_sdnf = "|        ";
            sdnf = "|      + ";
            countD = 0;
        }
        else if (i == 15) {
            respD.push_back(void_sdnf);
            respD.push_back(sdnf);
        }
        if (boolFunc[i] == "1") {
            countD += 1;
            std::vector<int> tempPart;
            for (int j = 0; j < row.size(); j++) {
                sdnf += symbols[j];
                if (row[j] == "0") {
                    void_sdnf += "_";
                    tempPart.push_back(-1);
                }
                else {
                    void_sdnf += " ";
                    tempPart.push_back(0);
                }
            }
            sdnfList.push_back(tempPart);
            void_sdnf += "   ";
            sdnf += " + ";
        }
        else {
            countK += 1;
            void_sknf += " ";
            sknf += "(";
            for (int j = 0; j < row.size(); j++) {
                if (j + 1 == row.size()) {
                    sknf += symbols[j];
                    if (row[j] == "0")
                        void_sknf += "_";
                    else
                        void_sknf += " ";
                }
                else {
                    sknf += symbols[j];
                    sknf += "+";
                    if (row[j] == "0")
                        void_sknf += "_ ";
                    else
                        void_sknf += "  ";
                }
            }
            if (i == 15) {
                void_sknf += "    ";
                sknf += ")   ";
            }
            else {
                void_sknf += "    ";
                sknf += ") * ";
            }
        }
    }
    int last = respK.size() - 1;
    respK[last] = respK[last].substr(0, respK[last].size() - 2);
    if (last == 1)
        respK[last] = respK[last].substr(0, respK[last].size() - 2);

    if (last != 1)
        respK[last - 1] += std::string(respK[0].size() - respK[last - 1].size() - 1, ' ') + "|";
    respK[last] += std::string(respK[0].size() - respK[last].size() - 1, ' ') + "|";
    last = respD.size() - 1;

    respD[last] = respD[last].substr(0, respD[last].size() - 2);
    if (last == 1)
        respD[last] = respD[last].substr(0, respD[last].size() - 2);

    if (last != 1)
        respD[last - 1] += std::string(respD[0].size() - respD[last - 1].size() - 1, ' ') + "|";
    respD[last] += std::string(respD[0].size() - respD[last].size() - 1, ' ') + "|";
    std::string gap(44, '-');
    response.push_back("+" + gap + "+");
    for (std::string line : respK)
        response.push_back(line);
    response.push_back("+" + gap + "+");
    for (std::string line : respD)
        response.push_back(line);
    response.push_back("+" + gap + "+");

    return response;
}

std::vector<std::vector<int>> polyList;
std::vector<std::string> polyPrint() {
    std::vector<std::string> response;
    std::string formula = "Ж = ";
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
    response.push_back("+" + std::string(formula.size()-1, '-') + "+");
    response.push_back("| " + formula.substr(0, formula.size()-3) + " |");
    response.push_back("+" + std::string(formula.size()-1, '-') + "+");

    return response;
}