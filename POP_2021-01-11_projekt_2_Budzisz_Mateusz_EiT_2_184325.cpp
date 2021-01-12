// POP 2021-01-11 project 2 Budzisz Mateusz EiT 2 184325
// Clion 2020.3 MSVC 14.28
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <queue>
#include <stack>
#include <stdexcept>
#include <string>
#include <variant>

using namespace std;
using Symbol = variant<float, char, string>;
bool ParseString(float &result, const char* inputString);

int main() {
    ifstream input("input.txt");
    ofstream output("output.txt");
    if (!input.is_open()) return 1;
    if (!output.is_open()) return 2;

    string line;
    // For every line in file
    while (getline(input, line)) {
        output << line;
        for (char &c : line) {
            if (c == ',') c = '.';
            if (c == '=') c = ' ';
        }

        float f;
        if (ParseString(f, line.c_str())) {
            output << f << endl;
        } else {
            output << "ERR0R" << endl;
        }
    }
    // Close files before changing names
    output.close();
    input.close();

    // Need to remove before rename
    if (remove("input.txt")) return 3;
    if (rename("output.txt", "input.txt")) return 4;
    return 0;
}

int Priority(Symbol symbol) {
    if (!(get_if<char>(&symbol))) throw std::invalid_argument("Non char symbol");

    switch (get<char>(symbol)) {
        case '(':
            return 0;
        case '+':
        case '-':
        case ')':
            return 1;
        case '*':
        case '/':
        case '%':
            return 2;
        case '^':
            return 3;
        default:
            throw invalid_argument(&"Not supported operator: " [ get<char>(symbol)]);
    }
}

bool ParseSymbols(queue<Symbol>& symbols, const char* s) {
    for (int i = 0, n = 0, tmp = 0; i < strlen(s) && n < strlen(s); i++, n += tmp) {
        char c; // Scan string char by char, then check if we can scan for string or number
        if (sscanf(s + n, "%c%n", &c, &tmp) == 1) {
            variant<float, char, string> symbol;
            if (isdigit(c)) { // Scan for number
                float f;
                if (sscanf(s + n, "%f%n", &f, &tmp) != 1) {
                    return false;
                }
                symbol = f;
            } else if (isalpha(c)) { // Scan for function name
                char cs[7];  // Longest possible value is "arcsin"
                if (sscanf(s + n, "%6s%n", cs, &tmp) != 1) {
                    return false;
                }

                for (int j = 0; j < strlen(cs); j++) {
                    if (isalpha(cs[j])) continue;

                    cs[j] = '\0';
                    tmp = j;
                    break;
                }
                symbol = cs;

            } else {
                if (c == ' ') continue; // Ignore empty space
                symbol = c;
            }
            symbols.push(symbol);
        } else {
            return false;
        }
    }
    return true;
}

bool PrintDebug(queue<Symbol> symbols) {
    while (!symbols.empty()) {
        Symbol symbol = symbols.front();
        symbols.pop();

        if (auto f = get_if<float>(&symbol)) {
            cout << "\x1B[33m" << *f << "\033[0m ";
        } else if (auto s = get_if<string>(&symbol)) {
            cout << "\x1B[32m" << *s << "\033[0m ";
        } else if (auto c = get_if<char>(&symbol)) {
            cout << "\x1B[31m" << *c << "\033[0m ";
        } else {
            return false;
        }
    }
    cout << endl;
    return true;
}

bool ConvertInput(queue<Symbol> symbols, queue<Symbol>& out) {
    stack<Symbol> stack;

    while (!symbols.empty()) {
        Symbol symbol = symbols.front();
        symbols.pop();

        if (get_if<float>(&symbol)) {
            out.push(symbol);
            continue;

        } else if (auto c = get_if<char>(&symbol)) {
            if (*c == '=') {
                out.push(symbol);
                break;

            } else if (*c == '(') {
                stack.push(symbol);
                continue;

            } else if (*c == ')') {
                while (true) {
                    if (stack.empty()) {
                        break;
                    }

                    auto top = get_if<char>(&stack.top());
                    if (!top || *top == '(') break;

                    out.push(stack.top());
                    stack.pop();
                }

                stack.pop();
                if (stack.empty()) {
                    continue;
                }

                if (auto top = get_if<string>(&stack.top())) {
                    out.push(stack.top());
                    stack.pop();
                }

                continue;
            }
        } else if (auto s = get_if<string>(&symbol)) {
            stack.push(symbol);
            continue;
        }

        while (true) {
            if (stack.empty()) break;
            auto top = get_if<char>(&stack.top());
            if (!top || *top == ')' || *top == '(') break;
            try {
                if (Priority(stack.top()) < Priority(symbol)) break;
            } catch(invalid_argument &e) {
                return false;
            }

            out.push(stack.top());
            stack.pop();
        }

        stack.push(symbol);
    }

    while (!stack.empty()) {
        out.push(stack.top());
        stack.pop();
    }

    return true;
}

bool Eval(float &result, queue<Symbol> out) {
    stack<float> stack;

    while (!out.empty()) {
        Symbol symbol = out.front();
        out.pop();

        if (auto f = get_if<float>(&symbol)) {
            stack.push(*f);
        } else if (auto c = get_if<char>(&symbol)) {
            auto b = stack.top();
            stack.pop();

            auto a = stack.top();
            stack.pop();

            float t;
            switch (*c) {
                case '-': t = a - b; break;
                case '+': t = a + b; break;
                case '/': t = a / b; break;
                case '*': t = a * b; break;
                case '^': t = pow(a, b); break;
                default:
                    return false;
            }

            stack.push(t);
        } else if (auto s = get_if<string>(&symbol)) {
            auto v = stack.top();
            stack.pop();

            if (*s == "sin") {
                stack.push(sin(v));
            } else if (*s == "cos") {
                stack.push(cos(v));
            } else if (*s == "tg") {
                stack.push(tan(v));
            } else if (*s == "ctg") {
                stack.push(cos(v)/sin(v));
            } else if (*s == "arcsin") {
                stack.push(asin(v));
            } else if (*s == "arccos") {
                stack.push(acos(v));
            } else if (*s == "arctg") {
                stack.push(atan(v));
            } else if (*s == "arcctg") {
                stack.push(1/atan(v));
            } else if (*s == "ln") {
                stack.push(log(v));
            } else if (*s == "sqrt") {
                stack.push(sqrt(v));
            } else if (*s == "log") {
                stack.push(log10(v));
            } else {
                return false;
            }
        }
    }

    result = stack.top();
    stack.pop();
    return true;
}

bool ParseString(float &result, const char* inputString) {
    queue<Symbol> symbols;
    if (!ParseSymbols(symbols, inputString)) return false;

    printf("\n\x1B[36mREAD\033[0m ");
    if (!PrintDebug(symbols)) return false;

    queue<Symbol> out;
    if (!ConvertInput(symbols, out)) return false;

    printf("\x1B[36mCONVERT\033[0m ");
    if (!PrintDebug(out)) return false;

    if (!Eval(result, out)) {
        printf("\x1B[36mEVAL \x1B[31mERR0R \033[0m\n");
        return false;
    }

    printf("\x1B[36mEVAL \x1B[33m%f \033[0m\n", result);
}
