#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <queue>

std::unordered_map<char, std::string> buildInvertedBase(std::unordered_map<char, std::string>& bs);
void update_inverted_dist();
void buildHasseDiagram();

// ==================== СТРУКТУРЫ ДАННЫХ ====================

// Для редактора отношений
struct HasseNode {
    char symbol;
    int level;
    std::vector<char> children;
    std::vector<char> parents;
};

// Для операций вставки
struct insert_data {
    char new_object;
    char parent;
    std::string childs;
};

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================

// Основные структуры (общие)
std::string task = "b d g e c/f g h a";
std::unordered_map<char, std::string> base;
std::unordered_map<char, std::string> I_base;
std::vector<std::string> road = { "a" };
std::vector<HasseNode> hasseNodes;

// Структуры для операций 
std::unordered_map<char, std::string> default_base{
    { 'a', "b" },   { 'b', "d" }, { 'c', "g" },   { 'd', "e" },
    { 'e', "c/f" }, { 'f', "g" }, { 'g', "h" },   { 'h', "a" }
};
std::unordered_map<char, std::string> base_clone;
std::unordered_map<char, std::string> I_base_clone;
std::unordered_map<char, size_t> road_size;
std::unordered_map<char, size_t> road_clone_size;
std::unordered_map<char, size_t> inverted_road_clone_size;
std::string alph;
std::string alph_clone;

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

std::vector<char> branch_list(std::string branch) {
    size_t pos = 0;
    std::vector<char> branches;
    std::string tmp;

    while ((pos = branch.find('/')) != std::string::npos) {
        tmp = branch.substr(0, pos);
        branches.push_back(tmp[0]);
        branch.erase(0, 2);
    }
    tmp = branch.substr(0);
    if (!tmp.empty()) branches.push_back(tmp[0]);

    return branches;
}

inline std::string alphabet(const std::unordered_map<char, std::string>& bs) {
    // dynamic vertex universe: all keys + all children, ordered a..z then A..Z
    auto key = [](char c) {
        if (c >= 'a' && c <= 'z') return (int)(c - 'a');
        if (c >= 'A' && c <= 'Z') return 26 + (int)(c - 'A');
        return 1000 + (unsigned char)c;
        };
    std::unordered_set<char> seen;
    std::vector<char> v;
    v.reserve(bs.size() * 2);

    for (const auto& kv : bs) {
        if (seen.insert(kv.first).second) v.push_back(kv.first);
    }
    for (const auto& kv : bs) {
        std::vector<char> kids = branch_list(kv.second);
        for (char c : kids) {
            if (seen.insert(c).second) v.push_back(c);
        }
    }
    std::sort(v.begin(), v.end(), [&](char a, char b) { return key(a) < key(b); });

    std::string out;
    out.reserve(v.size());
    for (char c : v) out.push_back(c);
    return out;
}

// Overload for non-const maps (kept for backward compatibility)
inline std::string alphabet(std::unordered_map<char, std::string>& bs) {
    return alphabet(static_cast<const std::unordered_map<char, std::string>&>(bs));
}


std::unordered_map<char, size_t> calc_road_size(std::unordered_map<char, std::string>& bs) {
    std::unordered_map<char, size_t> road_size;
    if (bs.empty()) return road_size;

    auto key = [](char c) {
        if (c >= 'a' && c <= 'z') return (int)(c - 'a');
        if (c >= 'A' && c <= 'Z') return 26 + (int)(c - 'A');
        return 1000 + (unsigned char)c;
        };

    // pick root: 'a' if present; else indegree==0; else min by key
    std::unordered_map<char, int> indeg;
    for (auto& kv : bs) indeg.try_emplace(kv.first, 0);
    for (auto& kv : bs) {
        for (char c : branch_list(kv.second)) indeg[c]++;
    }
    char root = 0;
    if (bs.find('a') != bs.end()) root = 'a';
    if (!root) {
        for (auto& kv : indeg) {
            if (kv.second == 0) {
                if (!root || key(kv.first) < key(root)) root = kv.first;
            }
        }
    }
    if (!root) {
        for (auto& kv : indeg) {
            if (!root || key(kv.first) < key(root)) root = kv.first;
        }
    }

    // BFS
    std::vector<char> q;
    std::unordered_set<char> vis;
    q.push_back(root);
    vis.insert(root);
    road_size[root] = 0;
    for (size_t i = 0; i < q.size(); ++i) {
        char u = q[i];
        for (char v : branch_list(bs[u])) {
            if (vis.insert(v).second) {
                road_size[v] = road_size[u] + 1;
                q.push_back(v);
            }
        }
    }
    return road_size;
}

