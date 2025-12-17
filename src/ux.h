#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <conio.h>
#include <cmath>
#include "MainModule.h"
#include "AddModule.h"
#include "comp.h"
#include "Table.h"
#include <set>
#include <map>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <tuple>
#include <memory>

// ==================== ОБЪЯВЛЕНИЯ ФУНКЦИЙ ====================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
std::vector<char> branch_list(std::string branch);
std::string alphabet(std::unordered_map<char, std::string>& bs);
std::unordered_map<char, size_t> calc_road_size(std::unordered_map<char, std::string>& bs);
std::unordered_map<char, std::string> copying_map(std::unordered_map<char, std::string>& bs);
void update_base();
void update_clone();
void childs_format(std::string& childs);
std::string erase_childs(std::string all_childs, std::vector<char>& child_to_delete);
void update_inverted_dist();

// ФУНКЦИИ ВАЛИДАЦИИ
std::vector<char> find_neighbours(char object);
std::vector<char> get_childs(std::vector<char>& parents);
bool find_gate(char parent, char child);
int valid_insert_vertex(insert_data data);
int valid_insert(std::vector<insert_data> ins_d);
bool simulateDeleteWithPullUp(const std::vector<std::string>& del);

// ФУНКЦИИ ОПЕРАЦИЙ
void replace_vertex_base_clone(char object, char parent, std::vector<char>& childs_list, std::string childs_in_string);
void insert_vertex_base_clone(insert_data data);
void insert_base_clone(std::vector<insert_data> all_data);
size_t clearing_part_of_layer(std::vector<char>& objects_to_del);
void clearing_all_layer(std::vector<char>& objects_to_del);
size_t deleting_object(std::string obj, bool layer_clear);
size_t deleting_data(std::vector<std::string> delete_data);

// ОБЩИЕ ФУНКЦИИ
void buildBase();
std::unordered_map<char, std::string> buildInvertedBase(std::unordered_map<char, std::string>& bs);
void buildHasseStructure();
void buildHasseDiagram();
std::string buildTaskString();
int getMaxLevel();

// ФУНКЦИИ ДЛЯ ИНТЕГРАЦИИ С РЕДАКТОРОМ
bool canInsertVertex(char newVertex, char parent, const std::string& childs);
bool canDeleteVertices(const std::vector<std::string>& vertices);
void executeInsert(char newVertex, char parent, const std::string& childs);
bool executeDelete(const std::string& vertex);

// СТАНДАРТНЫЕ ФУНКЦИИ
std::string inputCorrect(const std::string& whiteList, const std::string& comment);
std::string textStart();
std::tuple<std::string, std::string, char> textGetVal();
void initialize();

// Рекурсивная структура для веток
struct BranchNode;
using BranchNodePtr = std::shared_ptr<BranchNode>;
using BranchChain = std::vector<BranchNodePtr>;


static std::string ux_getCursorElement();
void parseTaskToRelation();
static void ux_navigateGraphArrow(int key);
static void ux_drawDeleteOrAddGraph(const std::string& title,
    const std::unordered_set<std::string>* marked,
    const std::string& footerLine1,
    const std::string& footerLine2,
    const std::string& message);


struct BranchNode {
    std::string element;
    std::vector<std::vector<BranchNodePtr>> subBranches; // Ветки от этого элемента
    explicit BranchNode(std::string el = "") : element(std::move(el)) {}
};


struct RelationStructure {
    std::vector<std::string> mainPath;
    std::map<int, std::vector<BranchChain>> branches; // branches[позиция] = список веток
    std::set<std::string> allElements;
};

RelationStructure currentRelation;
// Функция для проверки валидности графа (все листья на одинаковом расстоянии)
struct ValidationResult {
    bool isValid;
    std::string errorMessage;
    std::vector<std::pair<char, size_t>> invalidLeaves; // вершина и её расстояние
    size_t maxDistance;
};
// ===== Helpers: build base_clone from editor RelationStructure and parse base_clone back =====
static inline int ux_key(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return 26 + (c - 'A');
    return 1000 + (unsigned char)c;
}
static inline bool ux_less(char a, char b) { return ux_key(a) < ux_key(b); }

static inline std::string ux_strip_separators(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) if (c != '/' && c != ' ' && c != '\t' && c != '\r' && c != '\n') out.push_back(c);
    return out;
}

static void ux_edges_from_chain(char parent, const BranchChain& chain,
    std::unordered_map<char, std::set<char>>& adj,
    std::set<std::string>& all) {
    if (parent) {
        all.insert(std::string(1, parent));
        adj.try_emplace(parent, std::set<char>{});
    }
    for (size_t i = 0; i < chain.size(); ++i) {
        if (chain[i]->element.empty()) continue;
        char u = chain[i]->element[0];
        all.insert(std::string(1, u));
        adj.try_emplace(u, std::set<char>{});
        if (i == 0 && parent) adj[parent].insert(u);
        if (i + 1 < chain.size() && !chain[i + 1]->element.empty()) {
            adj[u].insert(chain[i + 1]->element[0]);
        }
        for (const auto& sub : chain[i]->subBranches) {
            ux_edges_from_chain(u, sub, adj, all);
        }
    }
}

static std::unordered_map<char, std::string> ux_base_from_relation(const RelationStructure& rel) {
    std::unordered_map<char, std::set<char>> adj;
    std::set<std::string> all = rel.allElements;

    // ensure vertices
    for (const auto& s : all) if (!s.empty()) adj.try_emplace(s[0], std::set<char>{});

    // main path edges
    for (size_t i = 0; i + 1 < rel.mainPath.size(); ++i) {
        if (rel.mainPath[i].empty() || rel.mainPath[i + 1].empty()) continue;
        char u = rel.mainPath[i][0], v = rel.mainPath[i + 1][0];
        adj[u].insert(v);
        adj.try_emplace(v, std::set<char>{});
        all.insert(std::string(1, u));
        all.insert(std::string(1, v));
    }

    // branches from main path
    for (const auto& kv : rel.branches) {
        int pos = kv.first;
        if (pos < 0 || pos >= (int)rel.mainPath.size()) continue;
        if (rel.mainPath[pos].empty()) continue;
        char parent = rel.mainPath[pos][0];
        for (const auto& chain : kv.second) ux_edges_from_chain(parent, chain, adj, all);
    }

    // serialize
    std::vector<char> verts;
    verts.reserve(adj.size());
    for (auto& p : adj) verts.push_back(p.first);
    std::sort(verts.begin(), verts.end(), ux_less);

    std::unordered_map<char, std::string> out;
    for (char u : verts) {
        auto it = adj.find(u);
        std::vector<char> kids;
        if (it != adj.end()) kids.assign(it->second.begin(), it->second.end());
        std::sort(kids.begin(), kids.end(), ux_less);
        std::string s;
        for (size_t i = 0; i < kids.size(); ++i) {
            if (i) s.push_back('/');
            s.push_back(kids[i]);
        }
        out[u] = s;
    }
    // include leaves too
    for (const auto& s : all) if (!s.empty()) out.try_emplace(s[0], std::string{});
    return out;
}

static char ux_select_root(const std::unordered_map<char, std::string>& bs) {
    if (bs.empty()) return 'a';
    if (bs.find('a') != bs.end()) return 'a';
    std::unordered_map<char, int> indeg;
    for (auto& kv : bs) indeg.try_emplace(kv.first, 0);
    for (auto& kv : bs) {
        std::string kids = ux_strip_separators(kv.second);
        for (char c : kids) indeg[c]++; // create if missing
    }
    char best = 0;
    for (auto& kv : indeg) if (kv.second == 0) {
        if (!best || ux_less(kv.first, best)) best = kv.first;
    }
    if (best) return best;
    for (auto& kv : bs) if (!best || ux_less(kv.first, best)) best = kv.first;
    return best ? best : 'a';
}

static void ux_build_chain(char start,
    const std::unordered_map<char, std::string>& bs,
    BranchChain& out,
    std::unordered_set<char>& onPath) {
    char cur = start;
    while (true) {
        if (onPath.count(cur)) break; // cycle guard
        onPath.insert(cur);

        BranchNodePtr node = std::make_shared<BranchNode>(std::string(1, cur));
        auto it = bs.find(cur);
        std::string kids = (it == bs.end()) ? "" : ux_strip_separators(it->second);
        if (!kids.empty()) {
            // first child continues the chain
            char mainChild = kids[0];
            // other children are sub-branches
            for (size_t i = 1; i < kids.size(); ++i) {
                BranchChain sub;
                std::unordered_set<char> subPath = onPath;
                ux_build_chain(kids[i], bs, sub, subPath);
                node->subBranches.push_back(std::move(sub));
            }
            out.push_back(std::move(node));
            cur = mainChild;
            continue;
        }
        out.push_back(std::move(node));
        break;
    }
}


ValidationResult validateRelationDistances() {
    ValidationResult res;
    res.isValid = true;
    res.maxDistance = 0;

    // build a temporary base from the editor structure and compute distances to leaves
    auto bs = ux_base_from_relation(currentRelation);
    if (bs.empty()) return res;
    char root = ux_select_root(bs);

    // BFS for distances
    std::unordered_map<char, int> dist;
    std::vector<char> q;
    q.push_back(root);
    dist[root] = 0;
    for (size_t qi = 0; qi < q.size(); ++qi) {
        char u = q[qi];
        std::string kids = ux_strip_separators(bs[u]);
        for (char v : kids) {
            if (!dist.count(v)) {
                dist[v] = dist[u] + 1;
                q.push_back(v);
            }
        }
    }

    // compute leaves and max distance
    std::set<char> hasChildren;
    for (auto& kv : bs) {
        std::string kids = ux_strip_separators(kv.second);
        if (!kids.empty()) hasChildren.insert(kv.first);
    }
    for (auto& kv : dist) if (kv.second > res.maxDistance) res.maxDistance = kv.second;
    if (res.maxDistance == 0) return res;

    for (auto& kv : dist) {
        char v = kv.first;
        if (hasChildren.find(v) == hasChildren.end()) {
            if (kv.second < res.maxDistance) {
                res.isValid = false;
                res.invalidLeaves.push_back({ v, kv.second });
            }
        }
    }
    if (!res.isValid) {
        res.errorMessage = "Invalid graph: some leaves are closer than the deepest leaf";
    }
    return res;
}
int cursorPosition = 0;
int cursorLevel = 0; // 0 - main path, 1,2,3... - branch levels
std::string tempInput = "";
int editMode = 0; // 0-none, 1-insert, 2-delete, 3-modify
int insertStep = 0; // 0-parent, 1-vertex, 2-child
std::string insertParent = "";
std::string insertVertex = "";
std::string insertChild = "";
int deletePosition = -1;
int branchCursor = 0; // For branch navigation
int branchIndex = 0; // Which branch we're in
int subBranchLevel = 0;      // Уровень вложенности в ветке (0 - сама ветка, 1 - подветка и т.д.)
int subBranchIndex = 0;      // Индекс подветки
int subBranchCursor = 0;     // Позиция в подветке

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


    auto printLineWithMarkers = [&](const std::string& line) {
        bool red = false;
        for (char c : line) {
            if (c == '\001') { setTextColor(12, 0); red = true; std::cout << '['; continue; }
            if (c == '\002') { std::cout << ']'; resetColors(); red = false; continue; }
            std::cout << c;
        }
        if (red) resetColors();
        };

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
                char c = block[i][j];
                if (c == '\001' || c == '\002') continue;
                if (c == '|') {
                    std::cout << c;
                }
                else {
                    setTextColor(0, 15);
                    std::cout << c;
                    resetColors();
                }
            }
        }
        else {
            printLineWithMarkers(block[i]);
        }
        std::cout << std::endl;
    }
}

// ==================== Console helpers ====================
static int ux_console_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

