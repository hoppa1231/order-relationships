#include <iostream>
#include <tuple>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

// forward declaration
std::string alphabet(const std::unordered_map<char, std::string>& bs);


// Forward declaration (defined in AddModule.h)
std::string alphabet(const std::unordered_map<char, std::string>& bs);



class Table {
private:
    std::string string;
    int size;
    std::unordered_map<char, std::string> base;
    std::vector<std::string> road;
    int lenRoad;
    std::vector<std::vector<std::string>> plusTable;
    std::vector<std::vector<std::string>> minusTable;
    std::vector<std::vector<std::string>> mulTable;
    std::vector<std::vector<int>> answer;

public:
    Table(const std::unordered_map<char, std::string>& base, const std::vector<std::string>& road)
        : string(alphabet(base)), size((int)alphabet(base).size()), base(base), road(road), lenRoad(road.size()),
        plusTable(size, std::vector<std::string>(size, "#")),
        minusTable(size, std::vector<std::string>(size, "#")),
        mulTable(size, std::vector<std::string>(size, "#")) {
        generatePlusTable();
        generateMulTable();
        generateMinusTable();
    }

    std::string pomogi(const std::string& a, const std::string& b, char sign) {
        std::string answer = calc(a, b, sign);
        answer = replaceF(answer, "c", "c/f");
        if (std::count(answer.begin(), answer.end(), ',') > size - 1) {
            return "[ OVERFLOW ]";
        }
        return answer;
    }

    const std::vector<std::vector<std::string>>& getTable(char sign) const {
        if (sign == '+') return plusTable;
        if (sign == '-') return minusTable;
        if (sign == '*') return mulTable;
        throw std::invalid_argument("Invalid sign");
    }

    std::tuple<std::string, std::string, int, std::string, char> format(std::string a, std::string b, char sign) {
        bool altA = (a[0] == '-');
        bool altB = (b[0] == '-');

        if (altA) a = a.substr(1);
        if (altB) b = b.substr(1);

        std::string PorM = "";

        std::tuple<std::string, std::string, std::string> tmp;
        tmp = comparison(a, b);
        a = std::get<0>(tmp);
        b = std::get<1>(tmp);
        std::string comp = std::get<2>(tmp);

        if ((sign == '*' || sign == '/') && altA != altB) {
            PorM = "-";
        }
        else if (sign == '+' &&
            ((altA && altB) ||
                (altA && !altB && comp == ">") ||
                (!altA && altB && comp == "<"))) {
            PorM = "-";
        }
        else if (sign == '-' &&
            ((altA && !altB) ||
                (altA && altB && comp == ">") ||
                (!altA && !altB && comp == "<"))) {
            PorM = "-";
        }

        if (sign == '+' && altA != altB) {
            if (altA) {
                std::swap(a, b);
                sign = '-';
                comp = (comp == ">") ? "<" : ">";
            }
            else {
                sign = '-';
            }
        }
        else if (sign == '-' && (altA && !altB || !altA && altB)) {
            sign = '+';
        }
        if (sign == '-' && comp == "<") {
            std::swap(a, b);
        }
        if (sign == '*' && a.find_first_not_of('a') == std::string::npos) {
            std::swap(a, b);
        }
        return { a, b, static_cast<int>(a.size()), PorM, sign };
    }

    int overflowF(const char& a, const char& b, char sign) {
        int result = 0;
        if (sign == '+') {
            result = roadFind(a) + roadFind(b);
        }
        else if (sign == '-') {
            result = roadFind(a) - roadFind(b);
        }
        else if (sign == '*') {
            result = roadFind(a) * roadFind(b);
        }
        if (result < 0)
            return result / lenRoad - 1;
        return result / lenRoad;
    }
    std::string mul(int x, int y) const {
        return mulTable[y][x];
    }

    std::string addList(const std::vector<std::string>& array) {
        std::string first = array[0];
        for (size_t i = 1; i < array.size(); ++i) {
            std::string second = array[i];
            first = add(first, second);
        }
        return first;
    }

    std::string add(const std::string& x, const std::string& y) {
        char xChar = (x.size() > 1) ? x[0] : x[0];
        char yChar = (y.size() > 1) ? y[0] : y[0];
        int xIndex = string.find(xChar);
        int yIndex = string.find(yChar);
        return plusTable[yIndex][xIndex];
    }

