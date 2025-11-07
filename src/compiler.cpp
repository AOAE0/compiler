#include <iostream>
using namespace std;
#include<string>
#include<vector>
#include<unordered_set>

/*
  Token类型：
   - TK_ID: 标识符
   - TK_NUM: 数字（可以带前导'-'）
   - TK_KW: 关键字（int, void, if, else, while, break, continue, return）
   - TK_OP: 运算符（==, !=, <=, >=, ||, &&, +, -, *, /, %, =, <, >, !）
   - TK_LPAREN/TK_RPAREN/TK_LBRACE/TK_RBRACE: 括号
   - TK_COMMA/TK_SEMI: 分隔符
   - TK_EOF: 文件结束
   - TK_INVALID: 非法符号
*/

enum TokKind {
    TK_ID, TK_NUM, TK_KW, TK_OP,
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE,
    TK_COMMA, TK_SEMI, TK_EOF, TK_INVALID
};

struct Token {
    TokKind kind;   // Token类型
    string lexeme;  // Token的文本
    int line;       // 所在行号
};

// 关键字集合
static const unordered_set<string> keywords = {
    "int","void","if","else","while","break","continue","return"
};

// 双字符运算符集合
static const unordered_set<string> twoCharOps = {
    "==","!=","<=",">=","||","&&"
};

// 单字符运算符集合
static const unordered_set<char> singleOps = {
    '+','-','*','/','%','=','<','>','!'
};

vector<int> errorLines; // 记录出错的行号，按发现顺序

// 词法分析器：将整个输入（可能多行）转为Token
vector<Token> lexAll(istream& in) {
    vector<Token> toks;
    string line;
    int lineNo = 0;
    bool inBlockComment = false;
    // 将输入按行读取，以便保留行号
    vector<string> lines;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    size_t totalLines = lines.size();
    for (size_t idx = 0; idx < totalLines; ++idx) {
        lineNo = (int)idx + 1;
        string s = lines[idx];
        size_t i = 0, n = s.size();
        while (i < n) {
            if (inBlockComment) {
                // 搜索块注释结束标志 */
                bool closed = false;
                for (; i + 1 < n; ++i) {
                    if (s[i] == '*' && s[i + 1] == '/') {
                        inBlockComment = false;
                        i += 2;
                        closed = true;
                        break;
                    }
                }
                if (!closed) {
                    // 下一行仍在注释中
                    i = n;
                    break;
                }
                else {
                    continue;
                }
            }
            char c = s[i];
            if (isspace((unsigned char)c)) { i++; continue; }
            // 注释处理
            if (c == '/') {
                if (i + 1 < n && s[i + 1] == '/') {
                    // 单行注释 -> 跳过该行剩余部分
                    break;
                }
                else if (i + 1 < n && s[i + 1] == '*') {
                    // 块注释开始
                    inBlockComment = true;
                    i += 2;
                    continue;
                }
            }
            // 标识符或关键字
            if (isalpha((unsigned char)c) || c == '_') {
                string id;
                while (i < n && (isalnum((unsigned char)s[i]) || s[i] == '_')) {
                    id.push_back(s[i++]);
                }
                if (keywords.count(id)) {
                    toks.push_back({ TK_KW, id, lineNo });
                }
                else {
                    toks.push_back({ TK_ID, id, lineNo });
                }
                continue;
            }
            // 数字：允许前导'-'，且紧跟数字
            if (c == '-' && i + 1 < n && isdigit((unsigned char)s[i + 1])) {
                string num;
                num.push_back('-');
                i++;
                while (i < n && isdigit((unsigned char)s[i])) {
                    num.push_back(s[i++]);
                }
                toks.push_back({ TK_NUM, num, lineNo });
                continue;
            }
            if (isdigit((unsigned char)c)) {
                string num;
                while (i < n && isdigit((unsigned char)s[i])) {
                    num.push_back(s[i++]);
                }
                toks.push_back({ TK_NUM, num, lineNo });
                continue;
            }
            // 双字符运算符
            if (i + 1 < n) {
                string two = s.substr(i, 2);
                if (twoCharOps.count(two)) {
                    toks.push_back({ TK_OP, two, lineNo });
                    i += 2;
                    continue;
                }
            }
            // 单字符运算符
            if (singleOps.count(c)) {
                string op(1, c);
                toks.push_back({ TK_OP, op, lineNo });
                i++;
                continue;
            }
            // 括号和分隔符
            if (c == '(') { toks.push_back({ TK_LPAREN, "(", lineNo }); i++; continue; }
            if (c == ')') { toks.push_back({ TK_RPAREN, ")", lineNo }); i++; continue; }
            if (c == '{') { toks.push_back({ TK_LBRACE, "{", lineNo }); i++; continue; }
            if (c == '}') { toks.push_back({ TK_RBRACE, "}", lineNo }); i++; continue; }
            if (c == ',') { toks.push_back({ TK_COMMA, ",", lineNo }); i++; continue; }
            if (c == ';') { toks.push_back({ TK_SEMI, ";", lineNo }); i++; continue; }
            // 其他为非法符号
            string inval(1, c);
            toks.push_back({ TK_INVALID, inval, lineNo });
            errorLines.push_back(lineNo);
            i++;
        }
    }
    if (inBlockComment) {
        // 块注释未结束：报最后一行错误
        if (!lines.empty()) errorLines.push_back((int)lines.size());
    }
    // 添加文件结束Token
    toks.push_back({ TK_EOF, "EOF", (int)totalLines + 1 });
    return toks;
}

