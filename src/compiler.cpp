#include<iostream>
using namespace std;
#include<string>



void compile(string code,int &row,bool &notation)
{
	int i = 0;
	while (i<code.length())
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
		else if (isalpha(c)||c=='_')
		{
			string id;
			while ((i < code.length()) &&(isalnum(code[i])||code[i]=='_'))
			{
				id += code[i];
				i++;
			}
			if(id=="int"||id=="void"|| id == "if"|| id == "else"|| id == "while"|| id == "break"|| id == "continue"|| id == "return")
				cout << row << ":'" << id << "':\""<<id<<"\"" << endl;
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
				op = code.substr(i-1,2);
				cout<<row<< ":'" << op << "':\"" << op << "\"" << endl;
				i++;
			}
			else
			{
				op = c;
				cout << row << ":'" << op << "':\"" << op << "\"" << endl;
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
				cout << row << ":'" << op << "':\"" << op << "\"" << endl;
				i += 2;
			}
			row++;
		}
		else if(c=='/')
		{
			i++;
			if (code[i] == '/')
			{
				while (i < code.length() && code[i] != '\n')
					i++;
			}
			else if (code[i] == '*')
			{
				notation = 1;
				i++;
			}
			else
			{
				cout << row << ":'/'" << ":\"/\"" << endl;
				row++;

			}
		 
		}
		else
		{
			string op;
			op = c;
			cout << row << ":'" << op << "':\"" << op << "\"" << endl;
			i++;
			row++;
		}
		
	}
}

int main()
{	
	int row = 0;
	bool notation = 0;

	string code;
	while (getline(cin, code)) {

		compile(code,row,notation);
	}

	return 0;
}



