#include "DBG.h"

DBG::DBG() {
	_level=-2;
	inc("");
}

void
DBG::inc(string pfx, string fill) {
	_pfxs.push(pfx);
	_fill = fill;
	_level++;

	string indent="";
	for (int i=0; i < _level; i++) {
		indent += fill;
	}
	indent += pfx;
	_indents.push(indent);
	update();
}
void
DBG::dec() {
	_pfxs.pop();
	_indents.pop();
	_level--;
	update();
}
void
DBG::flush(ostream &stream) {
	stream << buf.str();
	buf.str("");
}

void
DBG::update() {
	start = _indents.top();
	cont = "\n" + start + "+- ";
}
