#include <windows.h>
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

string trim(const string& text) {
    size_t begin = text.find_first_not_of(" \t\r\n");
    if (begin == string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

long long parseInteger(const string& text) {
    string s = trim(text);
    if (s.empty()) throw invalid_argument("Порожнє число.");
    size_t used = 0;
    long long value = stoll(s, &used, 10);
    if (used != s.size()) throw invalid_argument("Некоректне ціле число: " + s);
    return value;
}

string readLine(const string& prompt) {
    cout << prompt;
    string s;
    if (!getline(cin, s)) {
        cout << "\nВвід завершено.\n";
        exit(0);
    }
    return trim(s);
}

int readInt(const string& prompt, int lower, int upper) {
    while (true) {
        string s = readLine(prompt);
        try {
            long long n = parseInteger(s);
            if (n >= lower && n <= upper) return static_cast<int>(n);
        }
        catch (const exception&) {
        }
        cout << "Помилка: введіть ціле число від " << lower
            << " до " << upper << ".\n";
    }
}

void deleteTree(TreeNode* root) {
    if (!root) return;
    stack<TreeNode*> nodes;
    nodes.push(root);
    while (!nodes.empty()) {
        TreeNode* node = nodes.top();
        nodes.pop();
        if (node->left) nodes.push(node->left);
        if (node->right) nodes.push(node->right);
        delete node;
    }
}

struct ValueToken {
    bool isNull;
    int value;
    ValueToken(bool isNullValue, int number) : isNull(isNullValue), value(number) {}
};

vector<ValueToken> parseTreeTokens(const string& input, int minValue, int maxValue) {
    string s = trim(input);
    if (s.empty()) throw invalid_argument("Введіть дерево або [].");
    if (s[0] == '[' || s[s.size() - 1] == ']') {
        if (s.size() < 2 || s.front() != '[' || s.back() != ']')
            throw invalid_argument("Неправильні квадратні дужки.");
        s = s.substr(1, s.size() - 2);
    }
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '[' || s[i] == ']')
            throw invalid_argument("Зайві квадратні дужки.");
        if (s[i] == ',') s[i] = ' ';
    }
    istringstream stream(s);
    vector<ValueToken> result;
    string part;
    while (stream >> part) {
        string lower = part;
        for (size_t i = 0; i < lower.size(); ++i)
            lower[i] = static_cast<char>(tolower(static_cast<unsigned char>(lower[i])));
        if (lower == "null" || lower == "#") {
            result.push_back(ValueToken(true, 0));
        }
        else {
            long long n = parseInteger(part);
            if (n < minValue || n > maxValue)
                throw invalid_argument("Значення вузла не відповідає обмеженням задачі.");
            result.push_back(ValueToken(false, static_cast<int>(n)));
        }
    }
    return result;
}

TreeNode* buildTree(const string& input, int maxNodes, int minValue, int maxValue) {
    vector<ValueToken> values = parseTreeTokens(input, minValue, maxValue);
    int count = 0;
    for (size_t i = 0; i < values.size(); ++i)
        if (!values[i].isNull) ++count;
    if (count > maxNodes) throw invalid_argument("Завелика кількість вузлів.");
    if (values.empty()) return nullptr;
    if (values[0].isNull) {
        if (count != 0) throw invalid_argument("Після порожнього кореня не може бути вузлів.");
        return nullptr;
    }

    TreeNode* root = new TreeNode(values[0].value);
    queue<TreeNode*> parents;
    parents.push(root);
    size_t pos = 1;
    try {
        while (!parents.empty() && pos < values.size()) {
            TreeNode* parent = parents.front();
            parents.pop();
            if (pos < values.size() && !values[pos].isNull) {
                parent->left = new TreeNode(values[pos].value);
                parents.push(parent->left);
            }
            ++pos;
            if (pos < values.size() && !values[pos].isNull) {
                parent->right = new TreeNode(values[pos].value);
                parents.push(parent->right);
            }
            ++pos;
        }
        while (pos < values.size()) {
            if (!values[pos].isNull)
                throw invalid_argument("Зайвий вузол, для якого немає батька.");
            ++pos;
        }
    }
    catch (...) {
        deleteTree(root);
        throw;
    }
    return root;
}

TreeNode* readTree(const string& prompt, int maxNodes, int minValue, int maxValue,
    bool allowEmpty = true) {
    while (true) {
        string input = readLine(prompt);
        try {
            TreeNode* root = buildTree(input, maxNodes, minValue, maxValue);
            if (!root && !allowEmpty) {
                cout << "Помилка: для цієї задачі дерево має бути непорожнім.\n";
                continue;
            }
            return root;
        }
        catch (const exception& ex) {
            cout << "Помилка введення дерева: " << ex.what() << "\n";
            cout << "Формат: [1,2,3,null,4] або [] для порожнього дерева.\n";
        }
    }
}

