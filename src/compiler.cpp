#include<iostream>
using namespace std;
#include<string>
#include<vector>
#include<map>
#include<queue>
#include<algorithm>

//0为常数，1为变量，2为函数，3为运算符，4为关键字，5为括号，6为分隔符，7为非法符号
queue<int> types;
queue<pair<string, int>> tokens;
vector<int> errorLines;

void compile(string code, int row, bool& notation)
{
	int i = 0;
	while (i < code.length())
	{
		char c = code[i];
		if (notation)
		{
			while (i < code.length() && (code[i] != '*' || code[i + 1] != '/'))
				i++;
			if (code[i] == '*' && code[i + 1] == '/')
			{
				i += 2;
				notation = 0;
			}
			continue;
		}

		if (isspace(c))
		{
			i++;
			continue;
		}
		else if (isalpha(c) || c == '_')
		{
			string id;
			while ((i < code.length()) && (isalnum(code[i]) || code[i] == '_'))
			{
				id += code[i];
				i++;
			}
			if (id == "int" || id == "void" || id == "if" || id == "else" || id == "while" || id == "break" || id == "continue" || id == "return")
			{
				types.push(4);
				tokens.push({ id, row });
			}
			else
			{
				types.push(1);
				tokens.push({ id, row });
			}
		}
		else if (isdigit(c))
		{
			string number;
			while (i < code.length() && isdigit(code[i]))
			{
				number += code[i];
				i++;
			}
			types.push(0);
			tokens.push({ number, row });
		}
		else if (c == '>' || c == '<' || c == '=' || c == '!')
		{
			string op;
			if (i + 1 < code.length() && code[i + 1] == '=')
			{
				op = code.substr(i, 2);
				i += 2;
			}
			else
			{
				op = c;
				i++;
			}
			types.push(3);
			tokens.push({ op, row });
		}
		else if (c == '|' || c == '&')
		{
			string op;
			if (i + 1 < code.length() && code[i + 1] == c)
			{
				op = string(2, c);
				i += 2;
			}
			else
			{
				op = string(1, c);
				i++;
			}
			types.push(3);
			tokens.push({ op, row });
		}
		else if (c == '/')
		{
			i++;
			if (code[i] == '/')//单行注释
			{
				while (i < code.length() && code[i] != '\n')
					i++;
			}
			else if (code[i] == '*')//多行注释
			{
				notation = 1;
				i++;
			}
			else//运算符
			{
				types.push(3);
				tokens.push({ "/", row });
			}
		}
		else if (c == '+' || c == '-' || c == '*' || c == '%')
		{
			string op;
			op = c;
			types.push(3);
			tokens.push({ op, row });
			i++;
		}
		else if (c == '(' || c == ')' || c == '{' || c == '}')
		{
			string s;
			s = c;
			types.push(5);
			tokens.push({ s, row });
			i++;
		}
		else if (c == ',' || c == ';')
		{
			string s;
			s = c;
			types.push(6);
			tokens.push({ s, row });
			i++;
		}
		else
		{
			string s;
			s = c;
			types.push(7);
			tokens.push({ s, row });
			i++;
		}
	}
}



// 语法分析函数
class Parser 
{
private:
    queue<pair<string, int>>& tokens;
    pair<string, int> currentToken;
    int currentType;
    bool hasError;

    void advance() 
    {
        if (!tokens.empty()) 
        {
            currentToken = tokens.front();
            tokens.pop();
			currentType = types.front();
			types.pop();
        }
        else 
        {
            currentToken.first = "EOF";
            currentType = -1;
        }
    }

    bool match(const string& expected) 
    {
        if (currentToken.first == expected) 
        {
            advance();
            return true;
        }
        return false;
    }

    void expect(const string& expected) 
    {
        if (currentToken.first == expected) 
        {
            advance();
        }
        else {
            if (currentType != -1) 
            { // 不是EOF才记录错误
                errorLines.push_back(currentToken.second);
            }
            hasError = true;
            if (currentType != -1) 
            {
                advance();
            }
        }
    }

    void recordError() 
    {
        if (currentType != -1) { // 不是EOF才记录错误
            errorLines.push_back(currentToken.second);
        }
        hasError = true;
        if (currentType != -1) 
        {
            advance();
        }
    }