static int ux_console_height() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
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

void drawHasseDiagram() {
    system("cls");

    initialize();

    std::cout << std::endl;
    std::cout << "        HASSE DIAGRAM" << std::endl;
    std::cout << "        =============" << std::endl << std::endl;

    setTextColor(11, 0);
    std::cout << "  Order relation: ";
    setTextColor(14, 0);
    std::cout << task << std::endl;

    setTextColor(11, 0);
    std::cout << "  Alphabet: ";
    setTextColor(14, 0);
    std::cout << alphabet(base_clone.empty() ? base : base_clone) << std::endl << std::endl;
    resetColors();

    int maxLevel = getMaxLevel();

    std::vector<std::vector<char>> levels(maxLevel + 1);
    for (const auto& node : hasseNodes) {
        levels[node.level].push_back(node.symbol);
    }

    for (int level = maxLevel; level >= 0; level--) {
        setTextColor(10, 0);
        std::cout << "  Level " << level << ": ";
        resetColors();

        for (char symbol : levels[level]) {
            setTextColor(14, 0);
            std::cout << symbol << "   ";
            resetColors();
        }
        std::cout << std::endl;

        if (level > 0) {
            std::cout << "            ";

            for (char symbol : levels[level]) {
                auto it = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                    [symbol](const HasseNode& n) { return n.symbol == symbol; });

                if (it != hasseNodes.end() && !it->parents.empty()) {
                    setTextColor(12, 0);
                    std::cout << " |   ";
                    resetColors();
                }
                else {
                    std::cout << "     ";
                }
            }
            std::cout << std::endl;

            std::cout << "            ";
            for (char symbol : levels[level]) {
                auto it = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                    [symbol](const HasseNode& n) { return n.symbol == symbol; });

                if (it != hasseNodes.end() && !it->parents.empty()) {
                    setTextColor(12, 0);
                    std::cout << " |   ";
                    resetColors();
                }
                else {
                    std::cout << "     ";
                }
            }
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    setTextColor(11, 0);
    std::cout << "  CONNECTION STRUCTURE:" << std::endl;
    resetColors();
    std::cout << "  --------------------" << std::endl;

    for (int level = maxLevel; level >= 0; level--) {
        bool hasConnections = false;

        for (char symbol : levels[level]) {
            auto it = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                [symbol](const HasseNode& n) { return n.symbol == symbol; });

            if (it != hasseNodes.end() && !it->children.empty()) {
                if (!hasConnections) {
                    setTextColor(10, 0);
                    std::cout << "  Level " << level << ": ";
                    resetColors();
                    hasConnections = true;
                }

                setTextColor(14, 0);
                std::cout << it->symbol << " -> {";
                for (size_t i = 0; i < it->children.size(); i++) {
                    std::cout << it->children[i];
                    if (i < it->children.size() - 1) std::cout << ",";
                }
                std::cout << "} ";
                resetColors();
            }
        }

        if (hasConnections) {
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
    setTextColor(11, 0);
    std::cout << "  INCOMPARABLE ELEMENTS:" << std::endl;
    resetColors();
    std::cout << "  ---------------------" << std::endl;

    bool hasIncomparable = false;
    for (const auto& node : hasseNodes) {
        if (!node.children.empty() && node.children.size() > 1) {
            std::vector<char> sameLevelChildren;
            for (char child : node.children) {
                auto childIt = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                    [child](const HasseNode& n) { return n.symbol == child; });
                if (childIt != hasseNodes.end() && childIt->level == node.level + 1) {
                    sameLevelChildren.push_back(child);
                }
            }

            if (sameLevelChildren.size() > 1) {
                hasIncomparable = true;
                setTextColor(14, 0);
                std::cout << "  " << node.symbol << " -> {";
                for (size_t i = 0; i < sameLevelChildren.size(); i++) {
                    std::cout << sameLevelChildren[i];
                    if (i < sameLevelChildren.size() - 1) std::cout << " and ";
                }
                std::cout << "} incomparable" << std::endl;
                resetColors();
            }
        }
    }

    if (!hasIncomparable) {
        std::cout << "  No incomparable elements (linear order)" << std::endl;
    }

    std::cout << std::endl;
    setTextColor(11, 0);
    std::cout << "  PATH FROM BOTTOM TO TOP (road):" << std::endl;
    resetColors();
    std::cout << "  -------------------------------" << std::endl;
    std::cout << "  ";

    setTextColor(14, 0);
    for (size_t i = 0; i < road.size(); i++) {
        std::cout << road[i];
        if (i < road.size() - 1) {
            std::cout << " -> ";
        }
    }
    resetColors();

    std::cout << std::endl << std::endl;
    setTextColor(11, 0);
    std::cout << "  STATISTICS:" << std::endl;
    resetColors();
    std::cout << "  -----------" << std::endl;
    std::cout << "  Total elements: " << hasseNodes.size() << std::endl;
    std::cout << "  Total levels: " << maxLevel + 1 << std::endl;
    std::cout << "  Minimal element: " << "a" << std::endl;

    std::cout << "  Maximal elements: ";
    std::vector<char> maximalElements;
    for (const auto& node : hasseNodes) {
        if (node.children.empty()) {
            maximalElements.push_back(node.symbol);
        }
    }

    setTextColor(14, 0);
    for (size_t i = 0; i < maximalElements.size(); i++) {
        std::cout << maximalElements[i];
        if (i < maximalElements.size() - 1) std::cout << ", ";
    }
    resetColors();

    std::cout << std::endl << std::endl;
    setTextColor(15, 0);
    std::cout << "  (Press any key to continue...)" << std::endl;
    resetColors();
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
    if (type == "BDR")
        response = getBDR(A, B, C, D);
    else if (type == "SDNF")
        for (std::vector<int> el : sdnfList) {
            unsigned int tempRes = 1;
            for (int i = 0; i < 4; i++)
                tempRes *= values[i] + el[i];
            if (response == 0) { response = 0; }
            response = response or tempRes;
        }
    else if (type == "POLY") {
        for (std::vector<int> el : polyList) {
            int tempRes = 1;
            for (int i = 0; i < 4; i++)
                if (el[i]) { tempRes *= values[i]; }
            if (response == 0) { response = tempRes; }
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
        temp.push_back("Enter A B C D");
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

Table* matrixTable = nullptr;

static std::vector<std::string> ux_buildRelationGraphBlock(
    const std::string& title,
    bool drawCursor,
    bool drawTempInput,
    const std::string& message = "",
    const std::string& help1 = "",
    const std::string& help2 = ""
) {
    const int WINDOW_WIDTH = max(80, ux_console_width() - 4);
    const int INNER_W = WINDOW_WIDTH - 2;

    auto makeLine = [&]() -> std::string { return "|" + std::string(INNER_W, ' ') + "|"; };
    auto frameTop = [&]() -> std::string { return "+" + std::string(INNER_W, '-') + "+"; };

    auto put = [&](std::string& line, int x, char c) {
        if (x < 0 || x >= INNER_W) return;
        char& cell = line[1 + x];
        if ((c == '|' || c == '-' || c == '+' || c == '>') && cell != ' ') return;
        cell = c;
        };
    auto putStr = [&](std::string& line, int x, const std::string& s) {
        for (int i = 0; i < (int)s.size(); ++i) put(line, x + i, s[i]);
        };
    auto token3 = [&](const std::string& el, bool isCursor) -> std::string {
        return isCursor ? ("[" + el + "]") : (" " + el + " ");
        };

    // центры токенов главного пути
    std::vector<int> mainCenterX;
    {
        int x = 2;
        for (size_t i = 0; i < currentRelation.mainPath.size(); ++i) {
            mainCenterX.push_back(x + 1);
            x += 3;
            if (i + 1 < currentRelation.mainPath.size()) x += 5;
        }
    }

    std::vector<std::string> block;
    block.push_back(frameTop());
    {
        std::string line = makeLine();
        int pad = (INNER_W - (int)title.size()) / 2;
        putStr(line, max(0, pad), title);
        block.push_back(line);
    }
    block.push_back(frameTop());

    // main path
    {
        std::string line = makeLine();
        putStr(line, 0, "  ");
        int x = 2;
        for (size_t i = 0; i < currentRelation.mainPath.size(); ++i) {
            const std::string& el = currentRelation.mainPath[i];
            bool isCur = drawCursor && (cursorLevel == 0 && (int)i == cursorPosition);
            putStr(line, x, token3(el, isCur));
            x += 3;
            if (i + 1 < currentRelation.mainPath.size()) {
                putStr(line, x, "---> ");
                x += 5;
            }
        }
        block.push_back(line);
    }

    // ветки любой глубины
    std::function<void(int, const BranchChain&, int, int)> renderChain;
    renderChain = [&](int parentCenterX, const BranchChain& chain, int branchPos, int branchIdx) {
        { std::string v = makeLine(); put(v, parentCenterX, '|'); block.push_back(v); }

        std::string line = makeLine();
        put(line, parentCenterX, '+');
        putStr(line, parentCenterX + 1, "---> ");
        int x = parentCenterX + 6;

        std::vector<int> centers; centers.reserve(chain.size());
        for (size_t i = 0; i < chain.size(); ++i) {
            const std::string& el = chain[i]->element;
            bool isCur = drawCursor && (cursorLevel > 0 && subBranchLevel == 0 &&
                cursorPosition == branchPos && (branchIndex - 1) == branchIdx &&
                (int)i == branchCursor);
            putStr(line, x, token3(el, isCur));
            centers.push_back(x + 1);
            x += 3;
            if (i + 1 < chain.size()) { putStr(line, x, "---> "); x += 5; }
        }

        // временный ввод — только если включено (в Hasse выключаем)
        if (drawTempInput &&
            cursorLevel > 0 && subBranchLevel == 0 &&
            cursorPosition == branchPos && (branchIndex - 1) == branchIdx) {
            if (!tempInput.empty()) {
                if (chain.empty()) putStr(line, x, "[" + tempInput + "]");
                else putStr(line, x, "---> [" + tempInput + "]");
            }
            else if (branchCursor >= (int)chain.size()) {
                if (chain.empty()) putStr(line, x, "[_]");
                else putStr(line, x, "---> [_]");
            }
        }

        block.push_back(line);

        for (size_t i = 0; i < chain.size(); ++i)
            for (const auto& sub : chain[i]->subBranches)
                renderChain(centers[i], sub, branchPos, branchIdx);
        };

    if (!currentRelation.branches.empty()) {
        std::vector<int> positions;
        for (const auto& kv : currentRelation.branches) positions.push_back(kv.first);
        std::sort(positions.begin(), positions.end());

        for (int pos : positions) {
            if (pos < 0 || pos >= (int)mainCenterX.size()) continue;
            const auto& chains = currentRelation.branches.at(pos);
            for (size_t bi = 0; bi < chains.size(); ++bi)
                renderChain(mainCenterX[pos], chains[bi], pos, (int)bi);
        }
    }

    block.push_back(frameTop());

    if (!help1.empty() || !help2.empty()) {
        auto clip = [&](const std::string& s) {
            return s.substr(0, min((int)s.size(), INNER_W - 2));
            };
        std::string l1 = makeLine(); putStr(l1, 1, clip(help1)); block.push_back(l1);
        std::string l2 = makeLine(); putStr(l2, 1, clip(help2)); block.push_back(l2);
        block.push_back(frameTop());
    }

    if (!message.empty()) {
        std::string msg = message;
        while ((int)msg.size() > INNER_W - 2) {
            std::string l = makeLine();
            putStr(l, 1, msg.substr(0, INNER_W - 2));
            block.push_back(l);
            msg = msg.substr(INNER_W - 2);
        }
        std::string l = makeLine();
        putStr(l, 1, msg);
        block.push_back(l);
        block.push_back(frameTop());
    }

    return block;
}

void printHasseDiagram() {
    parseTaskToRelation();
    task = buildTaskString();

    cursorPosition = 0;
    cursorLevel = 0;
    branchCursor = 0;
    branchIndex = 0;
    subBranchLevel = 0;
    subBranchIndex = 0;
    subBranchCursor = 0;

    while (true) {
        auto block = ux_buildRelationGraphBlock(
            "HASSE DIAGRAM",
            true,   // курсор показываем
            false,  // tempInput НЕ рисуем
            "",
            "Arrows: move cursor   ESC: back",
            ""
        );
        system("cls");
        printCenteredBlock(block, -1);

        char ch = _getch();
        if (ch == 27) return;
        if (ch == -32) {
            ch = _getch();
            ux_navigateGraphArrow((unsigned char)ch);
        }
    }
}


// Parse current task string to relation structure
void parseTaskToRelation() {
    currentRelation = RelationStructure();
    currentRelation.mainPath.clear();
    currentRelation.branches.clear();
    currentRelation.allElements.clear();

    const auto& bs = (!base_clone.empty()) ? base_clone : base;
    if (bs.empty()) {
        currentRelation.mainPath.push_back("a");
        currentRelation.allElements.insert("a");
        return;
    }

    // collect all elements
    for (const auto& kv : bs) currentRelation.allElements.insert(std::string(1, kv.first));
    for (const auto& kv : bs) {
        std::string kids = ux_strip_separators(kv.second);
        for (char c : kids) currentRelation.allElements.insert(std::string(1, c));
    }

    char root = ux_select_root(bs);

    // build main path following first child
    char cur = root;
    std::unordered_set<char> onPath;
    while (true) {
        if (onPath.count(cur)) break;
        onPath.insert(cur);
        currentRelation.mainPath.push_back(std::string(1, cur));
        auto it = bs.find(cur);
        if (it == bs.end()) break;
        std::string kids = ux_strip_separators(it->second);
        if (kids.empty()) break;
        char next = kids[0];
        // add other kids as branches
        int pos = (int)currentRelation.mainPath.size() - 1;
        for (size_t i = 1; i < kids.size(); ++i) {
            BranchChain chain;
            std::unordered_set<char> subPath = onPath;
            ux_build_chain(kids[i], bs, chain, subPath);
            currentRelation.branches[pos].push_back(std::move(chain));
        }
        cur = next;
    }

    // also attach branches for non-main vertices that still have side children are already handled in ux_build_chain recursion
}

// Convert structure back to task string
std::string buildTaskString() {
    if (currentRelation.mainPath.empty()) {
        return "";
    }

    std::string result = "";

    for (size_t i = 0; i < currentRelation.mainPath.size(); i++) {
        // Сначала добавляем текущий элемент основного пути
        result += currentRelation.mainPath[i];

        // Проверяем, есть ли ветки от этой позиции
        if (currentRelation.branches.find(i) != currentRelation.branches.end() &&
            !currentRelation.branches.at(i).empty()) {

            // Добавляем все ветки через /
            const auto& branchesAtPos = currentRelation.branches.at(i);
            for (const auto& branch : branchesAtPos) {
                result += "/";
                // Добавляем элементы ветки
                for (size_t j = 0; j < branch.size(); j++) {
                    result += branch[j]->element;

                    // Рекурсивно добавляем подветки (если есть)
                    // Для простоты пока игнорируем глубокую вложенность
                }
            }
        }

        // Пробел между элементами (кроме последнего)
        if (i < currentRelation.mainPath.size() - 1) {
            result += " ";
        }
    }

    return result;
}

// Рекурсивная функция для отрисовки ветки и её подветок
void drawBranchRecursive(const BranchChain& branch, int startCol,
    int depth, int parentBranchPos, int parentBranchIdx,
    int& lineCount, const int WINDOW_WIDTH) {
    if (branch.empty()) return;

    // Вертикальная линия
    std::cout << "|";
    for (int i = 0; i < startCol - 1; i++) std::cout << " ";
    std::cout << "|";
    int remaining = WINDOW_WIDTH - startCol - 2;
    for (int i = 0; i < remaining; i++) std::cout << " ";
    std::cout << "|" << std::endl;
    lineCount++;

    // Сама ветка
    std::cout << "|";
    for (int i = 0; i < startCol - 1; i++) std::cout << " ";
    std::cout << "+ ---> ";

    int col = startCol + 7;

    for (size_t k = 0; k < branch.size(); k++) {
        // Проверка подсветки
        bool isHighlighted = false;
        if (cursorLevel > 0 && parentBranchPos == cursorPosition &&
            parentBranchIdx == branchIndex - 1 && (int)k == branchCursor &&
            subBranchLevel == 0) {
            isHighlighted = true;
        }

        if (isHighlighted) {
            std::cout << "[" << branch[k]->element << "]";
        }
        else {
            std::cout << " " << branch[k]->element << " ";
        }
        col += 3;

        if (k < branch.size() - 1) {
            std::cout << "---> ";
            col += 5;
        }
    }

    // tempInput
    if (cursorLevel > 0 && parentBranchPos == cursorPosition &&
        parentBranchIdx == branchIndex - 1 && subBranchLevel == 0) {
        if (!tempInput.empty()) {
            std::cout << "---> [" << tempInput << "]";
            col += 8;
        }
        else if (branchCursor >= (int)branch.size()) {
            std::cout << "---> [_]";
            col += 8;
        }
    }

    remaining = WINDOW_WIDTH - col - 1;
    for (int i = 0; i < remaining; i++) std::cout << " ";
    std::cout << "|" << std::endl;
    lineCount++;

    // Рекурсивно рисуем подветки каждого элемента
    int subCol = startCol + 10 + depth * 2; // more spacing for deeper branches
    for (size_t k = 0; k < branch.size(); k++) {
        if (!branch[k]->subBranches.empty()) {
            for (size_t sb = 0; sb < branch[k]->subBranches.size(); sb++) {
                drawBranchRecursive(branch[k]->subBranches[sb], subCol, depth + 1,
                    parentBranchPos, parentBranchIdx, lineCount, WINDOW_WIDTH);
            }
        }
        subCol += 3; // Ширина элемента
        if (k < branch.size() - 1) {
            subCol += 5; // Ширина стрелки
        }
    }
}

void drawCreateRelationEditor(const std::string& message = "") {
    std::string h1 = "[ARROWS] move   [SPACE] add/confirm   [BKSP] delete   [/] branch";
    std::string h2;
    if (cursorLevel == 0) h2 = "[a-z] edit node   [DOWN] to branches   [ENTER] save   [ESC] cancel";
    else if (subBranchLevel == 0) h2 = "[a-z] add   [UP] to main   [DOWN] next branch   [ENTER] save   [ESC] cancel";
    else h2 = "[a-z] add   [UP] to parent branch   [ENTER] save   [ESC] cancel";

    auto block = ux_buildRelationGraphBlock("CREATE ORDER RELATION", true, true, message, h1, h2);
    system("cls");
    printCenteredBlock(block, -1);
}


// Draw edit relation editor
void drawEditRelationEditor(const std::string& message = "") {
    system("cls");

    // Wider window: legend + 3-line input area should fit.
    const int WINDOW_WIDTH = max(80, ux_console_width() - 4);

    std::cout << "+" << std::string(WINDOW_WIDTH - 2, '-') << "+" << std::endl;

    std::string title;
    if (editMode == 1) {
        title = "INSERT RELATION";
    }
    else if (editMode == 2) {
        title = "DELETE ORDER RELATION";
    }
    else if (editMode == 3) {
        title = "MODIFY ORDER RELATION";
    }
    else {
        title = "EDIT ORDER RELATION";
    }
    int padding = (WINDOW_WIDTH - 2 - title.length()) / 2;
    std::cout << "|" << std::string(padding, ' ') << title << std::string(WINDOW_WIDTH - 2 - padding - title.length(), ' ') << "|" << std::endl;
    std::cout << "+" << std::string(WINDOW_WIDTH - 2, '-') << "+" << std::endl;

    // Собираем позиции элементов
    std::vector<int> elementPositions;
    int col = 4;

    // Основной путь
    std::string mainLine = "  ";
    for (size_t i = 0; i < currentRelation.mainPath.size(); i++) {
        elementPositions.push_back(col);

        if (editMode == 2 && (int)i == deletePosition && cursorLevel == 0) {
            setTextColor(12, 0);
            mainLine += "[" + currentRelation.mainPath[i] + "]";
            // Нужно вывести до этого места, потом цвет, потом после
        }
        else if ((int)i == cursorPosition && cursorLevel == 0) {
            mainLine += "[" + currentRelation.mainPath[i] + "]";
        }
        else {
            mainLine += " " + currentRelation.mainPath[i] + " ";
        }
        col += 3;

        if (i < currentRelation.mainPath.size() - 1) {
            mainLine += "---> ";
            col += 5;
        }
    }

    // Вывод основного пути с подсветкой
    std::cout << "|  ";
    for (size_t i = 0; i < currentRelation.mainPath.size(); i++) {
        if (editMode == 2 && (int)i == deletePosition && cursorLevel == 0) {
            setTextColor(12, 0);
            std::cout << "[" << currentRelation.mainPath[i] << "]";
            resetColors();
        }
        else if ((int)i == cursorPosition && cursorLevel == 0) {
            std::cout << "[" << currentRelation.mainPath[i] << "]";
        }
        else {
            std::cout << " " << currentRelation.mainPath[i] << " ";
        }

        if (i < currentRelation.mainPath.size() - 1) {
            std::cout << "---> ";
        }
    }
    // Дополняем пробелами
    int currentLen = 4 + currentRelation.mainPath.size() * 3 + (currentRelation.mainPath.size() - 1) * 5;
    std::cout << std::string(max(0, WINDOW_WIDTH - 2 - currentLen), ' ') << "|" << std::endl;

    // Ветки
    if (!currentRelation.branches.empty()) {
        std::vector<int> branchPositions;
        for (const auto& pair : currentRelation.branches) {
            branchPositions.push_back(pair.first);
        }
        std::sort(branchPositions.begin(), branchPositions.end());

        size_t maxBranchLevels = 0;
        for (const auto& pair : currentRelation.branches) {
            if (pair.second.size() > maxBranchLevels) {
                maxBranchLevels = pair.second.size();
            }
        }

        for (size_t level = 0; level < maxBranchLevels; level++) {
            // Вертикальные линии
            std::cout << "|  ";
            int charCount = 4;

            for (size_t i = 0; i < currentRelation.mainPath.size(); i++) {
                while (charCount < elementPositions[i]) {
                    std::cout << " ";
                    charCount++;
                }

                bool hasBranch = false;
                if (currentRelation.branches.find(i) != currentRelation.branches.end()) {
                    if (level < currentRelation.branches.at(i).size()) {
                        hasBranch = true;
                    }
                }

                if (hasBranch) {
                    std::cout << " | ";
                }
                else {
                    std::cout << "   ";
                }
                charCount += 3;

                if (i < currentRelation.mainPath.size() - 1) {
                    std::cout << "     ";
                    charCount += 5;
                }
            }
            std::cout << std::string(max(0, WINDOW_WIDTH - 2 - charCount), ' ') << "|" << std::endl;

            // Ветки
            std::cout << "|  ";
            charCount = 4;

            for (size_t i = 0; i < currentRelation.mainPath.size(); i++) {
                while (charCount < elementPositions[i]) {
                    std::cout << " ";
                    charCount++;
                }

                bool hasBranch = false;
                BranchChain branchPath;

                if (currentRelation.branches.find(i) != currentRelation.branches.end()) {
                    if (level < currentRelation.branches.at(i).size()) {
                        hasBranch = true;
                        branchPath = currentRelation.branches.at(i)[level];
                    }
                }

                if (hasBranch) {
                    std::cout << " + ---> ";
                    charCount += 8;

                    for (size_t k = 0; k < branchPath.size(); k++) {
                        bool isHighlighted = false;

                        if (editMode == 2 && cursorLevel > 0) {
                            if ((int)i == deletePosition && (int)level == branchIndex - 1 && (int)k == branchCursor) {
                                setTextColor(12, 0);
                                std::cout << "[" << branchPath[k]->element << "]";
                                resetColors();
                                isHighlighted = true;
                                charCount += 3;
                            }
                        }
                        else if (cursorLevel > 0) {
                            if ((int)i == cursorPosition && (int)level == branchIndex - 1 && (int)k == branchCursor) {
                                std::cout << "[" << branchPath[k]->element << "]";
                                isHighlighted = true;
                                charCount += 3;
                            }
                        }

                        if (!isHighlighted) {
                            std::cout << " " << branchPath[k]->element << " ";
                            charCount += 3;
                        }

                        if (k < branchPath.size() - 1) {
                            std::cout << "---> ";
                            charCount += 5;
                        }
                    }
                }
                else {
                    std::cout << "   ";
                    charCount += 3;
                    if (i < currentRelation.mainPath.size() - 1) {
                        std::cout << "     ";
                        charCount += 5;
                    }
                }
            }
            std::cout << std::string(max(0, WINDOW_WIDTH - 2 - charCount), ' ') << "|" << std::endl;
        }
    }

    std::cout << "+" << std::string(WINDOW_WIDTH - 2, '-') << "+" << std::endl;

    // Режимы редактирования
    if (editMode == 1) {
        // 3-line input area (as requested): parent / new vertex / childs
        auto padLine = [&](const std::string& s) {
            std::string inner = s;
            if ((int)inner.size() > WINDOW_WIDTH - 4) inner = inner.substr(0, WINDOW_WIDTH - 4);
            std::cout << "| " << inner << std::string(max(0, WINDOW_WIDTH - 4 - (int)inner.size()), ' ') << " |" << std::endl;
            };

        std::string parentDisplay = insertParent.empty() ? "_" : insertParent;
        std::string vertexDisplay = insertVertex.empty() ? "_" : insertVertex;
        std::string childsDisplay = insertChild.empty() ? "_" : insertChild;

        // highlight only NEW / CHILDS fields; parent is always the vertex at cursor
        padLine("Parent (selected): [" + parentDisplay + "]");
        padLine(std::string("New vertex: ") + (insertStep == 1 ? ("[" + vertexDisplay + "]") : vertexDisplay));
        padLine(std::string("Childs: ") + (insertStep == 2 ? ("[" + childsDisplay + "]") : childsDisplay) + "   (use '/' to separate)");

        padLine("[A-Z] input   [SPACE] switch field   [BKSP] back");
        padLine("[ENTER] confirm insert   [ESC] cancel");
    }
    else if (editMode == 2) {
        std::cout << "| Selected for deletion: ";
        if (deletePosition >= 0 && deletePosition < (int)currentRelation.mainPath.size()) {
            setTextColor(12, 0);
            std::cout << currentRelation.mainPath[deletePosition];
            resetColors();
        }
        else {
            std::cout << "_";
        }
        std::cout << "                                 |" << std::endl;
        std::cout << "|                                                          |" << std::endl;
        std::cout << "| [SPACE] confirm delete   [ARROWS] navigate               |" << std::endl;
        std::cout << "| [ESC] cancel                                             |" << std::endl;
    }
    else if (editMode == 3) {
        std::string display = tempInput.empty() ? currentRelation.mainPath[cursorPosition] : tempInput;
        std::cout << "| Editing: [" << display << "]";
        std::cout << std::string(WINDOW_WIDTH - 16 - display.length(), ' ') << "|" << std::endl;
        std::cout << "|                                                          |" << std::endl;
        std::cout << "| [a-z] change   [SPACE] confirm   [BKSP] clear            |" << std::endl;
        std::cout << "| [ESC] cancel                                             |" << std::endl;
    }
    else {
        std::cout << "| Current: " << currentRelation.mainPath[cursorPosition];
        std::cout << std::string(WINDOW_WIDTH - 14 - currentRelation.mainPath[cursorPosition].length(), ' ') << "|" << std::endl;
        std::cout << "|                                                          |" << std::endl;
        std::cout << "| [I] insert   [D] delete   [M] modify                     |" << std::endl;
        std::cout << "| [ARROWS] navigate   [ENTER] save   [ESC] cancel          |" << std::endl;
    }
    std::cout << "+" << std::string(WINDOW_WIDTH - 2, '-') << "+" << std::endl;

    if (!message.empty()) {
        std::string msg = message;
        while (msg.length() > WINDOW_WIDTH - 4) {
            std::cout << "| " << msg.substr(0, WINDOW_WIDTH - 4) << " |" << std::endl;
            msg = msg.substr(WINDOW_WIDTH - 4);
        }
        if (!msg.empty()) {
            std::cout << "| " << msg << std::string(WINDOW_WIDTH - 4 - msg.length(), ' ') << " |" << std::endl;
        }
        std::cout << "+" << std::string(WINDOW_WIDTH - 2, '-') << "+" << std::endl;
    }
}


// Draw edit submenu
int drawEditSubmenu() {
    std::vector<std::string> submenu = {
        "~      EDIT OPTIONS           ~",
        "+-----------------------------+",
        "| 1. Insert new relation      |",
        "| 2. Delete existing relation |",
        "| 3. Modify existing relation |",
        "| 4. Cancel                   |",
        "+-----------------------------+"
    };

    int highlight = 2;
    hideCursor();
    printCenteredBlock(submenu, highlight);

    while (true) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == 27) { // ESC - cancel
                return 4;
            }
            else if (ch == -32) { // Arrows
                ch = _getch();
                if (ch == 72 && highlight > 2) { // Up
                    --highlight;
                    system("cls");
                    printCenteredBlock(submenu, highlight);
                }
                else if (ch == 80 && highlight < submenu.size() - 2) { // Down
                    ++highlight;
                    system("cls");
                    printCenteredBlock(submenu, highlight);
                }
            }
            else if (ch == 13) { // Enter
                ValidationResult vr = validateRelationDistances();
                if (!vr.isValid) {
                    drawCreateRelationEditor(vr.errorMessage);
                    _getch();
                    drawCreateRelationEditor();
                    continue;
                }
                base_clone = ux_base_from_relation(currentRelation);
                update_clone();
                update_base();
                // optional: keep task for display/debug
                task = alphabet(base_clone);
                return highlight - 1;
            }
        }
    }
}