    void splitMul(int i, int j, int available) {
        char chY = string[i];
        int x, y;
        auto pair = findSum(chY);
        x = pair.first;
        y = pair.second;

        if (x == 0 || y == 0) {
            std::cerr << "ERROR: Zero" << std::endl;
            exit(1);
        }

        for (int obj : {x, y}) {
            if (obj <= available) {
                answer.push_back({ obj, j });
            }
            else {
                splitMul(obj, j, available);
            }
        }
    }

    std::pair<int, int> findSum(char ch) {
        for (int x = 1; x < size; ++x) {
            for (int y = 1; y < size; ++y) {
                if (plusTable[y][x].find(ch) != std::string::npos) {
                    return { x, y };
                }
            }
        }
        return { 0, 0 };
    }

    int roadFind(char ch) const {
        for (int i = 0; i < size; i++) {
            if (road[i].find(ch) != std::string::npos) {
                return i;
            }
        }
        return -1;
    }

    int dist(char ch) const {
        return lenRoad - roadFind(ch);
    }

    std::vector<std::string> shift(int x) const {
        int size = road.size();
        x = (x % size + size) % size;
        std::vector<std::string> part1(road.begin() + size - x, road.end());
        std::vector<std::string> part2(road.begin(), road.begin() + size - x);

        part1.insert(part1.end(), part2.begin(), part2.end());
        return part1;
    }
    std::string formatRes(const std::string& res) const {
        bool altRes = (res[0] == '-');
        std::string result = (altRes ? res.substr(1) : res);
        if (std::count(result.begin(), result.end(), 'a') == static_cast<int>(result.size())) {
            return "a";
        }
        while (!result.empty() && result[0] == 'a') {
            result = result.substr(1);
        }
        return altRes ? "-" + result : result;
    }

    void printTable(char type) const {
        std::vector<std::vector<std::string>> array2d;
        std::vector<std::string> line;
        line.push_back(std::string(1, type));
        for (int i = 0; i < string.size(); i++)
            line.push_back(std::string(1, string[i]));
        array2d.push_back(line);

        const auto& table = getTable(type);
        for (size_t indx = 0; indx < table.size(); ++indx) {
            std::vector<std::string> line = { std::string(1, string[indx]) };
            for (const auto& cell : table[indx]) {
                line.push_back(cell);
            }
            array2d.push_back(line);
        }
        nicePrint(array2d);
    }

    void generatePlusTable() {
        for (size_t indx = 0; indx < string.size(); ++indx) {
            char ch = string[indx];
            int shift1 = dist(ch);
            std::vector<std::string> new_road = shift(shift1);

            for (int i = 0; i < size; ++i) {
                plusTable[indx][i] = new_road[roadFind(string[i])];
            }
        }

        for (const auto& el : road) {
            if (el.size() == 3) {
                plusTable[string.find(el[0])][0] = el[0];
                plusTable[string.find(el[2])][0] = el[2];
                plusTable[0][string.find(el[0])] = el[0];
                plusTable[0][string.find(el[2])] = el[2];
            }
        }
    }

    void generateMulTable() {
        mulTable[0] = std::vector<std::string>(size, string.substr(0, 1));
        mulTable[1] = std::vector<std::string>(string.size());
        for (size_t i = 0; i < string.size(); ++i) {
            mulTable[1][i] = std::string(1, string[i]);
        }

        for (size_t i = 0; i < size; ++i) {
            mulTable[i][0] = string.substr(0, 1);
            mulTable[i][1] = string.substr(i, 1);
        }

        int available = 1;

        for (size_t i = 2; i < size; ++i) {
            for (size_t j = 2; j < size; ++j) {
                answer.clear();
                splitMul(i, j, available);

                std::vector<std::string> arrayAdds;
                for (int u = 0; u < answer.size(); u++) {
                    int x = answer[u][0];
                    int y = answer[u][1];
                    arrayAdds.push_back(mul(x, y));
                }

                mulTable[i][j] = addList(arrayAdds);
            }
        }
    }

    void generateMinusTable() {
        for (size_t indx = 0; indx < string.size(); ++indx) {
            for (size_t jndx = 0; jndx < string.size(); ++jndx) {
                int dist = abs(roadFind(string[jndx]) - roadFind(string[indx]));
                minusTable[indx][jndx] = road[dist];
            }
        }
        for (const auto& el : road) {
            if (el.size() == 3) {
                minusTable[string.find(el[0])][0] = el[0];
                minusTable[string.find(el[2])][0] = el[2];
                minusTable[0][string.find(el[0])] = el[0];
                minusTable[0][string.find(el[2])] = el[2];
            }
        }
    }

