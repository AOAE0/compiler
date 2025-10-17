#include<iostream>
using namespace std;

void compile(string code)
{
	int i = 0;
	int row = 0;
	while (i<code.length())
	{
		char c = code[i];
		 
		if (isspace(c))
		{
			i++;
			continue;
		}
		else if (isalpha(c))
		{
			string id;
			while ((i < code.length()) &&isalnum(code[i]))
			{
				id += code[i];
				i++;
			}
			if(id=="int"||id=="void"|| id == "if"|| id == "else"|| id == "while"|| id == "break"|| id == "continue"|| id == "return")
				cout << row << ":'" << id << "':\""<<id<<" \"" << endl;
			else
				cout << row << ":Ident:\"" << id << "\"" << endl;
			row++;
		}
		else if (isdigit(c))
		{
			string number;
			while (i < code.length() && isdigit(code[i]))
			{
				number += code[i];
				i++;
			}
			cout << row << ":IntConst:\"" << number << "\"" << endl;
			row++;
		}
		else if(c=='>'||c=='<'|| c == '='|| c == '!')
		{
			string op;
			i++;
			if (code[i] == '=')
			{
				op = c+code[i];
				cout<<row<< ":'" << op << "':\"" << op << " \"" << endl;
				i++;
			}
			else
			{
				op = c;
				cout << row << ":'" << op << "':\"" << op << " \"" << endl;
			}
			row++;
		}
		else if (c == '|' || c == '&')
		{
			string op;
			op = c;
			if (code[i + 1] == c)
			{
				op += c;
				cout << row << ":'" << op << "':\"" << op << " \"" << endl;
				i += 2;
			}
			row++;
		}
		else if(c=='/')
		{
			i++;
			if (code[i] == '/')
			{
				cout << row << ":'//':\"// \"" << endl;
				row++;
				while (i < code.length() && code[i] != '\n')
					i++;
			}
			else if (code[i] == '*')
			{
				cout << row << ":'/*':\"/* \"" << endl;
				row++;
				i++;
				while (i < code.length() && (code[i] != '*' || code[i + 1] != '/'))
					i++;
				cout << row << ":'*/':\"*/ \"" << endl;
				i += 2;
				row++;
			}
			else
			{
				cout << row << ":'/'" << ":\"/ \"" << endl;
				row++;
			}
		 
		}
		else
		{
			string op;
			op = c;
			cout << row << ":'" << op << "':\"" << op << " \"" << endl;
			i++;
			row++;
		}
		
	}
}

int main()
{
	//string code = "int main() {\n"
	//	"   // comment\n"
	//	"   int a = 10;\n"
	//	"   if (a >= 10) {\n"
	//	"       a = a + 1;\n"
	//	"   }\n"
	//	"   return a;\n"
	//	"}\n";

	string code;

	cin >> code;
	cout<<code<<endl;

	//compile(code);

	return 0;

}