// Interactive relation creation - УЛУЧШЕННАЯ НАВИГАЦИЯ ПО ВЕТВЛЕНИЯМ
void interactiveCreateRelation() {
    // Start from a clean relation (only 'a')
    currentRelation = RelationStructure();
    currentRelation.mainPath = { "a" };
    currentRelation.allElements.insert("a");
    cursorPosition = 0;
    cursorLevel = 0;
    branchCursor = 0;
    branchIndex = 0;
    subBranchLevel = 0;
    subBranchIndex = 0;
    subBranchCursor = 0;
    tempInput = "";

    drawCreateRelationEditor();

    while (true) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == 27) { // ESC - cancel
                return;
            }
            else if (ch == 13) { // ENTER - save with validation
                ValidationResult vr = validateRelationDistances();
                if (!vr.isValid) {
                    drawCreateRelationEditor(vr.errorMessage);
                    _getch();
                    drawCreateRelationEditor();
                    continue;
                }
                // Commit: base_clone is the source of truth for the whole program
                base_clone = ux_base_from_relation(currentRelation);
                update_clone();
                update_base();
                return;
            }
            else if (ch == -32) { // Arrows
                ch = _getch();

                if (ch == 75) { // Left arrow
                    if (cursorLevel == 0) {
                        if (cursorPosition > 0) {
                            cursorPosition--;
                            branchCursor = 0;
                            branchIndex = 0;
                        }
                    }
                    else if (subBranchLevel == 0) {
                        // В ветке
                        int branchPos = cursorPosition;
                        if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                            if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                                auto& currentBranch = currentRelation.branches[branchPos][branchIndex - 1];
                                if (branchCursor > 0) {
                                    branchCursor--;
                                }
                                else if (!tempInput.empty()) {
                                    tempInput = "";
                                }
                                else {
                                    cursorLevel = 0;
                                    branchIndex = 0;
                                    branchCursor = 0;
                                    tempInput = "";
                                }
                            }
                        }
                    }
                    else {
                        // В подветке
                        if (subBranchCursor > 0) {
                            subBranchCursor--;
                        }
                        else if (!tempInput.empty()) {
                            tempInput = "";
                        }
                        else {
                            subBranchLevel = 0;
                            subBranchIndex = 0;
                            subBranchCursor = 0;
                            tempInput = "";
                        }
                    }
                    drawCreateRelationEditor();
                }
                else if (ch == 77) { // Right arrow
                    if (cursorLevel == 0) {
                        if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
                            cursorPosition++;
                            branchCursor = 0;
                            branchIndex = 0;
                        }
                    }
                    else if (subBranchLevel == 0) {
                        // В ветке
                        int branchPos = cursorPosition;
                        if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                            if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                                auto& currentBranch = currentRelation.branches[branchPos][branchIndex - 1];
                                if (branchCursor < (int)currentBranch.size() - 1) {
                                    branchCursor++;
                                }
                                else if (branchCursor == (int)currentBranch.size() - 1 && !tempInput.empty()) {
                                    currentBranch.push_back(std::make_shared<BranchNode>(tempInput));
                                    currentRelation.allElements.insert(tempInput);
                                    tempInput = "";
                                    branchCursor++;
                                }
                            }
                        }
                    }
                    else {
                        // В подветке
                        int branchPos = cursorPosition;
                        if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                            if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                                auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                                if (branchCursor >= 0 && branchCursor < (int)branch.size()) {
                                    if (subBranchIndex > 0 && subBranchIndex <= (int)branch[branchCursor]->subBranches.size()) {
                                        auto& subBranch = branch[branchCursor]->subBranches[subBranchIndex - 1];
                                        if (subBranchCursor < (int)subBranch.size() - 1) {
                                            subBranchCursor++;
                                        }
                                        else if (!tempInput.empty()) {
                                            subBranch.push_back(std::make_shared<BranchNode>(tempInput));
                                            currentRelation.allElements.insert(tempInput);
                                            tempInput = "";
                                            subBranchCursor++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    drawCreateRelationEditor();
                }
                else if (ch == 72) { // Up arrow
                    if (subBranchLevel > 0) {
                        subBranchLevel = 0;
                        subBranchIndex = 0;
                        subBranchCursor = 0;
                        tempInput = "";
                    }
                    else if (cursorLevel > 0) {
                        cursorLevel = 0;
                        branchIndex = 0;
                        branchCursor = 0;
                        tempInput = "";
                    }
                    drawCreateRelationEditor();
                }
                else if (ch == 80) { // Down arrow
                    if (cursorLevel == 0) {
                        int branchPos = cursorPosition;
                        if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                            if (!currentRelation.branches[branchPos].empty()) {
                                cursorLevel = 1;
                                branchIndex = 1;
                                branchCursor = 0;
                                subBranchLevel = 0;
                                tempInput = "";
                            }
                        }
                    }
                    else if (subBranchLevel == 0) {
                        int branchPos = cursorPosition;
                        if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                            if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                                auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                                if (branchCursor >= 0 && branchCursor < (int)branch.size()) {
                                    if (!branch[branchCursor]->subBranches.empty()) {
                                        subBranchLevel = 1;
                                        subBranchIndex = 1;
                                        subBranchCursor = 0;
                                        tempInput = "";
                                    }
                                    else if (branchIndex < (int)currentRelation.branches[branchPos].size()) {
                                        branchIndex++;
                                        branchCursor = 0;
                                    }
                                }
                            }
                        }
                    }
                    drawCreateRelationEditor();
                }
            } // Конец обработки стрелок
            else if (ch == 32) { // SPACE
                if (cursorLevel == 0) {
                    // Основной путь
                    cursorPosition++;
                    if (cursorPosition >= (int)currentRelation.mainPath.size()) {
                        std::string newElement = "b";
                        for (char c = 'b'; c <= 'z'; c++) {
                            std::string letter(1, c);
                            if (currentRelation.allElements.find(letter) == currentRelation.allElements.end()) {
                                newElement = letter;
                                break;
                            }
                        }
                        currentRelation.mainPath.push_back(newElement);
                        currentRelation.allElements.insert(newElement);
                    }
                    branchCursor = 0;
                    branchIndex = 0;
                }
                else if (subBranchLevel == 0) {
                    // В ветке
                    int branchPos = cursorPosition;
                    if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                        if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                            auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                            if (!tempInput.empty()) {
                                if (branchCursor >= (int)branch.size()) {
                                    branch.push_back(std::make_shared<BranchNode>(tempInput));
                                }
                                else {
                                    branch[branchCursor] = std::make_shared<BranchNode>(tempInput);
                                }
                                currentRelation.allElements.insert(tempInput);
                                tempInput = "";
                                branchCursor++;
                            }
                            else if (branchCursor < (int)branch.size() - 1) {
                                branchCursor++;
                            }
                            else {
                                std::string newElement = "b";
                                for (char c = 'b'; c <= 'z'; c++) {
                                    std::string letter(1, c);
                                    if (currentRelation.allElements.find(letter) == currentRelation.allElements.end()) {
                                        newElement = letter;
                                        break;
                                    }
                                }
                                tempInput = newElement;
                                branchCursor++;
                            }
                        }
                    }
                }
                else {
                    // В подветке
                    int branchPos = cursorPosition;
                    if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                        if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                            auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                            if (branchCursor >= 0 && branchCursor < (int)branch.size()) {
                                if (subBranchIndex > 0 && subBranchIndex <= (int)branch[branchCursor]->subBranches.size()) {
                                    auto& subBranch = branch[branchCursor]->subBranches[subBranchIndex - 1];

                                    if (!tempInput.empty()) {
                                        if (subBranchCursor >= (int)subBranch.size()) {
                                            subBranch.push_back(std::make_shared<BranchNode>(tempInput));
                                        }
                                        else {
                                            subBranch[subBranchCursor] = std::make_shared<BranchNode>(tempInput);
                                        }
                                        currentRelation.allElements.insert(tempInput);
                                        tempInput = "";
                                        subBranchCursor++;
                                    }
                                    else if (subBranchCursor < (int)subBranch.size()) {
                                        subBranchCursor++;
                                    }
                                    else {
                                        std::string newElement = "b";
                                        for (char c = 'b'; c <= 'z'; c++) {
                                            std::string l(1, c);
                                            if (currentRelation.allElements.find(l) == currentRelation.allElements.end()) {
                                                newElement = l;
                                                break;
                                            }
                                        }
                                        tempInput = newElement;
                                    }
                                }
                            }
                        }
                    }
                }
                drawCreateRelationEditor();
            }
            else if (ch == 8) { // BACKSPACE
                if (cursorLevel == 0) {
                    if (currentRelation.mainPath.size() > 1) {
                        currentRelation.allElements.erase(currentRelation.mainPath.back());
                        currentRelation.mainPath.pop_back();
                        cursorPosition = min(cursorPosition, (int)currentRelation.mainPath.size() - 1);
                        if (cursorPosition < 0) cursorPosition = 0;
                        branchCursor = 0;
                        branchIndex = 0;
                    }
                }
                else if (subBranchLevel == 0) {
                    // В ветке
                    int branchPos = cursorPosition;
                    if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                        if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                            auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                            if (!tempInput.empty()) {
                                tempInput = "";
                            }
                            else if (!branch.empty()) {
                                currentRelation.allElements.erase(branch.back()->element);
                                branch.pop_back();
                                branchCursor = max(0, (int)branch.size() - 1);

                                if (branch.empty()) {
                                    currentRelation.branches[branchPos].erase(
                                        currentRelation.branches[branchPos].begin() + branchIndex - 1);

                                    if (currentRelation.branches[branchPos].empty()) {
                                        currentRelation.branches.erase(branchPos);
                                        cursorLevel = 0;
                                        branchIndex = 0;
                                    }
                                    else {
                                        branchIndex = max(1, branchIndex - 1);
                                    }
                                    branchCursor = 0;
                                }
                            }
                            else {
                                currentRelation.branches[branchPos].erase(
                                    currentRelation.branches[branchPos].begin() + branchIndex - 1);

                                if (currentRelation.branches[branchPos].empty()) {
                                    currentRelation.branches.erase(branchPos);
                                    cursorLevel = 0;
                                    branchIndex = 0;
                                }
                                else {
                                    branchIndex = max(1, branchIndex - 1);
                                }
                                branchCursor = 0;
                            }
                        }
                    }
                }
                else {
                    // В подветке
                    int branchPos = cursorPosition;
                    if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                        if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                            auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                            if (branchCursor >= 0 && branchCursor < (int)branch.size()) {
                                if (subBranchIndex > 0 && subBranchIndex <= (int)branch[branchCursor]->subBranches.size()) {
                                    auto& subBranch = branch[branchCursor]->subBranches[subBranchIndex - 1];

                                    if (!tempInput.empty()) {
                                        tempInput = "";
                                    }
                                    else if (!subBranch.empty()) {
                                        currentRelation.allElements.erase(subBranch.back()->element);
                                        subBranch.pop_back();
                                        subBranchCursor = max(0, (int)subBranch.size() - 1);

                                        if (subBranch.empty()) {
                                            branch[branchCursor]->subBranches.erase(
                                                branch[branchCursor]->subBranches.begin() + subBranchIndex - 1);

                                            if (branch[branchCursor]->subBranches.empty()) {
                                                subBranchLevel = 0;
                                                subBranchIndex = 0;
                                            }
                                            else {
                                                subBranchIndex = max(1, subBranchIndex - 1);
                                            }
                                            subBranchCursor = 0;
                                        }
                                    }
                                    else {
                                        branch[branchCursor]->subBranches.erase(
                                            branch[branchCursor]->subBranches.begin() + subBranchIndex - 1);

                                        if (branch[branchCursor]->subBranches.empty()) {
                                            subBranchLevel = 0;
                                            subBranchIndex = 0;
                                        }
                                        else {
                                            subBranchIndex = max(1, subBranchIndex - 1);
                                        }
                                        subBranchCursor = 0;
                                    }
                                }
                            }
                        }
                    }
                }
                drawCreateRelationEditor();
            }
            else if (ch == '/') { // Создание ветки
                if (cursorLevel == 0) {
                    BranchChain newBranch;
                    currentRelation.branches[cursorPosition].push_back(newBranch);
                    cursorLevel = 1;
                    branchIndex = currentRelation.branches[cursorPosition].size();
                    branchCursor = 0;
                    subBranchLevel = 0;
                    subBranchIndex = 0;
                    subBranchCursor = 0;
                    tempInput = "";
                }
                else if (subBranchLevel == 0) {
                    int branchPos = cursorPosition;
                    if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                        if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                            auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                            if (branchCursor >= 0 && branchCursor < (int)branch.size()) {
                                if (ux_getCursorElement() == "a") { drawCreateRelationEditor("Branching from vertex a is not allowed."); _getch(); drawCreateRelationEditor(); break; }
                                BranchChain newSubBranch;
                                branch[branchCursor]->subBranches.push_back(newSubBranch);
                                subBranchLevel = 1;
                                subBranchIndex = branch[branchCursor]->subBranches.size();
                                subBranchCursor = 0;
                                tempInput = "";
                            }
                        }
                    }
                }
                else {
                    int branchPos = cursorPosition;
                    if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                        if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                            auto& branch = currentRelation.branches[branchPos][branchIndex - 1];
                            if (branchCursor >= 0 && branchCursor < (int)branch.size()) {
                                if (ux_getCursorElement() == "a") { drawEditRelationEditor("Branching from vertex a is not allowed."); _getch(); drawEditRelationEditor(); break; }
                                BranchChain newSubBranch;
                                branch[branchCursor]->subBranches.push_back(newSubBranch);
                                subBranchIndex = branch[branchCursor]->subBranches.size();
                                subBranchCursor = 0;
                                tempInput = "";
                            }
                        }
                    }
                }
                drawCreateRelationEditor();
            }
            else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                std::string letterStr(1, ch);

                // Allow replacing the current element with itself.
                std::string currentAtCursor;
                if (cursorLevel == 0 && cursorPosition >= 0 && cursorPosition < (int)currentRelation.mainPath.size()) {
                    currentAtCursor = currentRelation.mainPath[cursorPosition];
                }

                bool uniqueOrSame = (currentRelation.allElements.find(letterStr) == currentRelation.allElements.end()) ||
                    (!currentAtCursor.empty() && letterStr == currentAtCursor);

                if (uniqueOrSame) {
                    if (cursorLevel == 0) {
                        if (cursorPosition >= 0 && cursorPosition < (int)currentRelation.mainPath.size()) {
                            currentRelation.allElements.erase(currentRelation.mainPath[cursorPosition]);
                            currentRelation.mainPath[cursorPosition] = letterStr;
                            currentRelation.allElements.insert(letterStr);
                        }
                    }
                    else {
                        tempInput = letterStr;
                    }
                }
                drawCreateRelationEditor();
            }
        } // Конец if (_kbhit())
    } // Конец while (true)
}