    std::string replaceF(const std::string& text, const std::string& str1, const std::string& str2) {
        std::string result = text;
        size_t pos = 0;

        while ((pos = result.find(str1, pos)) != std::string::npos) {
            result.replace(pos, str1.length(), str2);
            pos += str2.length();
        }

        return result;
    }

    std::string normalFormat(const std::string s) {
        return replaceF(replaceF(replaceF(s, "c/f", "c"), ",", ""), "-", "");
    }
    std::string minusFunc(const std::string& x, const std::string& y) {
        return normalFormat(calc(x, y, '-'));
    }

    std::pair<std::string, std::string> div(const std::string& a, const std::string& b) {
        bool altA = (a[0] == '-');
        bool altB = (b[0] == '-');
        std::string aCopy = (altA ? a.substr(1) : a);
        std::string bCopy = (altB ? b.substr(1) : b);

        std::tuple<std::string, std::string, std::string> trio;
        trio = comparison(aCopy, bCopy);
        std::string aNorm = std::get<0>(trio);
        std::string bNorm = std::get<1>(trio);
        std::string comp = std::get<2>(trio);

        std::string nAnB = ((altA && altB) ? "-" : "");
        std::string AorB = ((!altA && altB) ? "-" : "");

        std::string res = aNorm;
        std::string count = "a";

        if (std::count(bCopy.begin(), bCopy.end(), 'a') == bCopy.size()) {
            return { std::string(8, road.back()[0]), std::string(8, road.back()[0]) };
        }
        if (std::count(aCopy.begin(), aCopy.end(), 'a') == aCopy.size()) {
            return { "a", "a" };
        }

        if (!altA == altB && comp == ">") {
            while (std::get<2>(comparison(res, bNorm)) != "<") {
                res = minusFunc(res, bNorm);
                count = increment(count);
            }
            if (res.empty()) {
                return { count, "a" };
            }
            return { "-" + count, res };
        }


        if (altA == altB) {
            if (comp == "<") {
                return { "a", nAnB + aNorm };
            }
            else {
                while (std::get<2>(comparison(res, bNorm)) != "<") {
                    res = minusFunc(res, bNorm);
                    count = increment(count);
                }
                if (res.empty()) {
                    return { count, "a" };
                }
                return { count, nAnB + res };
            }
        }
        else {
            if (comp == "<") {
                return { "-b", AorB + minusFunc(res, bNorm) };
            }
            else {
                res = aNorm;
                count = "a";
                while (std::get<2>(comparison(res, bNorm)) != "<") {
                    res = minusFunc(res, bNorm);
                    count = increment(count);
                }
                if (res.empty()) {
                    return { "-" + count, "a" };
                }
                res = minusFunc(res, bNorm);
                count = increment(count);
                return { "-" + count, AorB + res };
            }
        }
    }

    std::string increment(const std::string& a) const {
        std::string res;
        int overflow = 1;

        for (int i = a.size() - 1; i >= 0; --i) {
            int tempN = roadFind(a[i]) + overflow;
            if (tempN == lenRoad) {
                tempN = 0;
                overflow = 1;
                res = road[tempN] + res;
            }
            else {
                overflow = 0;
                res = a.substr(0, i) + road[tempN][0] + res;
                break;
            }
        }
        if (overflow == 1) {
            res = road[1] + res;
        }

        return res;
    }