bool isSameTree(TreeNode* p, TreeNode* q) {
    stack<pair<TreeNode*, TreeNode*> > pending;
    pending.push(make_pair(p, q));
    while (!pending.empty()) {
        pair<TreeNode*, TreeNode*> current = pending.top();
        pending.pop();
        if (!current.first && !current.second) continue;
        if (!current.first || !current.second) return false;
        if (current.first->val != current.second->val) return false;
        pending.push(make_pair(current.first->left, current.second->left));
        pending.push(make_pair(current.first->right, current.second->right));
    }
    return true;
}

bool isMirror(TreeNode* left, TreeNode* right) {
    stack<pair<TreeNode*, TreeNode*> > pending;
    pending.push(make_pair(left, right));
    while (!pending.empty()) {
        pair<TreeNode*, TreeNode*> current = pending.top();
        pending.pop();
        if (!current.first && !current.second) continue;
        if (!current.first || !current.second || current.first->val != current.second->val)
            return false;
        pending.push(make_pair(current.first->left, current.second->right));
        pending.push(make_pair(current.first->right, current.second->left));
    }
    return true;
}

bool isSymmetric(TreeNode* root) {
    return root == nullptr || isMirror(root->left, root->right);
}

TreeNode* invertTree(TreeNode* root) {
    if (!root) return nullptr;
    stack<TreeNode*> nodes;
    nodes.push(root);
    while (!nodes.empty()) {
        TreeNode* current = nodes.top();
        nodes.pop();
        TreeNode* tmp = current->left;
        current->left = current->right;
        current->right = tmp;
        if (current->left) nodes.push(current->left);
        if (current->right) nodes.push(current->right);
    }
    return root;
}

bool isValidBST(TreeNode* root) {
    stack<TreeNode*> pending;
    TreeNode* current = root;
    long long previous = 0;
    bool hasPrevious = false;
    while (current || !pending.empty()) {
        while (current) {
            pending.push(current);
            current = current->left;
        }
        current = pending.top();
        pending.pop();
        if (hasPrevious && current->val <= previous) return false;
        previous = current->val;
        hasPrevious = true;
        current = current->right;
    }
    return true;
}

int countNodes(TreeNode* root) {
    if (!root) return 0;
    int count = 0;
    stack<TreeNode*> pending;
    pending.push(root);
    while (!pending.empty()) {
        TreeNode* current = pending.top();
        pending.pop();
        ++count;
        if (current->left) pending.push(current->left);
        if (current->right) pending.push(current->right);
    }
    return count;
}

int kthSmallest(TreeNode* root, int k) {
    stack<TreeNode*> pending;
    TreeNode* current = root;
    while (current || !pending.empty()) {
        while (current) {
            pending.push(current);
            current = current->left;
        }
        current = pending.top();
        pending.pop();
        if (--k == 0) return current->val;
        current = current->right;
    }
    throw invalid_argument("Значення k перевищує кількість вузлів.");
}

string serialize(TreeNode* root) {
    if (!root) return "[]";
    vector<string> values;
    queue<TreeNode*> nodes;
    nodes.push(root);
    while (!nodes.empty()) {
        TreeNode* node = nodes.front();
        nodes.pop();
        if (!node) {
            values.push_back("null");
        }
        else {
            values.push_back(to_string(node->val));
            nodes.push(node->left);
            nodes.push(node->right);
        }
    }
    while (!values.empty() && values.back() == "null") values.pop_back();
    string result = "[";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i != 0) result += ",";
        result += values[i];
    }
    return result + "]";
}

TreeNode* deserialize(const string& data) {
    return buildTree(data, 10000, -1000, 1000);
}

int maxPathSum(TreeNode* root) {
    if (!root) throw invalid_argument("Порожнє дерево не має непорожніх шляхів.");
    int answer = INT_MIN;
    unordered_map<TreeNode*, int> gains;
    stack<pair<TreeNode*, bool> > pending;
    pending.push(make_pair(root, false));
    while (!pending.empty()) {
        pair<TreeNode*, bool> item = pending.top();
        pending.pop();
        TreeNode* node = item.first;
        if (!node) continue;
        if (!item.second) {
            pending.push(make_pair(node, true));
            pending.push(make_pair(node->right, false));
            pending.push(make_pair(node->left, false));
        }
        else {
            int l = node->left ? gains[node->left] : 0;
            int r = node->right ? gains[node->right] : 0;
            if (l < 0) l = 0;
            if (r < 0) r = 0;
            int sum = node->val + l + r;
            if (sum > answer) answer = sum;
            gains[node] = node->val + (l > r ? l : r);
        }
    }
    return answer;
}