// Interactive relation editing
void interactiveEditRelation() {
    parseTaskToRelation();
    cursorPosition = 0;
    cursorLevel = 0;
    branchCursor = 0;
    branchIndex = 0;
    tempInput = "";
    editMode = 0;
    insertStep = 0;
    insertParent = "";
    insertVertex = "";
    insertChild = "";
    deletePosition = -1;

    if (editMode == 3) {
        tempInput = currentRelation.mainPath[cursorPosition];
        drawEditRelationEditor();
    }
    else if (editMode == 1) {
        insertStep = 0;
        insertParent = currentRelation.mainPath[cursorPosition];
        insertVertex = "";
        if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
            insertChild = currentRelation.mainPath[cursorPosition + 1];
        }
        else {
            insertChild = "";
        }
        drawEditRelationEditor();
    }
    else if (editMode == 2) {
        deletePosition = cursorPosition;
        drawEditRelationEditor();
    }
    else {
        drawEditRelationEditor();
    }

    while (true) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == 27) {
                if (editMode != 0) {
                    editMode = 0;
                    int choice = drawEditSubmenu();
                    if (choice == 4) return;
                    editMode = choice;
                    if (editMode == 1) {
                        insertStep = 0;
                        insertParent = currentRelation.mainPath[cursorPosition];
                        insertVertex = "";
                        if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
                            insertChild = currentRelation.mainPath[cursorPosition + 1];
                        }
                        else {
                            insertChild = "";
                        }
                    }
                    else if (editMode == 3) {
                        tempInput = currentRelation.mainPath[cursorPosition];
                    }
                    else if (editMode == 2) {
                        deletePosition = cursorPosition;
                    }
                    drawEditRelationEditor();
                }
                else {
                    return;
                }
            }
            else if (ch == 13) {
                if (editMode == 1) {
                    if (!insertVertex.empty() && !insertParent.empty()) {
                        std::string childrenRaw = insertChild;
                        std::string children;
                        for (char c : childrenRaw) {
                            if (std::isalpha((unsigned char)c)) children.push_back((char)std::tolower((unsigned char)c));
                        }

                        auto exists = [&](char v) -> bool {
                            return currentRelation.allElements.count(std::string(1, v)) > 0;
                            };

                        bool ok = true;
                        if (insertVertex.size() != 1 || !std::isalpha((unsigned char)insertVertex[0])) ok = false;
                        char newV = ok ? (char)std::tolower((unsigned char)insertVertex[0]) : '?';
                        char parentV = (char)std::tolower((unsigned char)insertParent[0]);

                        if (!ok) {
                            drawEditRelationEditor("Invalid new vertex name.");
                            _getch();
                        }
                        else if (exists(newV)) {
                            drawEditRelationEditor("Vertex already exists.");
                            _getch();
                        }
                        else {
                            if ((int)children.size() > 1) {
                                for (char c : children) {
                                    if (exists(c)) { ok = false; break; }
                                }
                                if (!ok) {
                                    drawEditRelationEditor("Chain contains existing vertex name.");
                                    _getch();
                                }
                            }
                        }

                        if (ok) {
                            std::string firstChild = (children.empty() ? "" : std::string(1, children[0]));
                            if (canInsertVertex(newV, parentV, firstChild)) {
                                executeInsert(newV, parentV, firstChild);
                                task = buildTaskString();
                                parseTaskToRelation();
                            }
                            else {
                                drawEditRelationEditor("Insertion is not allowed.");
                                _getch();
                                ok = false;
                            }
                        }

                        if (ok && (int)children.size() > 1) {
                            char chainParent = children[0];
                            for (size_t i = 1; i < children.size(); ++i) {
                                char cv = children[i];
                                if (!canInsertVertex(cv, chainParent, "")) {
                                    drawEditRelationEditor("Cannot extend chain (invalid insert).");
                                    _getch();
                                    ok = false;
                                    break;
                                }
                                executeInsert(cv, chainParent, "");
                                task = buildTaskString();
                                parseTaskToRelation();
                                chainParent = cv;
                            }
                        }

                        if (!ok) {
                            // nothing
                        }
                        else {
                            // success
                        }
                    }
                    else {
                        drawEditRelationEditor("Cannot insert element");
                        _getch();
                    }
                    editMode = 0;
                    drawEditRelationEditor();
                }
                else {
                    task = buildTaskString();
                    return;
                }
            }
            else if (ch == -32) {
                ch = _getch();
                if (editMode == 1) {
                    // Код для режима вставки с навигацией стрелками
                }
                else if (editMode == 2) {
                    if (ch == 75) {
                        if (deletePosition > 0) {
                            deletePosition--;
                        }
                        else {
                            deletePosition = currentRelation.mainPath.size() - 1;
                        }
                        drawEditRelationEditor();
                    }
                    else if (ch == 77) {
                        if (deletePosition < (int)currentRelation.mainPath.size() - 1) {
                            deletePosition++;
                        }
                        else {
                            deletePosition = 0;
                        }
                        drawEditRelationEditor();
                    }
                }
                else if (editMode == 3) {
                    // Код для режима изменения с навигацией стрелками
                }
                else {
                    if (ch == 75) {
                        if (cursorLevel == 0) {
                            if (cursorPosition > 0) {
                                cursorPosition--;
                                branchCursor = 0;
                                branchIndex = 0;
                            }
                        }
                        else {
                            int branchPos = cursorPosition;
                            if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                                if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                                    auto& currentBranch = currentRelation.branches[branchPos][branchIndex - 1];
                                    if (branchCursor > 0) {
                                        branchCursor--;
                                    }
                                    else if (branchIndex > 1) {
                                        branchIndex--;
                                        auto& prevBranch = currentRelation.branches[branchPos][branchIndex - 1];
                                        branchCursor = max(0, (int)prevBranch.size() - 1);
                                    }
                                    else {
                                        cursorLevel = 0;
                                        branchIndex = 0;
                                        branchCursor = 0;
                                    }
                                }
                            }
                        }
                        drawEditRelationEditor();
                    }
                    else if (ch == 77) {
                        if (cursorLevel == 0) {
                            if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
                                cursorPosition++;
                                branchCursor = 0;
                                branchIndex = 0;
                            }
                        }
                        else {
                            int branchPos = cursorPosition;
                            if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                                if (branchIndex > 0 && branchIndex <= (int)currentRelation.branches[branchPos].size()) {
                                    auto& currentBranch = currentRelation.branches[branchPos][branchIndex - 1];
                                    if (branchCursor < (int)currentBranch.size() - 1) {
                                        branchCursor++;
                                    }
                                    else if (branchIndex < (int)currentRelation.branches[branchPos].size()) {
                                        branchIndex++;
                                        branchCursor = 0;
                                    }
                                }
                            }
                        }
                        drawEditRelationEditor();
                    }
                    else if (ch == 72) {
                        if (cursorLevel > 0) {
                            int branchPos = cursorPosition;
                            if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                                if (branchIndex > 1) {
                                    branchIndex--;
                                    branchCursor = 0;
                                }
                                else if (branchIndex == 1) {
                                    cursorLevel = 0;
                                    branchCursor = 0;
                                    branchIndex = 0;
                                }
                            }
                        }
                        drawEditRelationEditor();
                    }
                    else if (ch == 80) {
                        int branchPos = cursorPosition;
                        if (currentRelation.branches.find(branchPos) != currentRelation.branches.end()) {
                            const auto& branches = currentRelation.branches.at(branchPos);
                            if (cursorLevel == 0) {
                                if (!branches.empty()) {
                                    cursorLevel = 1;
                                    branchIndex = 1;
                                    branchCursor = 0;
                                }
                            }
                            else {
                                if (branchIndex < (int)branches.size()) {
                                    branchIndex++;
                                    branchCursor = 0;
                                }
                            }
                        }
                        else if (cursorLevel == 0) {
                            // ЗАПРЕЩАЕМ автоматическое создание первой ветки стрелками
                        }
                        drawEditRelationEditor();
                    }
                }
            }
            else if (ch == 'I' || ch == 'i') {
                if (editMode == 0) {
                    editMode = 1;
                    insertStep = 1;
                    insertParent = ux_getCursorElement();
                    insertVertex = "";
                    insertChild = "";
                    drawEditRelationEditor();
                }
            }
            else if (ch == 'D' || ch == 'd') {
                if (editMode == 0) {
                    editMode = 2;
                    deletePosition = cursorPosition;
                    cursorLevel = 0;
                    branchCursor = 0;
                    branchIndex = 0;
                    drawEditRelationEditor();
                }
            }
            else if (ch == 'M' || ch == 'm') {
                if (editMode == 0) {
                    editMode = 3;
                    tempInput = currentRelation.mainPath[cursorPosition];
                    drawEditRelationEditor();
                }
            }
            else if (ch == 32) {
                if (editMode == 1) {
                    insertStep = (insertStep == 2 ? 1 : 2);
                    drawEditRelationEditor();
                }
                else if (editMode == 2) {
                    if (deletePosition >= 0 && currentRelation.mainPath.size() > 1) {

                        // выбранная вершина (у тебя вершины по 1 символу)
                        std::string v = currentRelation.mainPath[deletePosition];
                        if (v.size() != 1) {
                            drawEditRelationEditor("Cannot delete element");
                            _getch();
                            drawEditRelationEditor();
                            continue;
                        }

                        // НОВОЕ: проверяем как НАБОР
                        std::vector<std::string> del = { v };

                        base_clone = copying_map(base);
                        update_clone();

                        if (valid_delete(del) == 0) {
                            del = normalizeDeleteList(del);       // 0 = можно
                            if (deleting_data(del) == 0) {            // удаляем тоже пакетом
                                update_base();
                                parseTaskToRelation();
                                task = buildTaskString();
                            }
                            else {
                                drawEditRelationEditor("Deletion failed");
                                _getch();
                            }
                        }
                        else {
                            drawEditRelationEditor("Cannot delete element");
                            _getch();
                        }
                    }
                    drawEditRelationEditor();
                }
                else if (editMode == 3) {
                    if (!tempInput.empty() && tempInput != currentRelation.mainPath[cursorPosition]) {
                        if (canInsertVertex(tempInput[0], 'a', "")) {
                            currentRelation.allElements.erase(currentRelation.mainPath[cursorPosition]);
                            currentRelation.mainPath[cursorPosition] = tempInput;
                            currentRelation.allElements.insert(tempInput);
                        }
                        else {
                            drawEditRelationEditor("Cannot modify element");
                            _getch();
                        }
                    }
                    editMode = 0;
                    tempInput = "";
                    drawEditRelationEditor();
                }
                else {
                    if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
                        cursorPosition++;
                        branchCursor = 0;
                        branchIndex = 0;
                    }
                    drawEditRelationEditor();
                }
            }
            else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
                if (editMode == 1) {
                    char letter = std::tolower(ch);
                    std::string letterStr(1, letter);
                    if (insertStep == 1) {
                        if (currentRelation.allElements.find(letterStr) == currentRelation.allElements.end()) {
                            insertVertex = letterStr;
                        }
                    }
                    else {
                        if (insertChild.find(letter) == std::string::npos) insertChild.push_back(letter);
                    }
                    drawEditRelationEditor();
                }
                else if (editMode == 3) {
                    char letter = std::tolower(ch);
                    std::string letterStr(1, letter);
                    if (currentRelation.allElements.find(letterStr) == currentRelation.allElements.end() ||
                        letterStr == currentRelation.mainPath[cursorPosition]) {
                        tempInput = letterStr;
                    }
                    drawEditRelationEditor();
                }
            }
            else if (ch == '/') {
                if (editMode == 1 && insertStep == 2) {
                    if (insertChild.empty() || insertChild.back() != '/') insertChild.push_back('/');
                    drawEditRelationEditor();
                }
            }
            else if (ch == 8) {
                if (editMode == 1) {
                    if (insertStep == 1) {
                        if (!insertVertex.empty()) {
                            insertVertex = insertVertex.substr(0, insertVertex.length() - 1);
                        }
                    }
                    else {
                        if (!insertChild.empty()) {
                            insertChild = insertChild.substr(0, insertChild.length() - 1);
                        }
                    }
                    drawEditRelationEditor();
                }
                else if (editMode == 3) {
                    if (!tempInput.empty()) {
                        tempInput = tempInput.substr(0, tempInput.length() - 1);
                    }
                    drawEditRelationEditor();
                }
            }
        }
    }
}
void createOrderRelation() {
    system("cls");
    interactiveCreateRelation();
}