// 语法分析器：递归下降
class Parser {
    vector<Token>& toks;
    size_t pos;
    bool hasError;
    vector<int>& errs; // 引用全局errorLines，用于追加语法错误

    // 检查main函数存在性
    struct FuncInfo { string name; string rettype; vector<string> params; int line; };
    vector<FuncInfo> functions;

public:
    Parser(vector<Token>& tk, vector<int>& errRef) : toks(tk), pos(0), hasError(false), errs(errRef) {}

    Token& cur() { return toks[pos]; }
    Token& peek(size_t k) { return toks[min(pos + k, toks.size() - 1)]; }
    void advance() { if (pos < toks.size() - 1) pos++; } // 不越过EOF
    bool acceptOp(const string& s) {
        if (cur().kind == TK_OP && cur().lexeme == s) { advance(); return true; }
        return false;
    }
    bool acceptKind(TokKind k, const string& lex = "") {
        if (cur().kind == k) {
            if (lex.empty() || cur().lexeme == lex) {
                advance(); return true;
            }
        }
        return false;
    }
    void errorHere() {
        if (cur().kind != TK_EOF) {
            errs.push_back(cur().line);
        }
        else {
            if (!toks.empty()) errs.push_back(toks.back().line);
        }
        hasError = true;
    }

    // 顶层入口
    bool parse() {
        CompUnit();
        // 如果EOF之前仍有Token，标记错误
        if (cur().kind != TK_EOF) {
            while (cur().kind != TK_EOF) {
                errs.push_back(cur().line);
                advance();
            }
            hasError = true;
        }
        // 检查int main()是否存在
        bool hasMain = false;
        for (auto& f : functions) {
            if (f.name == "main" && f.rettype == "int" && f.params.empty()) {
                hasMain = true; break;
            }
        }
        if (!hasMain) {
            errs.push_back(1); // 没有main函数报第1行
            hasError = true;
        }
        return !hasError;
    }

private:
    // CompUnit → FuncDef+
    void CompUnit() {
        bool seenOne = false;
        while (cur().kind != TK_EOF) {
            if (cur().kind == TK_KW && (cur().lexeme == "int" || cur().lexeme == "void")) {
                seenOne = true;
                FuncDef();
            }
            else {
                // 顶层出现意外Token：记录错误并尝试同步
                errorHere();
                while (cur().kind != TK_EOF && !(cur().kind == TK_KW && (cur().lexeme == "int" || cur().lexeme == "void"))) advance();
            }
        }
    }

    // FuncDef → ("int" | "void") ID "(" (Param ("," Param)*)? ")" Block
    void FuncDef() {
        string rettype;
        if (cur().kind == TK_KW && (cur().lexeme == "int" || cur().lexeme == "void")) {
            rettype = cur().lexeme;
            advance();
        }
        else {
            errorHere();
            return;
        }
        int funcLine = cur().line;
        string name;
        if (cur().kind == TK_ID) {
            name = cur().lexeme;
            advance();
        }
        else {
            errorHere();
            while (cur().kind != TK_EOF && cur().lexeme != "(" && cur().lexeme != "{" && !(cur().kind == TK_KW && (cur().lexeme == "int" || cur().lexeme == "void"))) advance();
        }
        // 参数列表
        if (!acceptKind(TK_LPAREN)) {
            errorHere();
            while (cur().kind != TK_EOF && cur().lexeme != "(" && cur().lexeme != "{") advance();
            if (!acceptKind(TK_LPAREN)) return;
        }
        vector<string> params;
        if (cur().kind == TK_KW && cur().lexeme == "int") {
            while (true) {
                Param(params);
                if (cur().kind == TK_COMMA) {
                    advance();
                    continue;
                }
                else break;
            }
        }
        if (!acceptKind(TK_RPAREN)) {
            errorHere();
            while (cur().kind != TK_EOF && cur().kind != TK_LBRACE) advance();
        }
        // 记录函数信息
        functions.push_back({ name, rettype, params, funcLine });
        // 函数体
        Block();
    }

    // Param → "int" ID
    void Param(vector<string>& params) {
        if (cur().kind == TK_KW && cur().lexeme == "int") {
            advance();
        }
        else {
            errorHere();
            return;
        }
        if (cur().kind == TK_ID) {
            params.push_back(cur().lexeme);
            advance();
        }
        else {
            errorHere();
        }
    }

    // Block → "{" Stmt* "}"
    void Block() {
        if (!acceptKind(TK_LBRACE)) {
            errorHere();
            return;
        }
        while (cur().kind != TK_RBRACE && cur().kind != TK_EOF) {
            Stmt();
        }
        if (!acceptKind(TK_RBRACE)) {
            errorHere();
        }
    }