std::unordered_map<char, std::string> copying_map(std::unordered_map<char, std::string>& bs) {
    std::unordered_map<char, std::string> base;
    for (auto it = bs.begin(); it != bs.end(); ++it) {
        base[it->first] = bs[it->first];
    }
    return base;
}

void update_base() {
    base = copying_map(base_clone);
    I_base = buildInvertedBase(base);  // Исправлено: добавлен параметр
    road_size = calc_road_size(base);
    alph = alphabet(base);
    buildHasseDiagram(); // Обновляем диаграмму Хассе
}

void update_clone() {
    road_clone_size = calc_road_size(base_clone);
    I_base_clone = buildInvertedBase(base_clone);  // Исправлено: добавлен параметр
    alph_clone = alphabet(base_clone);
    update_inverted_dist();  // Исправлено: добавлен вызов
}

void childs_format(std::string& childs) {
    size_t pos = 0;
    while (childs.find("//") != std::string::npos) {
        pos = childs.find("//");
        childs.replace(pos, pos + 1, "/");
    }
    if (!childs.empty()) {
        if (childs[0] == '/') childs.erase(0);
        if ((!childs.empty()) && (childs[childs.length() - 1] == '/'))
            childs.erase(childs.length() - 1);
    }
}

std::string erase_childs(std::string all_childs, std::vector<char>& child_to_delete) {
    size_t pos = 0;
    std::string result = all_childs;
    for (char child : child_to_delete) {
        pos = result.find(child);
        if (pos != std::string::npos) {
            result.erase(pos, 1);
        }
    }

    childs_format(result);
    return result;
}

void update_inverted_dist() {
    std::unordered_map<char, size_t> result;
    if (alph_clone.empty() || I_base_clone.empty()) return;

    std::string last_objects = I_base_clone[alph_clone[0]];
    std::vector<char> currents = branch_list(I_base_clone[alph_clone[0]]);
    std::queue<char> queue;
    char cur_object;

    for (char current : currents) {
        queue.push(current);
        result[current] = 0;
    }

    while (!queue.empty()) {
        cur_object = queue.front();
        queue.pop();
        std::string next_objects = I_base_clone[cur_object];
        std::vector<char> cur_branches = branch_list(next_objects);

        for (char object : cur_branches) {
            if (last_objects.find(object) == std::string::npos) {
                result[object] = result[cur_object] + 1;
                queue.push(object);
            }
        }
    }
    inverted_road_clone_size = result;
}

// ==================== ФУНКЦИИ ВАЛИДАЦИИ ====================

std::vector<char> find_neighbours(char object) {
    if (alph_clone.empty() || road_clone_size.empty()) return {};
    size_t current_level = road_clone_size[object];
    std::vector<char> result;
    for (char obj_clone : alph_clone)
        if ((road_clone_size[obj_clone] == current_level) && (obj_clone != object))
            result.push_back(obj_clone);

    return result;
}

std::vector<char> get_childs(std::vector<char>& parents) {
    std::vector<char> result;
    std::vector<char> childs_branch;

    for (char object : parents) {
        if (base_clone.find(object) != base_clone.end()) {
            childs_branch = branch_list(base_clone[object]);
            result.insert(result.end(), childs_branch.begin(), childs_branch.end());
        }
    }

    return result;
}