// ==================== B-MODE EDITING: separate Add/Delete menus ====================

static std::string ux_getCursorElement() {
    // Cursor is on main path
    if (cursorLevel == 0) {
        if (cursorPosition >= 0 && cursorPosition < (int)currentRelation.mainPath.size()) {
            return currentRelation.mainPath[cursorPosition];
        }
        return "";
    }

    // Cursor is on a branch from main path
    const int branchPos = cursorPosition;
    auto it = currentRelation.branches.find(branchPos);
    if (it == currentRelation.branches.end() || branchIndex <= 0 || branchIndex > (int)it->second.size()) return "";
    auto& branch = it->second[branchIndex - 1];
    if (branchCursor < 0 || branchCursor >= (int)branch.size()) return "";

    // Cursor is on a sub-branch
    if (subBranchLevel > 0) {
        auto& node = branch[branchCursor];
        if (subBranchIndex <= 0 || subBranchIndex > (int)node->subBranches.size()) return "";
        auto& sub = node->subBranches[subBranchIndex - 1];
        if (subBranchCursor < 0 || subBranchCursor >= (int)sub.size()) return "";
        return sub[subBranchCursor]->element;
    }

    return branch[branchCursor]->element;
}

static void ux_navigateGraphArrow(int key) {
    // key: 75 left, 77 right, 72 up, 80 down (as returned after -32)
    if (key == 75) { // LEFT
        if (subBranchLevel > 0) {
            if (subBranchCursor > 0) {
                subBranchCursor--;
            }
            else {
                // if at first element of sub-branch: go back to parent branch node
                subBranchLevel = 0;
                subBranchIndex = 0;
                subBranchCursor = 0;
            }
            return;
        }

        if (cursorLevel == 0) {
            if (cursorPosition > 0) {
                cursorPosition--;
                branchCursor = 0;
                branchIndex = 0;
            }
            return;
        }

        // In a branch
        if (branchCursor > 0) {
            branchCursor--;
        }
        else {
            // At first element of branch: return to main path
            cursorLevel = 0;
            branchIndex = 0;
            branchCursor = 0;
        }
        return;
    }

    if (key == 77) { // RIGHT
        if (subBranchLevel > 0) {
            const int branchPos = cursorPosition;
            auto it = currentRelation.branches.find(branchPos);
            if (it == currentRelation.branches.end() || branchIndex <= 0 || branchIndex > (int)it->second.size()) return;
            auto& branch = it->second[branchIndex - 1];
            if (branchCursor < 0 || branchCursor >= (int)branch.size()) return;
            auto& node = branch[branchCursor];
            if (subBranchIndex <= 0 || subBranchIndex > (int)node->subBranches.size()) return;
            auto& sub = node->subBranches[subBranchIndex - 1];
            if (subBranchCursor < (int)sub.size() - 1) subBranchCursor++;
            return;
        }

        if (cursorLevel == 0) {
            if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
                cursorPosition++;
                branchCursor = 0;
                branchIndex = 0;
            }
            return;
        }

        const int branchPos = cursorPosition;
        auto it = currentRelation.branches.find(branchPos);
        if (it == currentRelation.branches.end() || branchIndex <= 0 || branchIndex > (int)it->second.size()) return;
        auto& branch = it->second[branchIndex - 1];
        if (branchCursor < (int)branch.size() - 1) branchCursor++;
        return;
    }

    if (key == 72) { // UP
        if (subBranchLevel > 0) {
            subBranchLevel = 0;
            subBranchIndex = 0;
            subBranchCursor = 0;
            return;
        }
        if (cursorLevel > 0) {
            cursorLevel = 0;
            branchIndex = 0;
            branchCursor = 0;
        }
        return;
    }

    if (key == 80) { // DOWN
        if (cursorLevel == 0) {
            const int branchPos = cursorPosition;
            auto it = currentRelation.branches.find(branchPos);
            if (it != currentRelation.branches.end() && !it->second.empty()) {
                cursorLevel = 1;
                branchIndex = 1;
                branchCursor = 0;
                subBranchLevel = 0;
                subBranchIndex = 0;
                subBranchCursor = 0;
            }
            return;
        }

        // In a branch: enter sub-branch if exists at current node, else go to next branch from same main vertex.
        if (subBranchLevel == 0) {
            const int branchPos = cursorPosition;
            auto it = currentRelation.branches.find(branchPos);
            if (it == currentRelation.branches.end() || branchIndex <= 0 || branchIndex > (int)it->second.size()) return;
            auto& branch = it->second[branchIndex - 1];
            if (branchCursor < 0 || branchCursor >= (int)branch.size()) return;
            auto& node = branch[branchCursor];
            if (!node->subBranches.empty()) {
                subBranchLevel = 1;
                subBranchIndex = 1;
                subBranchCursor = 0;
                return;
            }
            if (branchIndex < (int)it->second.size()) {
                branchIndex++;
                branchCursor = 0;
                return;
            }
            return;
        }

        // In sub-branch: cycle through sub-branches on the same node
        const int branchPos = cursorPosition;
        auto it = currentRelation.branches.find(branchPos);
        if (it == currentRelation.branches.end() || branchIndex <= 0 || branchIndex > (int)it->second.size()) return;
        auto& branch = it->second[branchIndex - 1];
        if (branchCursor < 0 || branchCursor >= (int)branch.size()) return;
        auto& node = branch[branchCursor];
        if (subBranchIndex < (int)node->subBranches.size()) {
            subBranchIndex++;
            subBranchCursor = 0;
        }
        return;
    }
}

