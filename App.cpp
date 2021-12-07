/*
 *  UCF COP3330 Fall 2021 Assignment 6 Solution
 *  Copyright 2021 Adan Rodriguez
 */
/*
Exercise 1:
Allow underscores in the calculator’s variable names.
Exercise 2:
Provide an assignment operator, =, so that you can change the value of a variable after you introduce it using let. 
Exercise 3:
Provide an assignment operator, =, so that you can change the value of a variable after you introduce it using let. 
*/
#include "std_lib_facilities.h"

struct Token {
	char kind;
	double value;
	string name;
	Token(char ch) :kind(ch), value(0) { }
	Token(char ch, double val) :kind(ch), value(val) { } 
	Token(char ch, string n) :kind(ch), name(n) { } 
};

class Token_stream {
	bool full;
	Token buffer; 
	string file;
public:
	Token_stream(istream& file) :full(0), buffer(0) { }

	Token get();
	void unget(Token t) { buffer=t; full=true; } 

	void ignore(char); 
};

const char assign = '='; 
const char constant = 'c';
const char prompt = '>'; 
const char quit = 'q'; 
const char help = 'h'; 
const string result = "= "; 

const char let = 'L'; 
const char print = ';'; 
const char number = '8'; 
const char name = 'a';
const char sqrts = 's'; 
const char pows = 'p'; 


Token Token_stream::get() 
{
	if (full) { full=false; return buffer; } 
	char ch;
	cin >> ch; 
	if (isspace(ch)) return Token(prompt); 
	switch (ch) {
	case '(':
	case ')':
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
	case ';':
	case '!':
		return Token(ch); 
	case assign:
		return Token(assign); 
	case '.':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{	
		cin.unget(); 
		double val;
		cin >> val; 
		return Token(number,val);
	}
	default:
		if (isalpha(ch)) {
			string s;
			char ch2 = ch; 
			s += ch; 
			while(cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) { 
				s+=ch; 
			}
			cin.unget(); 
			if (s == "let") return Token(let); 
			if (s == "sqrt") return Token(sqrts); 
			if (s == "const") return Token(constant); 
			if (s == "pow") return Token(pows); 
			if (ch2 == quit || s == "quit") return Token(quit);
			if (ch2 == help || s == "help" || s =="Help") return Token(help); 
			return Token(name,s); 
		}
		error("Bad token!");
	}
	return 0;
}

void Token_stream::ignore(char c) 
{
	if (full && c==buffer.kind) { 
		full = false;
		return;
	}
	full = false; 
	char ch;
	while (cin>>ch) 
		if (ch==c) return;
}

struct Variable { 
	string name; 
	double value; 
	char constant; 
	Variable(string n, double v) :name(n), value(v) { }
	Variable(string n, double v, char c) :name(n), value(v), constant(c) { } 
};

class Symbol_table {
	vector<Variable> names;
	public:
		double get_value(string s);
		void set_value(string s, double d);
		bool is_declared(string s);
		double define_name(char type);
};

double Symbol_table::get_value(string s) 
{
	for (Variable i: names) {
		if (i.name == s) return i.value;
	}
	error("Undefined name.",s);
	return 0;
}

void Symbol_table::set_value(string s, double d) 
{
	for (int i = 0; i<=names.size(); ++i)
		if (names[i].constant == constant) error("Constant assignment."); 
		else if (names[i].name == s) {	
			names[i].value = d;
			return;
		}
	error("Undefined name.",s);
}

bool Symbol_table::is_declared(string s) 
{
	for (int i = 0; i<names.size(); ++i)
		if (names[i].name == s) return true;
	return false;
}

Token_stream ts(cin);
Symbol_table names; 

double expression(Token_stream&); 

double primary(Token_stream&); 

double squareroot(){ 
	double d = expression(ts);
	if (d < 0) error("Square root of negative number is not allowed!"); 
	return sqrt(d);
}

double pow(){ 
	Token t = ts.get();
	if (t.kind == '(') {
		double lval = expression(ts);
		int rval = 0;
		t = ts.get();
		if(t.kind == ',') rval = narrow_cast<int>(primary(ts));
		else error("Second argument is not provided!");
		double result = 1;
		for(double i = 0; i < rval; i++) {
			result*=lval;
		}
		t = ts.get();
		if (t.kind != ')') error("')' missing!"); 
			return result;
	}
	else error("'(' missing!"); 
	return 0;
}

