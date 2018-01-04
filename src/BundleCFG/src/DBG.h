#ifndef DBG_H
#define DBG_H

#include<string>
#include<sstream>
#include<stack>
#include<iostream>
using namespace std;

class DBG  {
public:
	DBG();
	void inc(string pfx="", string fill=" ");
	void dec();
	void flush(ostream &stream);
	stringstream buf;
	/* 
	 * Symbol that can be added to the end of a line to continue a
	 * debug statement.
	 */
	string cont;
	/*
	 * Symbol used as the start of a line
	 */
	string start;
private:
	void update();
	int _level;
	stack<string> _indents;
	stack<string> _pfxs;
	string _fill=" ";
};

#endif /* DBG_H */
