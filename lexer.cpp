#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

struct Token { string type, text; int line, col; };

vector<string> lines;      // the source code, line by line
vector<Token> tokens;      // all tokens found
int errors = 0, warnings = 0;

const string OPS = "+-*/%=<>!&|^~";                  // characters an operator is made of
const string TWO = " ++ -- -> << >> <= >= == != && || += -= *= /= %= &= |= ^= ";  // 2-letter operators
string badOps[] = {"===", "!==", "=<", "=>"};         // wrong operators
string keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else",
    "enum", "extern", "float", "for", "goto", "if", "int", "long", "register", "return",
    "short", "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned",
    "void", "volatile", "while", "bool", "class", "true", "false"
};
const int KEYWORDS = sizeof(keywords) / sizeof(keywords[0]);

// ---------- small helper functions ----------
bool isWord(char c) { return isalnum((unsigned char)c) || c == '_'; }    // letter, digit or _

bool isKeyword(string w) {
    for (int k = 0; k < KEYWORDS; k++) if (keywords[k] == w) return true;
    return false;
}

// Same letters with two neighbours swapped?  (whlie / while)
bool isSwap(string a, string b) {
    if (a.size() != b.size() || a == b) return false;
    size_t i = 0;
    while (a[i] == b[i]) i++;                                            // first difference
    return i + 1 < a.size() && a[i] == b[i + 1] && a[i + 1] == b[i] && a.substr(i + 2) == b.substr(i + 2);
}

// Is w a valid number?  (123  3.14  1e-3  0xFF  10u  ...)
bool isNumber(string w) {
    size_t i = 0, n = w.size();
    if (n > 2 && w[0] == '0' && (w[1] == 'x' || w[1] == 'X'))            // hexadecimal
        return w.find_first_not_of("0123456789abcdefABCDEFuUlL", 2) == string::npos;
    while (i < n && isdigit(w[i])) i++;                                  // digits
    if (i < n && w[i] == '.') { i++; while (i < n && isdigit(w[i])) i++; }   // . digits
    if (i < n && (w[i] == 'e' || w[i] == 'E')) {                         // exponent
        i++;
        if (i < n && (w[i] == '+' || w[i] == '-')) i++;
        if (i == n || !isdigit(w[i])) return false;
        while (i < n && isdigit(w[i])) i++;
    }
    while (i < n && string("fFlLuU").find(w[i]) != string::npos) i++;    // suffix
    if (n > 1 && w[0] == '0' && w.find_first_not_of("0123456789") == string::npos
        && w.find_first_of("89") != string::npos) return false;          // 09 is not octal
    return i == n;
}

// Print one error or warning with the source line and a ^~~~ marker
void report(bool isError, int ln, int col, int len, string msg) {
    if (isError) errors++; else warnings++;
    cout << (isError ? "error" : "warning") << " at line " << ln << ", col " << col << ": " << msg << "\n";
    cout << "    " << lines[ln - 1] << "\n";
    cout << "    " << string(col - 1, ' ') << "^" << string(len > 1 ? len - 1 : 0, '~') << "\n\n";
}