bool find_gate(char parent, char child) {
    if (base_clone.find(parent) != base_clone.end()) {
        return base_clone[parent].find(child) != std::string::npos;
    }
    return false;
}

int valid_insert_vertex(insert_data data) {
    int valid = 0;
    std::vector<char> neighbours = find_neighbours(data.new_object);
    std::vector<char> childs_neighbours = get_childs(neighbours);
    std::vector<char> childs_list = branch_list(data.childs);

    for (char child : childs_list)
        for (char child_neighbour : childs_neighbours) {
            if (road_clone_size.find(child) != road_clone_size.end() &&
                road_clone_size.find(child_neighbour) != road_clone_size.end() &&
                inverted_road_clone_size.find(child) != inverted_road_clone_size.end() &&
                inverted_road_clone_size.find(child_neighbour) != inverted_road_clone_size.end()) {
                if ((road_clone_size[child] != road_clone_size[child_neighbour]) ||
                    (inverted_road_clone_size[child] != inverted_road_clone_size[child_neighbour]))
                    valid = 1;
            }
        }

    return valid;
}

int valid_insert(std::vector<insert_data> ins_d) {
    update_clone();
    int code = 0;
    for (auto data : ins_d) {
        code += valid_insert_vertex(data);
    }

    return code;
}

int valid_delete_vertex(std::string delete_v) {
    auto willDelete = [&](char x) { return delete_v.find(x) != std::string::npos; };

    // собрать поддерево из base_clone (включая root)
    auto collectSubtree = [&](char root) {
        std::unordered_set<char> out;
        std::vector<char> st{ root };

        while (!st.empty()) {
            char v = st.back();
            st.pop_back();
            if (out.count(v)) continue;
            out.insert(v);

            auto it = base_clone.find(v);
            if (it == base_clone.end()) continue;

            std::string tmp = it->second; // branch_list мутирует строку
            std::vector<char> kids = branch_list(tmp);
            for (char k : kids) st.push_back(k);
        }
        return out;
        };

    // RULE A: удаляем вершину с ветвлением (2+ детей) -> только полный снос её поддерева
    for (char v : delete_v) {
        auto it = base_clone.find(v);
        if (it == base_clone.end()) continue;

        std::string tmp = it->second;
        std::vector<char> kids = branch_list(tmp);
        if (kids.size() >= 2) {
            auto sub = collectSubtree(v);
            for (char x : sub) if (!willDelete(x)) return 1;
        }
    }

    // RULE B: если родитель ветвится и удаляем его ребёнка c -> только полный снос поддерева c
    for (const auto& kv : base_clone) {
        std::string tmp = kv.second;
        std::vector<char> kids = branch_list(tmp);
        if (kids.size() < 2) continue;

        for (char c : kids) {
            if (!willDelete(c)) continue;
            auto sub = collectSubtree(c);
            for (char x : sub) if (!willDelete(x)) return 1;
        }
    }

    // Старая проверка — ДОЛЖНА быть для каждого удаляемого v
    for (char v : delete_v) {
        std::vector<char> neighbours = find_neighbours(v);
        std::vector<char> childs_neighbours = get_childs(neighbours);

        auto it = base_clone.find(v);
        if (it == base_clone.end()) continue;

        std::string tmp = it->second;
        std::vector<char> childs_list = branch_list(tmp);

        for (char child : childs_list) {
            if (willDelete(child)) continue; // не сравниваем удаляемых

            for (char child_neighbour : childs_neighbours) {
                if (willDelete(child_neighbour)) continue; // не сравниваем удаляемых

                if (road_clone_size.find(child) != road_clone_size.end() &&
                    road_clone_size.find(child_neighbour) != road_clone_size.end() &&
                    inverted_road_clone_size.find(child) != inverted_road_clone_size.end() &&
                    inverted_road_clone_size.find(child_neighbour) != inverted_road_clone_size.end()) {

                    if ((road_clone_size[child] != road_clone_size[child_neighbour]) ||
                        (inverted_road_clone_size[child] != inverted_road_clone_size[child_neighbour]))
                        return 1;
                }
            }
        }
    }

    return 0;
}