static void ux_drawDeleteOrAddGraph(const std::string& title,
    const std::unordered_set<std::string>* marked,
    const std::string& footerLine1,
    const std::string& footerLine2,
    const std::string& message) {
    // Render into a centered block (same as main menu) and avoid branch anchor "drift".
    // Use (almost) full console width so the graph can be longer and legends fit.
    const int WINDOW_WIDTH = max(80, ux_console_width() - 4);
    const int INNER_W = WINDOW_WIDTH - 2;

    auto makeLine = [&]() -> std::string {
        return "|" + std::string(INNER_W, ' ') + "|";
        };
    auto frameTop = [&]() -> std::string {
        return "+" + std::string(INNER_W, '-') + "+";
        };
    auto put = [&](std::string& line, int x, char c) {
        // x is inside-frame coord [0..INNER_W-1]
        if (x < 0 || x >= INNER_W) return;
        line[1 + x] = c;
        };
    auto putStr = [&](std::string& line, int x, const std::string& s) {
        for (int i = 0; i < (int)s.size(); ++i) put(line, x + i, s[i]);
        };
    auto token3 = [&](const std::string& el, bool isCursor, bool isMarked) -> std::string {
        // fixed width 3, so X math never changes
        if (isCursor) return "[" + el + "]";
        if (isMarked) return std::string("\001") + el + "\002";
        return " " + el + " ";
        };

    // Compute X anchors for main path tokens (center char of the 3-wide token)
    std::vector<int> mainCenterX;
    {
        int x = 2; // inside-frame: starts after "|  "
        for (size_t i = 0; i < currentRelation.mainPath.size(); ++i) {
            mainCenterX.push_back(x + 1);
            x += 3;
            if (i + 1 < currentRelation.mainPath.size()) x += 5; // "---> "
        }
    }

    std::vector<std::string> block;
    block.push_back(frameTop());
    {
        std::string line = makeLine();
        int pad = (INNER_W - (int)title.size()) / 2;
        putStr(line, max(0, pad), title);
        block.push_back(line);
    }
    block.push_back(frameTop());

    // Main path line
    {
        std::string line = makeLine();
        putStr(line, 0, "  ");
        int x = 2;
        for (size_t i = 0; i < currentRelation.mainPath.size(); ++i) {
            const std::string& el = currentRelation.mainPath[i];
            bool isCursor = (cursorLevel == 0 && (int)i == cursorPosition);
            bool isMarked = marked && marked->count(el) > 0;
            putStr(line, x, token3(el, isCursor, isMarked));
            x += 3;
            if (i + 1 < currentRelation.mainPath.size()) {
                putStr(line, x, "---> ");
                x += 5;
            }
        }
        block.push_back(line);
    }

    // Recursive branch renderer: appends lines below, always anchoring to a fixed X.
    // branchPos/branchIdx are used only to highlight cursor correctly.
    std::function<void(int, const BranchChain&, int, int)> renderChain;
    renderChain = [&](int parentCenterX, const BranchChain& chain, int branchPos, int branchIdx) {
        // vertical line
        {
            std::string v = makeLine();
            put(v, parentCenterX, '|');
            block.push_back(v);
        }
        // branch line + chain
        {
            std::string line = makeLine();
            put(line, parentCenterX, '+');
            putStr(line, parentCenterX + 1, "---> ");
            int x = parentCenterX + 6;
            std::vector<int> centers;
            centers.reserve(chain.size());
            for (size_t i = 0; i < chain.size(); ++i) {
                const std::string& el = chain[i]->element;
                const bool isCursor = (cursorLevel > 0 && subBranchLevel == 0 &&
                    cursorPosition == branchPos && (branchIndex - 1) == branchIdx &&
                    (int)i == branchCursor);
                const bool isMarked = marked && marked->count(el) > 0;
                putStr(line, x, token3(el, isCursor, isMarked));
                centers.push_back(x + 1);
                x += 3;
                if (i + 1 < chain.size()) {
                    putStr(line, x, "---> ");
                    x += 5;
                }
            }
            block.push_back(line);

            // sub-branches (any depth)
            for (size_t i = 0; i < chain.size(); ++i) {
                for (size_t sb = 0; sb < chain[i]->subBranches.size(); ++sb) {
                    const auto& sub = chain[i]->subBranches[sb];
                    // NOTE: navigation currently supports only 1 level of sub-branch highlight;
                    // deeper levels are still rendered correctly.
                    renderChain(centers[i], sub, branchPos, branchIdx);
                }
            }
        }
        };

    // branches from main path
    if (!currentRelation.branches.empty()) {
        std::vector<int> positions;
        positions.reserve(currentRelation.branches.size());
        for (const auto& kv : currentRelation.branches) positions.push_back(kv.first);
        std::sort(positions.begin(), positions.end());

        for (int pos : positions) {
            if (pos < 0 || pos >= (int)mainCenterX.size()) continue;
            int anchorX = mainCenterX[pos];
            const auto& chains = currentRelation.branches.at(pos);
            for (size_t bi = 0; bi < chains.size(); ++bi) renderChain(anchorX, chains[bi], pos, (int)bi);
        }
    }

    block.push_back(frameTop());
    {
        std::string l1 = makeLine();
        putStr(l1, 1, footerLine1);
        block.push_back(l1);
        std::string l2 = makeLine();
        putStr(l2, 1, footerLine2);
        block.push_back(l2);
        std::string l3 = makeLine();
        putStr(l3, 1, "[ESC] cancel   [ENTER] confirm");
        block.push_back(l3);
    }
    block.push_back(frameTop());

    if (!message.empty()) {
        std::string msg = message;
        while ((int)msg.size() > INNER_W - 2) {
            std::string l = makeLine();
            putStr(l, 1, msg.substr(0, INNER_W - 2));
            block.push_back(l);
            msg = msg.substr(INNER_W - 2);
        }
        std::string l = makeLine();
        putStr(l, 1, msg);
        block.push_back(l);
        block.push_back(frameTop());
    }

    system("cls");
    printCenteredBlock(block, -1);
}