    std::tuple<std::string, std::string, std::string> comparison(std::string a, std::string b) const {
        std::string comp = "=";
        size_t lenA = a.size(), lenB = b.size();

        if (lenA > lenB) {
            b = std::string(lenA - lenB, string[0]) + b;
        }
        else if (lenA < lenB) {
            a = std::string(lenB - lenA, string[0]) + a;
        }

        for (size_t i = 0; i < a.size(); ++i) {
            int numA = roadFind(a[i]);
            int numB = roadFind(b[i]);

            if (numA > numB) {
                comp = ">";
                break;
            }
            else if (numA < numB) {
                comp = "<";
                break;
            }
        }

        return { a, b, comp };
    }
    std::string calc(const std::string& a, const std::string& b, char sign) {
        std::string result = "";

        if (sign == '/') {
            auto pair = div(a, b);
            std::string integer = pair.first;
            std::string res = pair.second;

            // NUMBER DIV ZERO

            if (std::count(a.begin(), a.end(), 'a') != a.size() &&
                std::count(b.begin(), b.end(), 'a') == b.size()) {
                return "empty";
            }

            ;
            // ZERO DIV ZERO
            /*
            if (std::count(b.begin(), b.end(), 'a') == 8 &&
                std::count(a.begin(), a.end(), 'a') == 8) {
                return "[ ???????????????? ]";
            }
            */

            if (std::count(integer.begin(), integer.end(), 'h') == 8 &&
                std::count(res.begin(), res.end(), 'h') == 8) {
                return "[-hhhhhhhh; hhhhhhhh]";
            }
            return "(" + formatRes(integer) + ", " + formatRes(res) + ")";
        }

        if (a.empty() || b.empty()) {
            return "";
        }

        std::tuple<std::string, std::string, int, std::string, char> quintet;
        quintet = format(a, b, sign);
        std::string formattedA = std::get<0>(quintet);
        std::string formattedB = std::get<1>(quintet);
        int n = std::get<2>(quintet);
        std::string PorM = std::get<3>(quintet);
        char formattedSign = std::get<4>(quintet);

        bool justMul = false;

        if (formattedSign == '*') {
            std::string tempB = formattedB;
            while (!tempB.empty() && tempB[0] == 'a') {
                tempB = tempB.substr(1);
            }
            if (tempB.size() <= 1) {
                formattedB = std::string(n, formattedB.back());
                justMul = true;
            }
        }

        if (formattedB.find_first_not_of('a') == std::string::npos && (formattedSign == '+' || formattedSign == '-')) {
            return PorM + replaceF(formattedA, "f", "c");
        }

        int overflow = 0;
        std::string overflow_ = "a";

        for (int i = n - 1; i >= 0; i--) {
            if (formattedSign == '*' && !justMul) {
                std::string tempResult = calc(formattedA, std::string(1, formattedB[i]), '*');
                if (tempResult.empty()) continue;

                tempResult = normalFormat(tempResult) + repeat("a", n - i - 1);
                overflow_ = calc(tempResult, normalFormat(overflow_), '+');
                continue;
            }
            int x = string.find(formattedA[i]);
            int y = string.find(formattedB[i]);

            std::string first_number = getTable(formattedSign)[x][y];
            int first_overflow = overflowF(formattedA[i], formattedB[i], formattedSign);

            char z = road[std::abs(overflow)][0];

            std::string number;
            if (first_overflow == -1) {
                number = road[lenRoad - roadFind(first_number[0])];
            }
            else if (first_overflow >= 0) {
                number = first_number;
            }

            if (overflow >= 0) {
                std::string number_x =
                    getTable('+')[string.find(number[0])][string.find(z)];
                overflow = first_overflow + overflowF(number[0], z, '+');
                number = number_x;
            }
            else {
                if (number == "a" && first_overflow == 0 && overflow < 0) {
                    number = "h";
                    overflow = -1;
                }
                else {
                    number = getTable('-')[string.find(number[0])][string.find(z)];
                    overflow = first_overflow;
                }
            }

            result = number + "," + result;
        }

        if (overflow > 0) {
            result = road[overflow] + "," + result;
        }

        if (overflow_ != "a") {
            return PorM + overflow_;
        }

        while (!result.empty() && result[0] == ',') {
            result = result.substr(1);
        }

        while (result.substr(0, 2) == "a,") {
            result = result.substr(2);
        }

        return PorM + result.substr(0, result.size() - 1);
    }
    static void nicePrint(const std::vector<std::vector<std::string>>& array2d) {
        const int width = 5;
        const char p = '+';
        int size = array2d.size();
        std::string gap = p + std::string((width + 1) * size, '-') + p;

        for (const auto& row : array2d) {
            std::cout << gap << std::endl;
            std::cout << "|";
            for (const auto& el : row) {
                std::string elStr = el;
                std::string space = repeat(" ", width - elStr.size() - 1);
                std::cout << space << elStr << " |";
            }
            std::cout << std::endl;
        }
        std::cout << gap << std::endl;
    }
    static std::string repeat(const std::string& str, int times) {
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