static std::vector<std::string> normalizeDeleteList(const std::vector<std::string>& del) {
    std::unordered_set<char> seen;
    std::vector<std::string> out;
    for (const auto& s : del) {
        for (unsigned char ch : s) {
            if (!std::isalpha(ch)) continue;
            char v = (char)std::tolower(ch);
            if (seen.insert(v).second) out.push_back(std::string(1, v));
        }
    }
    return out;
}



int valid_delete(const std::vector<std::string>& del) {
    auto norm = normalizeDeleteList(del);

    std::string packed;
    packed.reserve(norm.size());
    for (const auto& v : norm) packed.push_back(v[0]);

    return valid_delete_vertex(packed);
}

static std::vector<char> kidsOf(const std::unordered_map<char, std::string>& mp, char v) {
    auto it = mp.find(v);
    if (it == mp.end()) return {};
    std::string s = it->second;            // branch_list мутирует строку, поэтому копия
    return branch_list(s);
}

static bool containsChild(const std::string& s, char c) {
    // у тебя формат "b/c/d" (один символ на элемент)
    // поэтому просто ищем как символ, но аккуратно:
    for (char x : s) if (x == c) return true;
    return false;
}

static void addChild(std::unordered_map<char, std::string>& mp, char parent, char child) {
    if (parent == child) return;
    auto it = mp.find(parent);
    if (it == mp.end()) {
        mp[parent] = std::string(1, child);
        return;
    }
    if (!containsChild(it->second, child)) {
        if (!it->second.empty()) it->second.push_back('/');
        it->second.push_back(child);
    }
}

static void removeChild(std::unordered_map<char, std::string>& mp, char parent, char child) {
    auto it = mp.find(parent);
    if (it == mp.end()) return;
    std::vector<char> kids = kidsOf(mp, parent);
    std::string out;
    for (char k : kids) {
        if (k == child) continue;
        if (!out.empty()) out.push_back('/');
        out.push_back(k);
    }
    if (out.empty()) mp.erase(parent);
    else it->second = out;
}

static void removeVertexEverywhere(std::unordered_map<char, std::string>& mp, char v) {
    // убрать v из списков детей у всех
    std::vector<char> keys;
    keys.reserve(mp.size());
    for (auto& kv : mp) keys.push_back(kv.first);
    for (char p : keys) removeChild(mp, p, v);

    // убрать исходящие v (ключ)
    mp.erase(v);
}



// ==================== ФУНКЦИИ ОПЕРАЦИЙ ====================

void replace_vertex_base_clone(char object, char parent, std::vector<char>& childs_list, std::string childs_in_string) {
    if (base_clone.find(parent) != base_clone.end()) {
        std::string childs = base_clone[parent];
        childs = erase_childs(childs, childs_list);

        if (childs == "")
            base_clone[parent] = object;
        else
            base_clone[parent] += "/" + object;
    }

    if (base_clone.find(object) != base_clone.end()) {
        base_clone[object] = base_clone[object].empty() ? childs_in_string : base_clone[object] + "/" + childs_in_string;
    }
    else {
        base_clone[object] = childs_in_string;
    }
}

void insert_vertex_base_clone(insert_data data) {
    std::vector<char> childs_list = branch_list(data.childs);
    bool replacement = true;

    if (childs_list.empty()) replacement = false;
    for (char child : childs_list)
        if (!find_gate(data.parent, child)) replacement = false;

    if (replacement) {
        replace_vertex_base_clone(data.new_object, data.parent, childs_list, data.childs);
    }
    else {
        if (base_clone.find(data.parent) == base_clone.end() ||
            base_clone[data.parent].empty() ||
            (alph_clone.length() > 0 && base_clone[data.parent][0] == alph_clone[0])) {
            base_clone[data.parent] = data.new_object;
            if (!alph_clone.empty()) {
                base_clone[data.new_object] = std::string(1, alph_clone[0]);
            }
        }
        else {
            base_clone[data.parent] += "/" + data.new_object;
            if (base_clone.find(data.new_object) == base_clone.end()) {
                base_clone[data.new_object] = data.childs;
            }
            else {
                base_clone[data.new_object] = (base_clone[data.new_object].empty() ? data.childs : base_clone[data.new_object] + "/" + data.childs);
            }
        }
    }
}