double primary(Token_stream& ts) 
{
	Token t = ts.get(); 
	switch (t.kind) {
	case '(': 
	{	double d = expression(ts); 
		t = ts.get(); 
		if (t.kind != ')') error("')' missing!"); 
		return d;
	}
	case sqrts: 
	{
		return squareroot();
	}
	case pows:
	{
		return pow();
	}
	case '-': 
		return - primary(ts); 
	case number: 
		if(narrow_cast<int>(t.value)) return t.value; 
	case name: 
	{	
		string s = t.name; 
		t = ts.get();
		if (t.kind == assign) names.set_value(s,expression(ts)); 
		else ts.unget(t);
		return names.get_value(s); 
	}
	case help:
		return primary(ts);
	default:
		error("Unlisted operation."); 
	}
	return 0;
}

double term(Token_stream& ts)
{	
	double left = primary(ts); 
	while(true) {
		Token t = ts.get(); 
		switch(t.kind) {
		case '*': 
			left *= primary(ts); 
			break;
		case '/': 
		{	double d = primary(ts); 
			if (d == 0) error("Division by zero is not allowed!"); 
			left /= d;
			break;
		}
        case '!': 
            {   
                int x = left;
                for (int i = 1; i < left; i++) { 
                    x*=i;
                }
                if (x == 0) left = 1;
                else left = x;
                break;
            }        
        case '%':
            {
                double d = primary(ts);
                if(d == 0) error("Division by zero is not allowed!");
                left = fmod(left,d);
                break;
            }
		default:
			ts.unget(t); 
			if (left == -0) return 0; 
			return left; 
		}

	}
}

double expression(Token_stream& ts) 
{
	double left = term(ts); 
	while(true) {
		Token t = ts.get();
		switch(t.kind) {
		case '+':
			left += term(ts); 
			break;
		case '-':
			left -= term(ts);
			break;
		default:
			ts.unget(t); 
			return left; 
		}
	}
}

double Symbol_table::define_name(char type) 
{
	Token t = ts.get(); 
	if (t.kind != 'a') error ("Name missing in declaration."); 
	string name = t.name; 
	if (is_declared(name)) error(name, " declared twice!"); 
	Token t2 = ts.get(); 
	if (t2.kind != '=') error("= missing in definition of " ,name); 
	double d = expression(ts);
	switch(type) {
	case let:
		names.push_back(Variable(name,d)); 
		return d; 
	case constant:
		names.push_back(Variable(name,d,constant));
		return d; 
	}
	return 0;
}


double statement() 
{
	Token t = ts.get();
	switch(t.kind) {
	case let: 
		return  names.define_name(let);
	case constant:
		return names.define_name(constant);
	default: 
		ts.unget(t);
		return expression(ts);
	}
}

void clean_up_mess() 
{
	ts.ignore(print);
}

void calculate() 
{
	
	while(true) try {
		cout << prompt; 
		Token t = ts.get(); 
		if (t.kind == help) { 
		cout << "Your Calculator Guide: << List of commands:\n"
			"Operators are '+','-','/','*'. Floating-point numbers.\n"
			"'User input expression' + ';' - Prints the result, needs to be used everytime!\n"
			"'let' + 'name of variable' + '='' + 'value' - variable definition.\n"
			"'const' + 'name of constant' + '='' + 'value' - constant definition.\n"
			"'name of variable' + '=' + 'new value' - new variable's value.\n"
			"'pow(x,y)' - power function, y of type int only!\n"
			"'sqrt(x)' - square root function.\n"
			"'h' or 'help' - user guide request.\n"
			"'quit' - exit.\n";
			continue; 
		}
		while (t.kind == print) t=ts.get(); 
		if (t.kind == quit) return; 
		ts.unget(t); 
		cout << result << statement() << endl; 
	}
	catch(runtime_error& e) {
		cerr << e.what() << endl; 
		clean_up_mess(); 
	}
}

int main()
	try {	
		calculate();
		return 0;
	}
	catch (exception& e) { 
		cerr << "exception: " << e.what() << endl;
		char c;
		while (cin >>c&& c!=';');
		return 1;
	}
	catch (...) { 
		cerr << "exception\n";
		char c;
		while (cin>>c && c!=';');
		return 2;
	}