int minCameraCover(TreeNode* root) {
    if (!root) return 0;
    int cameras = 0;
    unordered_map<TreeNode*, int> state;
    stack<pair<TreeNode*, bool> > pending;
    pending.push(make_pair(root, false));
    while (!pending.empty()) {
        pair<TreeNode*, bool> item = pending.top();
        pending.pop();
        TreeNode* node = item.first;
        if (!node) continue;
        if (!item.second) {
            pending.push(make_pair(node, true));
            pending.push(make_pair(node->right, false));
            pending.push(make_pair(node->left, false));
        }
        else {
            int left = node->left ? state[node->left] : 2;
            int right = node->right ? state[node->right] : 2;
            if (left == 0 || right == 0) {
                ++cameras;
                state[node] = 1;
            }
            else if (left == 1 || right == 1) {
                state[node] = 2;
            }
            else {
                state[node] = 0;
            }
        }
    }
    if (state[root] == 0) ++cameras;
    return cameras;
}

vector<vector<int> > verticalTraversal(TreeNode* root) {
    vector<vector<int> > result;
    if (!root) return result;
    vector<tuple<int, int, int> > locations;
    queue<tuple<TreeNode*, int, int> > pending;
    pending.push(make_tuple(root, 0, 0));
    while (!pending.empty()) {
        tuple<TreeNode*, int, int> item = pending.front();
        pending.pop();
        TreeNode* node = get<0>(item);
        int row = get<1>(item);
        int col = get<2>(item);
        locations.push_back(make_tuple(col, row, node->val));
        if (node->left) pending.push(make_tuple(node->left, row + 1, col - 1));
        if (node->right) pending.push(make_tuple(node->right, row + 1, col + 1));
    }
    sort(locations.begin(), locations.end());
    int prevCol = 0;
    bool first = true;
    for (size_t i = 0; i < locations.size(); ++i) {
        int col = get<0>(locations[i]);
        if (first || col != prevCol) {
            result.push_back(vector<int>());
            first = false;
            prevCol = col;
        }
        result.back().push_back(get<2>(locations[i]));
    }
    return result;
}

TreeNode* recoverFromPreorder(const string& input) {
    string traversal = trim(input);
    if (traversal.empty()) throw invalid_argument("Рядок обходу не може бути порожнім.");
    vector<TreeNode*> path;
    vector<TreeNode*> allocated;
    TreeNode* root = nullptr;
    size_t i = 0;
    try {
        while (i < traversal.size()) {
            int depth = 0;
            while (i < traversal.size() && traversal[i] == '-') {
                ++depth;
                ++i;
            }
            if (i == traversal.size() || !isdigit(static_cast<unsigned char>(traversal[i])))
                throw invalid_argument("Очікується значення вузла після тире.");
            long long value = 0;
            while (i < traversal.size() && isdigit(static_cast<unsigned char>(traversal[i]))) {
                value = value * 10 + (traversal[i] - '0');
                if (value > 1000000000LL) throw invalid_argument("Значення вузла перевищує 10^9.");
                ++i;
            }
            if (allocated.size() >= 1000)
                throw invalid_argument("Максимальна кількість вузлів — 1000.");
            if (!root && depth != 0)
                throw invalid_argument("Перший вузол повинен мати глибину 0.");
            if (root && depth == 0)
                throw invalid_argument("У дереві може бути лише один корінь.");
            if (depth > static_cast<int>(path.size()))
                throw invalid_argument("Пропущено рівень глибини.");
            while (path.size() > static_cast<size_t>(depth)) path.pop_back();
            if (root && path.empty())
                throw invalid_argument("Не знайдено батька вузла.");
            if (root && path.back()->left && path.back()->right)
                throw invalid_argument("У вузла вже є два дочірні елементи.");

            TreeNode* node = new TreeNode(static_cast<int>(value));
            allocated.push_back(node);
            if (!root) {
                root = node;
            }
            else if (!path.back()->left) {
                path.back()->left = node;
            }
            else {
                path.back()->right = node;
            }
            path.push_back(node);
        }
    }
    catch (...) {
        for (size_t j = 0; j < allocated.size(); ++j) delete allocated[j];
        throw;
    }
    return root;
}

string vectorToString(const vector<int>& numbers) {
    string result = "[";
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (i) result += ",";
        result += to_string(numbers[i]);
    }
    return result + "]";
}

string matrixToString(const vector<vector<int> >& columns) {
    string result = "[";
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i) result += ",";
        result += vectorToString(columns[i]);
    }
    return result + "]";
}