    // 同步恢复函数 - 跳过token直到找到同步点
    void syncTo(const vector<string>& syncTokens) 
    {
        while (currentType != -1) 
        {
            for (int i = 0; i < syncTokens.size();i++) 
            {
                if (currentToken.first == syncTokens[i]) 
                {
                    return;
                }
            }
            advance();
        }
    }

public:
    Parser(queue<pair<string, int>>& t) : tokens(t), hasError(false) 
    {
        if (!tokens.empty()) 
        {
            advance();
        }
        else 
        {
            currentToken = { "EOF", -1 };
            currentType = -1;
        }
    }

    bool parse() 
    {
        CompUnit();
        return !hasError && tokens.empty();
    }

private:
    // 编译单元 CompUnit → FuncDef+
    void CompUnit() 
    {
        while (currentToken.first == "int" || currentToken.first == "void") 
        {
            FuncDef();
        }
        if (currentType != -1) {
            recordError();
        }
    }

    // 函数定义 FuncDef → ("int" | "void") ID "(" (Param ("." Param)*)? ")" Block
    void FuncDef() 
    {
        // 返回类型
        if (currentToken.first == "int" || currentToken.first == "void") 
        {
            advance();
        }
        else 
        {
            recordError();
            return;
        }

        // 函数名
        if (currentType == 1) 
        { // ID类型
            advance();
        }
        else 
        {
            recordError();
            return;
        }

        // 参数列表
        expect("(");

        if (currentToken.first == "int") 
        {
            Param();
            while (match(",")) 
            {
                Param();
            }
        }

        expect(")");
        Block();
    }

    // 形参 Param → "int" ID
    void Param() 
    {
        expect("int");
        if (currentType == 1) 
        { // ID类型
            advance();
        }
        else 
        {
            recordError();
        }
    }

    // 语句块 Block → "{" Stmt* "}"
    void Block() 
    {
        expect("{");
        while (currentToken.first != "}" && currentType != -1) 
        {
            Stmt();
            if (currentToken.first == "EOF") break;
        }
        expect("}");
    }

    // 语句 Stmt
    void Stmt() 
    {
        if (currentToken.first == "{") 
        {
            Block();
        }
        else if (currentToken.first == ";") 
        {
            advance(); // 空语句
        }
        else if (currentToken.first == "if") 
        {
            advance();
            expect("(");
            Expr();
            expect(")");
            Stmt();
            if (currentToken.first == "else") 
            {
                advance();
                Stmt();
            }
        }
        else if (currentToken.first == "while") 
        {
            advance();
            expect("(");
            Expr();
            expect(")");
            Stmt();
        }
        else if (currentToken.first == "break" || currentToken.first == "continue") 
        {
            advance();
            expect(";");
        }
        else if (currentToken.first == "return") 
        {
            advance();
            if (currentToken.first != ";") 
            {
                Expr();
            }
            expect(";");
        }
        else if (currentToken.first == "int") 
        {
            advance(); // int
            if (currentToken.first != "(" && currentToken.first != ")" &&currentToken.first != "{" && currentToken.first != "}" &&
                currentToken.first != ";" && currentToken.first != "EOF") 
            {
                advance();
            }
            else 
            {
                recordError();
                syncTo({ ";", "}", "EOF" });
                return;
            }
            expect("=");
            Expr();
            expect(";");
        }
        else if (currentToken.first != "(" && currentToken.first != ")" &&currentToken.first != "{" && currentToken.first != "}" &&
            currentToken.first != ";" && currentToken.first != "EOF") 
        {
            // 可能是赋值或函数调用或表达式
            string id = currentToken.first;
            advance();

            if (currentToken.first == "=") 
            {
                advance(); // =
                Expr();
                expect(";");
            }
            else if (currentToken.first == "(") 
            {
                // 函数调用
                advance(); // (
                if (currentToken.first != ")") 
                {
                    Expr();
                    while (match(",")) 
                    {
                        Expr();
                    }
                }
                expect(")");
                expect(";");
            }
            else 
            {
                // 表达式语句
                // 回退一个token，因为Expr需要从当前token开始
                queue<pair<string, int>> temp;
                temp.push({ id, currentToken.second });
                while (!tokens.empty()) 
                {
                    temp.push(tokens.front());
                    tokens.pop();
                }
                tokens = temp;
                advance();

                Expr();
                expect(";");
            }
        }
        else 
        {
            // 表达式语句
            Expr();
            expect(";");
        }
    }