// ---------- getting the code: from a file or from the keyboard ----------
bool readFile(string path) {
    ifstream file(path.c_str());
    if (!file) return false;
    string s;
    lines.clear();
    while (getline(file, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        lines.push_back(s);
    }
    return true;
}

void typeCode() {
    string s;
    lines.clear();
    cout << "Type your code. Type END on a new line when you are finished:\n";
    while (getline(cin, s) && s != "END") lines.push_back(s);
}

// ---------- the scanner: scans the code stored in 'lines' ----------
void analyze() {
    string src;
    tokens.clear(); errors = warnings = 0;
    for (size_t k = 0; k < lines.size(); k++) src += lines[k] + "\n";

    int n = src.size(), i = 0, ln = 1, lineStart = 0;
    cout << "\n";
    while (i < n) {
        char c = src[i];
        if (c == '\n') { ln++; lineStart = ++i; continue; }
        if (c == ' ' || c == '\t' || c == '\r') { i++; continue; }

        int b = i, tl = ln, col = i - lineStart + 1;     // where this token begins
        string type = "";                                // stays empty if it is not a valid token

        if (c == '#') {                                  // #include, #define ...
            while (i < n && src[i] != '\n') i++;
            type = "PREPROCESSOR";
        }
        else if (c == '/' && src[i + 1] == '/') {        // // comment
            while (i < n && src[i] != '\n') i++;
            type = "COMMENT";
        }
        else if (c == '/' && src[i + 1] == '*') {        // /* comment */
            i += 2;
            while (i < n && !(src[i] == '*' && src[i + 1] == '/')) {
                if (src[i] == '\n') { ln++; lineStart = i + 1; }
                i++;
            }
            if (i >= n) report(true, tl, col, 2, "comment is never closed (missing */)");
            else { i += 2; type = "COMMENT"; }
        }
        else if (isalpha((unsigned char)c) || c == '_') {   // keyword or identifier
            while (isWord(src[i])) i++;
            string w = src.substr(b, i - b);
            type = isKeyword(w) ? "KEYWORD" : "IDENTIFIER";
            if (type == "IDENTIFIER" && w.size() > 2)
                for (int k = 0; k < KEYWORDS; k++)
                    if (isSwap(w, keywords[k]))
                        report(false, tl, col, w.size(), "'" + w + "' looks like a misspelled keyword, did you mean '" + keywords[k] + "'?");
        }
        else if (isdigit((unsigned char)c) || (c == '.' && isdigit((unsigned char)src[i + 1]))) {   // number
            while (isWord(src[i]) || src[i] == '.' ||
                   ((src[i] == '+' || src[i] == '-') && (src[i - 1] == 'e' || src[i - 1] == 'E')
                    && src.compare(b, 2, "0x") != 0 && isdigit((unsigned char)src[i + 1]))) i++;
            string w = src.substr(b, i - b);
            if (isNumber(w)) type = "NUMBER";
            else if (w.find('.') != w.rfind('.')) report(true, tl, col, w.size(), "'" + w + "' has more than one decimal point");
            else {
                size_t p = w.find_first_not_of("0123456789");     // first non-digit
                bool isName = p != string::npos && isalpha((unsigned char)w[p]) && string("eExX").find(w[p]) == string::npos;
                report(true, tl, col, w.size(), "'" + w + (isName ? "' is an invalid identifier (a name cannot start with a digit)" : "' is not a valid number"));
            }
        }
        else if (c == '"') {                             // "string"
            i++;
            while (i < n && src[i] != '"' && src[i] != '\n') {
                if (src[i] == '\\' && src[i + 1] != '\n') i++;   // skip an escape like \n
                i++;
            }
            if (src[i] == '"') { i++; type = "STRING"; }
            else report(true, tl, col, i - b, "string is never closed (missing \")");
        }
        else if (c == '\'') {                            // 'c'
            i++;
            while (i < n && src[i] != '\'' && src[i] != '\n') {
                if (src[i] == '\\' && src[i + 1] != '\n') i++;
                i++;
            }
            if (src[i] != '\'') report(true, tl, col, i - b, "character is never closed (missing ')");
            else {
                i++;
                string inner = src.substr(b + 1, i - b - 2);
                if (inner.empty() || (inner.size() > 1 && inner[0] != '\\'))
                    report(true, tl, col, i - b, "a character literal needs exactly one character");
                else type = "CHAR";
            }
        }
        else if (OPS.find(c) != string::npos) {          // operators
            while (OPS.find(src[i]) != string::npos &&
                   !(i > b && src[i] == '/' && (src[i + 1] == '/' || src[i + 1] == '*'))) i++;
            string run = src.substr(b, i - b);
            bool bad = false;
            for (int k = 0; k < 4; k++)
                if (run.find(badOps[k]) != string::npos) {
                    report(true, tl, col, run.size(), "'" + badOps[k] + "' is not a valid operator");
                    bad = true;
                }
            for (size_t k = 0; k < run.size() && !bad; ) {   // split the run into operators
                size_t len = (k + 1 < run.size() && TWO.find(" " + run.substr(k, 2) + " ") != string::npos) ? 2 : 1;
                tokens.push_back({"OPERATOR", run.substr(k, len), tl, col + (int)k});
                k += len;
            }
        }
        else if (string("(){}[];,.:?").find(c) != string::npos) { i++; type = "PUNCTUATOR"; }
        else {                                           // anything else is illegal
            i++;
            while (isWord(src[i])) i++;
            report(true, tl, col, i - b, "illegal character in '" + src.substr(b, i - b) + "'");
        }

        if (type != "") tokens.push_back({type, src.substr(b, i - b), tl, col});
    }

    cout << "Lines: " << lines.size() << "   Tokens: " << tokens.size()
         << "   Errors: " << errors << "   Warnings: " << warnings << "\n";
    cout << (errors ? "RESULT: FAILED - lexical errors found" : "RESULT: PASSED - no lexical errors") << "\n\n";
}

void showTokens() {
    printf("\nLINE COL  TYPE          TEXT\n");
    for (size_t k = 0; k < tokens.size(); k++) {
        string text = tokens[k].text.substr(0, tokens[k].text.find('\n'));   // first line only
        printf("%-5d%-5d%-14s%s\n", tokens[k].line, tokens[k].col, tokens[k].type.c_str(), text.c_str());
    }
    printf("\n");
}

int main() {
    string choice, path;
    while (true) {
        cout << "===== LEXICAL ERROR DETECTOR AND REPORTER =====\n1. Analyze a file\n2. Type code in the console\n3. Show token table of the last analysis\n0. Exit\nEnter choice: ";
        if (!getline(cin, choice) || choice == "0") break;
        if (choice == "1") {
            cout << "File name (for example errors.c): ";
            getline(cin, path);
            if (path.size() > 1 && path[0] == '"') path = path.substr(1, path.size() - 2);   // remove "quotes"
            if (readFile(path)) analyze();
            else cout << "\nCannot open '" << path << "'. Put the file in the same folder as the program.\n\n";
        }
        else if (choice == "2") { typeCode(); analyze(); }
        else if (choice == "3") showTokens();
    }
    return 0;
}