void insert_base_clone(std::vector<insert_data> all_data) {
    for (auto data : all_data) {
        insert_vertex_base_clone(data);
    }
}

size_t clearing_part_of_layer(std::vector<char>& objects_to_del) {
    size_t pos = 0;
    std::string tmp2;

    for (char object : objects_to_del) {
        if (I_base_clone.find(object) != I_base_clone.end()) {
            std::vector<char> parents = branch_list(I_base_clone[object]);
            for (char parent : parents) {
                if (base_clone.find(parent) != base_clone.end()) {
                    tmp2 = base_clone[parent];
                    if (tmp2.length() > 1) {
                        pos = tmp2.find(object);
                        if (pos != std::string::npos) {
                            if (pos == tmp2.length() - 1)
                                base_clone[parent] = tmp2.erase(pos - 1, 2);
                            else
                                base_clone[parent] = tmp2.erase(pos, 2);
                        }
                    }
                    else
                        return 1;
                }
            }
        }
    }

    for (char object : objects_to_del) {
        base_clone.erase(object);
        road_clone_size.erase(object);
    }

    return 0;
}

void clearing_all_layer(std::vector<char>& objects_to_del) {
    std::string tmp2;

    for (char object : objects_to_del) {
        if (I_base_clone.find(object) != I_base_clone.end()) {
            std::vector<char> parents = branch_list(I_base_clone[object]);
            for (char parent : parents) {
                if (base_clone.find(parent) != base_clone.end()) {
                    base_clone[parent].clear();
                }
            }
        }
    }

    for (char object : objects_to_del) {
        if (I_base_clone.find(object) != I_base_clone.end()) {
            std::vector<char> parents = branch_list(I_base_clone[object]);
            for (char parent : parents) {
                if (base_clone.find(parent) != base_clone.end()) {
                    if (base_clone[parent].empty() && base_clone.find(object) != base_clone.end()) {
                        base_clone[parent] = base_clone[object];
                    }
                    else if (base_clone.find(object) != base_clone.end()) {
                        base_clone[parent] += "/" + base_clone[object];
                    }
                }
            }
        }
    }

    for (char object : objects_to_del) {
        base_clone.erase(object);
        road_clone_size.erase(object);
    }
}

size_t deleting_object(std::string obj, bool layer_clear) {
    size_t code = 0;
    std::vector<char> objects_to_del = branch_list(obj);

    if (layer_clear) {
        clearing_all_layer(objects_to_del);
    }
    else {
        code = clearing_part_of_layer(objects_to_del);
    }

    return code;
}

size_t deleting_data(std::vector<std::string> delete_data) {
    std::string all_deleted_objects;
    for (const auto& obj : delete_data) all_deleted_objects += obj;

    for (const auto& object : delete_data) {
        bool layer_clear = true;

        // важно: road_clone_size/alph_clone должны быть актуальны ДО расчёта слоя
        if (!all_deleted_objects.empty() && !alph_clone.empty() &&
            road_clone_size.find(object[0]) != road_clone_size.end()) {

            size_t cur_layer = road_clone_size[object[0]];
            for (char a : alph_clone) {
                if (road_clone_size.find(a) != road_clone_size.end() &&
                    road_clone_size[a] == cur_layer &&
                    all_deleted_objects.find(a) == std::string::npos) {
                    layer_clear = false;
                    break;
                }
            }
        }

        size_t code = deleting_object(object, layer_clear);
        if (code != 0) return code;   // <-- НОВОЕ: не затирать и не продолжать

        update_clone();               // <-- НОВОЕ: граф изменился, пересчитать слои/алфавит
    }

    return 0;
}