void addVertexMenu() {
    // Refresh relation from current base
    parseTaskToRelation();
    task = buildTaskString();

    cursorPosition = 0;
    cursorLevel = 0;
    branchCursor = 0;
    branchIndex = 0;
    subBranchLevel = 0;
    subBranchIndex = 0;
    subBranchCursor = 0;

    std::string newVertex;
    std::string childs;
    int field = 0; // 0=new vertex, 1=childs
    std::string msg;

    while (true) {
        std::string parent = ux_getCursorElement();
        std::string f1 = "Parent: [" + (parent.empty() ? std::string("_") : parent) + "]";
        std::string f2 = std::string("New: ") + (newVertex.empty() ? "_" : newVertex) +
            "   Childs: " + (childs.empty() ? "_" : childs) +
            (field == 0 ? "   <NEW>" : "   <CHILDS>");
        ux_drawDeleteOrAddGraph("ADD VERTEX", nullptr,
            f1,
            f2, msg);
        msg.clear();

        // Block until key (prevents endless redraw / 100% CPU)
        char ch = _getch();

        if (ch == 27) return; // ESC

        if (ch == -32) {
            ch = _getch();
            ux_navigateGraphArrow((unsigned char)ch);
            continue;
        }

        if (ch == 32) { // SPACE
            field = 1 - field;
            continue;
        }

        if (ch == 8) { // Backspace
            if (field == 0) {
                newVertex.clear();
            }
            else {
                if (!childs.empty()) childs.pop_back();
            }
            continue;
        }

        if (ch == 13) { // ENTER
            if (parent.empty()) {
                msg = "Parent vertex is not selected.";
                continue;
            }
            if (newVertex.size() != 1) {
                msg = "Enter exactly one letter for the new vertex.";
                continue;
            }
            if (currentRelation.allElements.count(newVertex) > 0) {
                msg = "New vertex already exists in the graph.";
                continue;
            }

            base_clone = copying_map(base);
            update_clone();
            if (!canInsertVertex(newVertex[0], parent[0], childs)) {
                msg = "Cannot insert: invalid insert operation.";
                continue;
            }
            executeInsert(newVertex[0], parent[0], childs);
            task = buildTaskString();
            parseTaskToRelation();
            return;
        }

        // Input
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            if (field == 0) {
                newVertex = std::string(1, ch);
            }
            else {
                childs.push_back(ch);
            }
        }
        else if (field == 1 && ch == '/') {
            childs.push_back('/');
        }
    }
}

