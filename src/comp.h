#ifndef COMP_H
#define COMP_H
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

std::vector<std::string> default_block_comp = {
    " ~ Make computations ~",
    "+----------------------------+",
    "| Operand 1: _               |",
    "| Operand 2:                 |",
    "| Operation:                 |",
    "| [+,-,*,/]                  |",
    "| Result:                    |",
    "+----------------------------+"
};

class BlockComp {
private:
    int maxLengthOperand = 8;
    std::vector<std::string> block = default_block_comp;
    std::string operand1;
    std::string operand2;
    std::string operation;
    std::string result;

    int cursor = 0; // 1 - operand1 | 2 - operand2 | 3 - operation | 4 - result

public:
    BlockComp() {
        print();
        nextCursor();
        getCursor();
        getOperand1();
        getOperand2();
        getOperation();
    };

    std::vector<std::string> print() {

        block[2].replace(13, 18, _curs(operand1, 1));
        block[3].replace(13, 18, _curs(operand2, 2));
        block[4].replace(13, 18, _curs(operation, 3));
        block[6].replace(10, 20, _curs(result, 4));

        return block;
    }

    void setOperation(char value) {
        std::string allow = "*+-/";
        if (allow.find(value) != std::string::npos) {
            operation = std::string{ value };
        }
    }
    void setResult(std::string value) {
        result = value;
    }

    void addOperand2(char value) {
        if (operand2.length() > maxLengthOperand) return;
        operand2 += std::string{ value };
    }

    void addOperand1(char value) {
        if (operand1.length() > maxLengthOperand) return;
        operand1 += std::string{ value };
    }
    void popOperand1() { if (!operand1.empty()) operand1.pop_back(); }

    void popOperand2() { if (!operand2.empty()) operand2.pop_back(); }

    int getCursor() {
        return cursor;
    }
    std::string getOperand1() {
        return operand1;
    }
    std::string getOperand2() {
        return operand2;
    }
    char getOperation() {
        return operation[0];
    }

    int nextCursor() {
        cursor += 1;
        if (cursor == 4) {
            return 1;
        }
        else if (cursor == 5) {
            resetBlock();
        }
        return 0;
    }

    void resetBlock() {
        cursor = 1;
        result = "";
        operand1 = "";
        operand2 = "";
        operation = "";
        block = default_block_comp;
    }

    std::string _curs(std::string s, int curs) {
        std::string tmp;
        if (curs == 3) {
            if (operation != "")
                return s + _repeat(" ", 16 - s.length()) + "|";
            else if (curs != cursor)
                return s + _repeat(" ", 16 - s.length()) + "|";
            else
                return "_" + _repeat(" ", 15 - s.length()) + "|";
        }
        else if (curs == 2 || curs == 1) {
            tmp = (curs == cursor) ? s + "_" + _repeat(" ", 15 - s.length()) + "|" : s + _repeat(" ", 16 - s.length()) + "|";
            return tmp;
        }
        else if (curs == 4) {
            return s + _repeat(" ", 19 - s.length()) + "|";
        }
        return std::string();
    }

    std::string _repeat(const std::string& str, int times) {
        if (times <= 0) {
            return "";
        }

        std::string result;
        result.reserve(str.size() * times);

        for (int i = 0; i < times; ++i) {
            result += str;
        }

        return result;
    }

};

#endif // COMP_H