// ==================== ОБЩИЕ ФУНКЦИИ ====================

void buildBase() {
    // Dynamic version: base/base_clone are the source of truth.
    // We keep this function for compatibility with old call sites.
    if (!base_clone.empty()) {
        base = copying_map(base_clone);
    }
    if (base.empty()) {
        // Fallback to a minimal graph if nothing is initialized yet.
        base['a'] = "a";
    }
    // Sync clones/alphabets
    base_clone = copying_map(base);
    alph = alphabet(base);
    alph_clone = alphabet(base_clone);
}

std::unordered_map<char, std::string> buildInvertedBase(std::unordered_map<char, std::string>& bs) {
    std::unordered_map<char, std::string> Inv_base;
    for (const auto& pair : bs) {
        std::string key{ pair.first };
        const std::string& value = pair.second;

        std::istringstream ss(value);
        std::string vertex;
        while (std::getline(ss, vertex, '/')) {
            if (!vertex.empty()) {
                if (Inv_base.find(vertex[0]) != Inv_base.end()) {
                    Inv_base[vertex[0]] += "/" + key;
                }
                else {
                    Inv_base[vertex[0]] = key;
                }
            }
        }
    }
    return Inv_base;
}

void buildHasseStructure() {
    hasseNodes.clear();

    for (char c : alph_clone) {
        HasseNode node;
        node.symbol = c;
        node.level = 0;
        hasseNodes.push_back(node);
    }

    for (size_t i = 0; i < road.size(); i++) {
        const std::string& levelStr = road[i];
        for (char c : levelStr) {
            auto it = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                [c](const HasseNode& node) { return node.symbol == c; });
            if (it != hasseNodes.end()) {
                it->level = i;
            }
        }
    }

    for (auto& node : hasseNodes) {
        if (base.find(node.symbol) != base.end()) {
            const std::string& childrenStr = base[node.symbol];

            if (childrenStr.size() == 1) {
                char childChar = childrenStr[0];
                node.children.push_back(childChar);

                auto childIt = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                    [childChar](const HasseNode& n) { return n.symbol == childChar; });
                if (childIt != hasseNodes.end()) {
                    childIt->parents.push_back(node.symbol);
                }
            }
            else if (childrenStr.size() == 3 && childrenStr[1] == '/') {
                char leftChar = childrenStr[0];
                char rightChar = childrenStr[2];

                node.children.push_back(leftChar);
                node.children.push_back(rightChar);

                auto leftIt = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                    [leftChar](const HasseNode& n) { return n.symbol == leftChar; });
                auto rightIt = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                    [rightChar](const HasseNode& n) { return n.symbol == rightChar; });

                if (leftIt != hasseNodes.end()) {
                    leftIt->parents.push_back(node.symbol);
                }
                if (rightIt != hasseNodes.end()) {
                    rightIt->parents.push_back(node.symbol);
                }
            }
            else if (childrenStr.find('/') != std::string::npos) {
                // Обработка множественных ветвлений
                std::vector<char> children = branch_list(childrenStr);
                for (char childChar : children) {
                    node.children.push_back(childChar);
                    auto childIt = std::find_if(hasseNodes.begin(), hasseNodes.end(),
                        [childChar](const HasseNode& n) { return n.symbol == childChar; });
                    if (childIt != hasseNodes.end()) {
                        childIt->parents.push_back(node.symbol);
                    }
                }
            }
        }
    }
}