    // Stmt → 语句类型
    void Stmt() {
        if (cur().kind == TK_LBRACE) { Block(); return; }
        if (cur().kind == TK_SEMI) { advance(); return; } // 空语句
        if (cur().kind == TK_KW) {
            string kw = cur().lexeme;
            if (kw == "if") {
                advance();
                if (!acceptKind(TK_LPAREN)) { errorHere(); syncToStmtEnd(); return; }
                Expr();
                if (!acceptKind(TK_RPAREN)) { errorHere(); syncToStmtEnd(); return; }
                Stmt();
                if (cur().kind == TK_KW && cur().lexeme == "else") { advance(); Stmt(); }
                return;
            }
            else if (kw == "while") {
                advance();
                if (!acceptKind(TK_LPAREN)) { errorHere(); syncToStmtEnd(); return; }
                Expr();
                if (!acceptKind(TK_RPAREN)) { errorHere(); syncToStmtEnd(); return; }
                Stmt();
                return;
            }
            else if (kw == "break" || kw == "continue") {
                advance();
                if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); }
                return;
            }
            else if (kw == "return") {
                advance();
                if (cur().kind != TK_SEMI) { Expr(); }
                if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); }
                return;
            }
            else if (kw == "int") {
                advance();
                if (cur().kind == TK_ID) { advance(); }
                else { errorHere(); syncToStmtEnd(); return; }
                if (!acceptOp("=")) { errorHere(); syncToStmtEnd(); return; }
                Expr();
                if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); }
                return;
            }
        }
        if (cur().kind == TK_ID) {
            Token idTok = cur(); advance();
            if (cur().kind == TK_OP && cur().lexeme == "=") {
                advance(); Expr(); if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); } return;
            }
            else if (cur().kind == TK_LPAREN) {
                advance(); if (cur().kind != TK_RPAREN) { Expr(); while (cur().kind == TK_COMMA) { advance(); Expr(); } }
                if (!acceptKind(TK_RPAREN)) { errorHere(); syncToStmtEnd(); return; }
                if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); } return;
            }
            else {
                if (pos > 0) pos--; Expr(); if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); } return;
            }
        }
        Expr(); if (!acceptKind(TK_SEMI)) { errorHere(); syncToStmtEnd(); }
    }

    void syncToStmtEnd() {
        while (cur().kind != TK_SEMI && cur().kind != TK_RBRACE && cur().kind != TK_EOF) advance();
        if (cur().kind == TK_SEMI) advance();
    }

    // 表达式解析
    void Expr() { LOrExpr(); }
    void LOrExpr() { LAndExpr(); while (cur().kind == TK_OP && cur().lexeme == "||") { advance(); LAndExpr(); } }
    void LAndExpr() { RelExpr(); while (cur().kind == TK_OP && cur().lexeme == "&&") { advance(); RelExpr(); } }
    void RelExpr() { AddExpr(); while (cur().kind == TK_OP && (cur().lexeme == "<" || cur().lexeme == ">" || cur().lexeme == "<=" || cur().lexeme == ">=" || cur().lexeme == "==" || cur().lexeme == "!=")) { advance(); AddExpr(); } }
    void AddExpr() { MulExpr(); while (cur().kind == TK_OP && (cur().lexeme == "+" || cur().lexeme == "-")) { advance(); MulExpr(); } }
    void MulExpr() { UnaryExpr(); while (cur().kind == TK_OP && (cur().lexeme == "*" || cur().lexeme == "/" || cur().lexeme == "%")) { advance(); UnaryExpr(); } }
    void UnaryExpr() { if (cur().kind == TK_OP && (cur().lexeme == "+" || cur().lexeme == "-" || cur().lexeme == "!")) { advance(); UnaryExpr(); } else { PrimaryExpr(); } }
    void PrimaryExpr() {
        if (cur().kind == TK_LPAREN) { advance(); Expr(); if (!acceptKind(TK_RPAREN)) { errorHere(); } return; }
        if (cur().kind == TK_ID) { Token t = cur(); advance(); if (cur().kind == TK_LPAREN) { advance(); if (cur().kind != TK_RPAREN) { Expr(); while (cur().kind == TK_COMMA) { advance(); Expr(); } } if (!acceptKind(TK_RPAREN)) { errorHere(); } } return; }
        if (cur().kind == TK_NUM) { advance(); return; }
        errorHere();
    }
};


int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    errorLines.clear();

    // 词法分析整个标准输入
    vector<Token> tokens = lexAll(cin);

    // 语法分析
    Parser parser(tokens, errorLines);
    bool ok = parser.parse();

    // 处理重复行号，只保留首次出现
    vector<int> uniqueLines;
    unordered_set<int> seen;
    for (int ln : errorLines) {
        if (!seen.count(ln)) {
            uniqueLines.push_back(ln);
            seen.insert(ln);
        }
    }

    if (ok) {
        cout << "accept\n";
    }
    else {
        cout << "reject\n";
        for (int ln : uniqueLines) {
            cout << ln << "\n";
        }
    }
    return 0;
}