    // 表达式 Expr → LOrExpr
    void Expr() 
    {
        LOrExpr();
    }

    // 逻辑或表达式 LOrExpr → LAndExpr | LOrExpr "||" LAndExpr
    void LOrExpr() {
        LAndExpr();
        while (currentToken.first == "||") 
        {
            advance();
            LAndExpr();
        }
    }

    // 逻辑与表达式 LAndExpr → RelExpr | LAndExpr "&&" RelExpr
    void LAndExpr() 
    {
        RelExpr();
        while (currentToken.first == "&&") 
        {
            advance();
            RelExpr();
        }
    }

    // 关系表达式 RelExpr → AddExpr | RelExpr ("<" | ">" | "<=" | ">=" | "==" | "!=") AddExpr
    void RelExpr() 
    {
        AddExpr();
        while (currentToken.first == "<" || currentToken.first == ">" || currentToken.first == "<=" ||currentToken.first == ">=" 
            || currentToken.first == "==" || currentToken.first == "!=") 
        {
            advance();
            AddExpr();
        }
    }

    // 加减表达式 AddExpr → MulExpr | AddExpr ("+" | "-") MulExpr
    void AddExpr() 
    {
        MulExpr();
        while (currentToken.first == "+" || currentToken.first == "-") 
        {
            advance();
            MulExpr();
        }
    }

    // 乘除模表达式 MulExpr → UnaryExpr | MulExpr ("*" | "/" | "%") UnaryExpr
    void MulExpr() 
    {
        UnaryExpr();
        while (currentToken.first == "*" || currentToken.first == "/" || currentToken.first == "%") 
        {
            advance();
            UnaryExpr();
        }
    }

    // 一元表达式 UnaryExpr → PrimaryExpr | ("+" | "-" | "!") UnaryExpr
    void UnaryExpr() 
    {
        if (currentToken.first == "+" || currentToken.first == "-" || currentToken.first == "!") 
        {
            advance();
            UnaryExpr();
        }
        else 
        {
            PrimaryExpr();
        }
    }

    // 基本表达式 PrimaryExpr → ID | NUMBER | "(" Expr ")" | ID "(" (Expr ("," Expr)*)? ")"
    void PrimaryExpr() 
    {
        if (currentType == 1) 
        { // ID类型
            string id = currentToken.first;
            advance();

            if (currentToken.first == "(") 
            {
                // 函数调用
                advance(); // (
                if (currentToken.first != ")") 
                {
                    Expr();
                    while (match(",")) 
                    {
                        Expr();
                    }
                }
                expect(")");
            }
            // 否则就是变量引用，不需要额外处理
        }
        else if (currentType == 0) 
        { // NUMBER类型
            advance();
        }
        else if (currentToken.first == "(") 
        {
            advance(); // (
            Expr();
            expect(")");
        }
        else 
        {
            recordError();
        }
    }

    // 基本表达式 PrimaryExpr → ID | NUMBER | "(" Expr ")" | ID "(" (Expr ("," Expr)*)? ")"
    void PrimaryExpr() 
    {
        if (currentToken.first == "(") 
        {
            advance(); // (
            Expr();
            expect(")");
        }
        else if (currentToken.first != "(" && currentToken.first != ")" &&currentToken.first != "{" && currentToken.first != "}" &&
            currentToken.first != ";" && currentToken.first != "," &&currentToken.first != "EOF") 
        {
            string id = currentToken.first;
            advance();

            if (currentToken.first == "(") 
            {
                // 函数调用
                advance(); // (
                if (currentToken.first != ")") 
                {
                    Expr();
                    while (match(",")) 
                    {
                        Expr();
                    }
                }
                expect(")");
            }
        }
        else 
        {
            recordError();
        }
    }
};

void syntaxAnalysis() 
{
    Parser parser(tokens);
    bool success = parser.parse();

    if (success) 
    {
        cout << "accept" << endl;
    }
    else 
    {
        cout << "reject" << endl;
        sort(errorLines.begin(), errorLines.end());
        errorLines.erase(unique(errorLines.begin(), errorLines.end()), errorLines.end());
        for (int i=0;i< errorLines.size();i++) 
        {
            cout << errorLines[i] << endl;
        }
    }
}

int main()
{
	int row = 1;
	bool notation = 0;

	string code;
	while (getline(cin, code)) 
    {
		compile(code, row, notation);
		row++;
	}
    syntaxAnalysis();

	return 0;
}