void buildHasseDiagram() {
    // Build `road` as a list of BFS levels so every vertex is assigned a level.
    // Table.h expects `road` to contain all vertices (possibly grouped per level).
    road.clear();

    // Ensure road_size is populated.
    road_size = calc_road_size(base);

    // Collect all vertices: keys + children.
    std::unordered_set<char> all;
    for (auto& kv : base) {
        all.insert(kv.first);
        for (char c : branch_list(kv.second)) all.insert(c);
    }
    if (all.empty()) {
        road.push_back("a");
        buildHasseStructure();
        return;
    }

    // Find max level among known vertices.
    size_t maxLevel = 0;
    for (char v : all) {
        auto it = road_size.find(v);
        if (it != road_size.end()) maxLevel = max(maxLevel, it->second);
    }

    // Any vertices not reachable from the chosen root get placed on a new last level.
    bool hasUnreached = false;
    for (char v : all) {
        if (road_size.find(v) == road_size.end()) { hasUnreached = true; break; }
    }
    size_t levels = maxLevel + 1 + (hasUnreached ? 1 : 0);
    road.assign(levels, "");

    // Stable order inside each level: by alphabet() ordering.
    std::string alph = alphabet(base);
    for (char v : alph) {
        size_t lvl = 0;
        auto it = road_size.find(v);
        if (it != road_size.end()) lvl = it->second;
        else lvl = levels - 1;
        road[lvl].push_back(v);
    }

    buildHasseStructure();
}

int getMaxLevel() {
    int maxLevel = 0;
    for (const auto& node : hasseNodes) {
        if (node.level > maxLevel) maxLevel = node.level;
    }
    return maxLevel;
}

// ==================== ФУНКЦИИ ДЛЯ ИНТЕГРАЦИИ С РЕДАКТОРОМ ====================

// Проверка возможности вставки вершины
bool canInsertVertex(char newVertex, char parent, const std::string& childs) {
    insert_data data;
    data.new_object = newVertex;
    data.parent = parent;
    data.childs = childs;
    std::vector<insert_data> check_data = { data };
    return valid_insert(check_data) == 0;  // 0 = можно вставить
}

// Проверка возможности удаления вершины
bool canDeleteVertices(const std::vector<std::string>& vertices) {
    base_clone = copying_map(base);
    update_clone();
    return valid_delete(vertices) == 0;
}

bool executeDeleteMany(const std::vector<std::string>& vertices) {
    // deleting_data у тебя уже принимает список
    base_clone = copying_map(base);
    update_clone();

    if (valid_delete(vertices) != 0) return false;

    size_t result = deleting_data(vertices);
    if (result == 0) {
        update_base();
        return true;
    }
    return false;
}

// Выполнение вставки вершины
void executeInsert(char newVertex, char parent, const std::string& childs) {
    insert_data data;
    data.new_object = newVertex;
    data.parent = parent;
    data.childs = childs;
    std::vector<insert_data> insert_list = { data };
    insert_base_clone(insert_list);
    update_base();  // Обновляем основные структуры
}

// Выполнение удаления вершины
bool executeDelete(const std::string& vertex) {
    std::vector<std::string> delete_list = { vertex };
    size_t result = deleting_data(delete_list);
    if (result == 0) {
        update_base();  // Обновляем только если успешно
        return true;
    }
    return false;
}

// ==================== СТАНДАРТНЫЕ ФУНКЦИИ ====================

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


std::tuple<std::string, std::string, char> textGetVal() {
    std::cout << "\033[2J\033[1;1H";
    std::cout << "Enter first operand: ";

    std::string allowedChars = alphabet(base_clone);
    if (allowedChars.empty()) allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string op1 = inputCorrect(allowedChars + "-", "Incorrect input, try again: ");

    std::cout << "Enter second operand: ";
    std::string op2 = inputCorrect(allowedChars + "-", "Incorrect input, try again: ");

    std::cout << "Enter operation (+, -, *, /): ";
    char sign = inputCorrect("*+-/", "Incorrect operation: ")[0];

    return { op1, op2, sign };
}

void initialize() {
    buildBase();
    I_base = buildInvertedBase(base);  // Исправлено: добавлен параметр
    buildHasseDiagram();
}