void deleteVerticesMenu() {
    parseTaskToRelation();
    task = buildTaskString();

    cursorPosition = 0;
    cursorLevel = 0;
    branchCursor = 0;
    branchIndex = 0;
    subBranchLevel = 0;
    subBranchIndex = 0;
    subBranchCursor = 0;

    std::unordered_set<std::string> marked;
    std::string msg;

    while (true) {
        std::string selected;
        for (const auto& s : marked) selected += s;
        if (selected.empty()) selected = "(none)";

        ux_drawDeleteOrAddGraph("DELETE VERTICES", &marked,
            "Arrows: move cursor   SPACE: toggle select   ENTER: delete",
            "Selected: " + selected, msg);
        msg.clear();

        char ch = _getch();

        if (ch == 27) return;

        if (ch == -32) {
            ch = _getch();
            ux_navigateGraphArrow((unsigned char)ch);
            continue;
        }

        if (ch == 32) { // SPACE toggle
            std::string el = ux_getCursorElement();
            if (!el.empty()) {
                if (marked.count(el)) marked.erase(el);
                else marked.insert(el);
            }
            continue;
        }

        if (ch != 13) continue; // only ENTER below

        if (marked.empty()) {
            msg = "Select at least one vertex to delete.";
            continue;
        }

        // 1) Собираем ВСЁ, что выбрано (и одиночные, и цепочки типа "hij")
        std::vector<std::string> del;
        del.reserve(marked.size());
        for (const auto& s : marked) del.push_back(s);

        // 2) Нормализуем в набор одиночных вершин: "hij" -> {"h","i","j"}
        del = normalizeDeleteList(del);
        if (del.empty()) {
            msg = "Nothing to delete.";
            continue;
        }

        // 3) Проверяем и удаляем уже нормализованный набор
        base_clone = copying_map(base);
        update_clone();

        if (valid_delete(del) != 0) {   // 0 = можно
            msg = "Cannot delete selected set.";
            continue;
        }

        size_t code = deleting_data(del);
        if (code != 0) {
            msg = "Deletion failed.";
            continue;
        }

        update_base();
        parseTaskToRelation();
        task = buildTaskString();

        marked.clear();
        msg = "Deleted.";
    }
}



void editOrderRelation() {
    system("cls");
    int choice = drawEditSubmenu();

    if (choice == 4) return;

    editMode = choice;

    if (editMode == 1) {
        parseTaskToRelation();
        cursorPosition = 0;
        insertStep = 0;
        insertParent = currentRelation.mainPath[cursorPosition];
        insertVertex = "";
        if (cursorPosition < (int)currentRelation.mainPath.size() - 1) {
            insertChild = currentRelation.mainPath[cursorPosition + 1];
        }
        else {
            insertChild = "";
        }
    }
    else if (editMode == 2) {
        parseTaskToRelation();
        cursorPosition = 0;
        cursorLevel = 0;
        branchCursor = 0;
        branchIndex = 0;
        deletePosition = 0;
    }
    else if (editMode == 3) {
        parseTaskToRelation();
        cursorPosition = 0;
        cursorLevel = 0;
        branchCursor = 0;
        branchIndex = 0;
        tempInput = currentRelation.mainPath[cursorPosition];
    }

    interactiveEditRelation();
}

void matrixMenu() {
    initialize();
    Table table(base, road);

    std::vector<std::string> matrixMenuBlock = {
        "~      MATRIX OPERATIONS           ~",
        "+-----------------------------------+",
        "|   Display sum matrix               |",
        "|   Display difference matrix        |",
        "|   Display product matrix           |",
        "|   Exit to main menu                |",
        "+-----------------------------------+"
    };

    int highlight = 2;
    hideCursor();
    printCenteredBlock(matrixMenuBlock, highlight);

    while (true) {
        if (_kbhit()) {
            char ch = _getch();

            if (ch == 27) {
                system("cls");
                return;
            }
            else if (ch == -32) {
                ch = _getch();
                if (ch == 72 && highlight > 2) {
                    --highlight;
                    system("cls");
                    printCenteredBlock(matrixMenuBlock, highlight);
                }
                else if (ch == 80 && highlight < matrixMenuBlock.size() - 2) {
                    ++highlight;
                    system("cls");
                    printCenteredBlock(matrixMenuBlock, highlight);
                }
            }
            else if (ch == 13) {
                int menuIndex = highlight - 2;

                switch (menuIndex) {
                case 0:
                    system("cls");
                    std::cout << "Sum matrix (+):\n";
                    table.printTable('+');
                    break;

                case 1:
                    system("cls");
                    std::cout << "Difference matrix (-):\n";
                    table.printTable('-');
                    break;

                case 2:
                    system("cls");
                    std::cout << "Product matrix (*):\n";
                    table.printTable('*');
                    break;

                case 3:
                    system("cls");
                    return;
                }

                if (menuIndex < 3) {
                    std::cout << "\n(Press any key to continue...)" << std::endl;
                    _getch();
                    system("cls");
                    printCenteredBlock(matrixMenuBlock, highlight);
                }
            }
        }
    }
}

bool simulateDeleteWithPullUp(const std::vector<std::string>& del) {

    std::unordered_set<char> S;
    for (const auto& s : del) if (s.size() == 1) S.insert(s[0]);
    if (S.empty()) return false;

    base_clone = copying_map(base);
    update_clone();
    for (char v : S) {

        std::vector<char> parents;
        {
            auto itP = I_base_clone.find(v);
            if (itP != I_base_clone.end()) {
                std::string pstr = itP->second;
                parents = branch_list(pstr);
            }
        }

        // дети v
        std::vector<char> childs = kidsOf(base_clone, v);

        for (char p : parents) {
            if (S.count(p)) continue;          // родитель тоже удаляется
            for (char c : childs) {
                if (S.count(c)) continue;      // ребёнок тоже удаляется
                addChild(base_clone, p, c);
            }
        }
    }
    for (char v : S) {
        removeVertexEverywhere(base_clone, v);
    }
    update_clone();
    std::string packed;
    packed.reserve(S.size());
    for (char v : S) packed.push_back(v);

    return (valid_delete_vertex(packed) == 0);
}




void choosePrintCenteredBlock(std::vector<std::string> block, int defaultHighLight = 0) {
    int highlightLine = defaultHighLight;
    hideCursor();
    printCenteredBlock(block, highlightLine);
    BlockComp block_comp;

    while (true) {
        if (!_kbhit()) continue;
        char ch = _getch();

        if (ch == 27) {
            // ESC: exit program
            std::cout << "[DEBUG] Exit programm" << std::endl;
            break;
        }

        if (ch == -32) {
            ch = _getch();
            if (ch == 72 && highlightLine > 2) {
                --highlightLine;
                system("cls");
                printCenteredBlock(block, highlightLine);
            }
            else if (ch == 80 && highlightLine < (int)block.size() - 2) {
                ++highlightLine;
                system("cls");
                printCenteredBlock(block, highlightLine);
            }
            continue;
        }

        if (ch != 13) continue;

        int menuIndex = highlightLine - 2;
        switch (menuIndex) {
        case 0: { // Hasse diagram (graph view)
            system("cls");
            parseTaskToRelation();          // чтобы currentRelation был актуален
            task = buildTaskString();

            // сброс курсоров навигации по графу
            cursorPosition = 0;
            cursorLevel = 0;
            branchCursor = 0;
            branchIndex = 0;
            subBranchLevel = 0;
            subBranchIndex = 0;
            subBranchCursor = 0;

            while (true) {
                auto view = ux_buildRelationGraphBlock(
                    "HASSE DIAGRAM",
                    true,    // курсор показывать
                    false,   // tempInput НЕ рисовать
                    "",      // message
                    "ESC: back",
                    ""
                );

                system("cls");
                printCenteredBlock(view, -1);

                char ch2 = _getch();
                if (ch2 == 27) break;               // ESC назад
            }

            system("cls");
            printCenteredBlock(block, highlightLine);
            break;
        }

        case 1: // Create
            system("cls");
            createOrderRelation();
            system("cls");
            printCenteredBlock(block, highlightLine);
            break;

        case 2: // Add vertex
            system("cls");
            addVertexMenu();
            system("cls");
            printCenteredBlock(block, highlightLine);
            break;

        case 3: // Delete vertices
            system("cls");
            deleteVerticesMenu();
            system("cls");
            printCenteredBlock(block, highlightLine);
            break;

        case 4: // Matrices
            system("cls");
            matrixMenu();
            system("cls");
            printCenteredBlock(block, highlightLine);
            break;

        case 5: { // Matrix calculations
            system("cls");
            if (base.empty()) {
                std::cout << "Base relation is empty. Cannot perform matrix calculations.\n";
                std::cout << "\n(Press any key to continue...)" << std::endl;
                _getch();
                system("cls");
                printCenteredBlock(block, highlightLine);
                break;
            }
            initialize();
            Table table(base, road);
            BlockComp block_comp;                 // лучше создать тут, чтобы был "чистый" режим
            printCenteredBlock(block_comp.print());

            std::string keyboard = alph;
            std::string ops = "+*-/";

            while (true) {
                if (!_kbhit()) continue;
                int key = _getch();
                if (key == 27) {
                    system("cls");
                    printCenteredBlock(block, highlightLine);
                    break;
                }                 // ESC

                if (key == 0 || key == 224) {         // служебные (стрелки и т.п.)
                    _getch();
                    continue;
                }

                char c = (char)key;

                // BACKSPACE
                if (key == 8) {
                    if (block_comp.getCursor() == 1) {
                        block_comp.popOperand1();
                    }
                    else if (block_comp.getCursor() == 2) {
                        block_comp.popOperand2();
                    }
                    else if (block_comp.getCursor() == 3) {
                        block_comp.setOperation(' ');
                    }
                    system("cls");
                    printCenteredBlock(block_comp.print());
                    continue;
                }

                // Ввод букв (только английские)
                if (keyboard.find(c) != std::string::npos) {
                    if (block_comp.getCursor() == 1) block_comp.addOperand1(c);
                    else if (block_comp.getCursor() == 2) block_comp.addOperand2(c);
                    system("cls");
                    printCenteredBlock(block_comp.print());
                    continue;
                }

                // Операции (+ - * /)
                if (c == '+' || c == '-' || c == '*' || c == '/') {
                    block_comp.setOperation(c);
                    system("cls");
                    printCenteredBlock(block_comp.print());
                    continue;
                }

                // ENTER -> следующий курсор / посчитать
                if (key == 13) {
                    int res = block_comp.nextCursor();
                    if (res) {
                        block_comp.setResult(table.pomogi(
                            block_comp.getOperand1(),
                            block_comp.getOperand2(),
                            block_comp.getOperation()
                        ));
                    }
                    system("cls");
                    printCenteredBlock(block_comp.print());
                }
            }
			break;
        }
        case 6: 
            return;
        }
    }
}