void showMenu() {
    cout << "\nАналітичне завдання 5 - Дерева\n"
        << "1. Перевірити, чи два дерева однакові\n"
        << "2. Перевірити симетричність дерева\n"
        << "3. Інвертувати бінарне дерево\n"
        << "4. Знайти k-й найменший елемент у BST\n"
        << "5. Серіалізувати та десеріалізувати дерево\n"
        << "6. Знайти максимальну суму шляху\n"
        << "7. Знайти мінімальну кількість камер\n"
        << "8. Виконати вертикальний обхід дерева\n"
        << "9. Відновити дерево з preorder-рядка\n"
        << "0. Завершити роботу\n";
}

void runTask(int number) {
    if (number == 1) {
        cout << "\nЗАВДАННЯ 1. Однакові дерева\n";
        TreeNode* p = readTree("Введіть дерево p: ", 100, -10000, 10000);
        TreeNode* q = readTree("Введіть дерево q: ", 100, -10000, 10000);
        cout << "Результат: " << (isSameTree(p, q) ? "true" : "false") << "\n";
        deleteTree(p);
        deleteTree(q);
    }
    else if (number == 2) {
        cout << "\nЗАВДАННЯ 2. Симетричне дерево\n";
        TreeNode* root = readTree("Введіть дерево: ", 1000, -10000, 10000);
        cout << "Результат: " << (isSymmetric(root) ? "true" : "false") << "\n";
        deleteTree(root);
    }
    else if (number == 3) {
        cout << "\nЗАВДАННЯ 3. Інвертування дерева\n";
        TreeNode* root = readTree("Введіть дерево: ", 100, -100, 100);
        invertTree(root);
        cout << "Інвертоване дерево: " << serialize(root) << "\n";
        deleteTree(root);
    }
    else if (number == 4) {
        cout << "\nЗАВДАННЯ 4. k-й найменший елемент у BST\n";
        TreeNode* root = readTree("Введіть BST: ", 10000, 0, 10000, false);
        if (!isValidBST(root)) {
            cout << "Помилка: це не бінарне дерево пошуку (BST).\n";
        }
        else {
            int k = readInt("Введіть k: ", 1, countNodes(root));
            cout << "Результат: " << kthSmallest(root, k) << "\n";
        }
        deleteTree(root);
    }
    else if (number == 5) {
        cout << "\nЗАВДАННЯ 5. Серіалізація та десеріалізація\n";
        TreeNode* root = readTree("Введіть дерево: ", 10000, -1000, 1000);
        string encoded = serialize(root);
        cout << "Серіалізований рядок: " << encoded << "\n";
        TreeNode* restored = deserialize(encoded);
        cout << "Десеріалізоване дерево: " << serialize(restored) << "\n";
        cout << "Структуру та значення збережено: "
            << (isSameTree(root, restored) ? "true" : "false") << "\n";
        deleteTree(root);
        deleteTree(restored);
    }
    else if (number == 6) {
        cout << "\nЗАВДАННЯ 6. Максимальна сума шляху\n";
        TreeNode* root = readTree("Введіть дерево: ", 30000, -1000, 1000, false);
        cout << "Максимальна сума шляху: " << maxPathSum(root) << "\n";
        deleteTree(root);
    }
    else if (number == 7) {
        cout << "\nЗАВДАННЯ 7. Камери бінарного дерева\n";
        TreeNode* root = readTree("Введіть дерево з нулів та null: ", 1000, 0, 0, false);
        cout << "Мінімальна кількість камер: " << minCameraCover(root) << "\n";
        deleteTree(root);
    }
    else if (number == 8) {
        cout << "\nЗАВДАННЯ 8. Вертикальний обхід дерева\n";
        TreeNode* root = readTree("Введіть дерево: ", 1000, 0, 1000, false);
        cout << "Вертикальний обхід: " << matrixToString(verticalTraversal(root)) << "\n";
        deleteTree(root);
    }
    else if (number == 9) {
        cout << "\nЗАВДАННЯ 9. Відновлення дерева з preorder-рядка\n";
        cout << "Приклад формату: 1-2--3--4-5--6--7\n";
        while (true) {
            string traversal = readLine("Введіть рядок обходу: ");
            try {
                TreeNode* root = recoverFromPreorder(traversal);
                cout << "Відновлене дерево: " << serialize(root) << "\n";
                deleteTree(root);
                break;
            }
            catch (const exception& ex) {
                cout << "Помилка: " << ex.what() << "\n";
            }
        }
    }
}

#ifndef TREE_TASKS_NO_MAIN
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    cout << "Введення дерева: [1,2,3,null,4]. null — відсутній вузол.\n"
        << "Позиції дітей задаються за рівнями, як у LeetCode.\n";
    while (true) {
        showMenu();
        int choice = readInt("Оберіть завдання (0..9): ", 0, 9);
        if (choice == 0) {
            cout << "Роботу завершено.\n";
            break;
        }
        runTask(choice);
    }
    return 0;
}
#endif

