#ifndef DBG_H
#define DBG_H

class DBG {
public:
	DBG() {}
	void pfx(string px) {
		_pfx = px;
		reset();
	}
	void inc(string fill=" ") {
		_fill = fill;
		_level++;
		reset();
	}
	void dec() {
		_level--;
		reset();
	}
	void flush(ostream &stream) {
		stream << buf.str();
		buf.str("");
	}
	stringstream buf;
	string cont;
private:
	void reset() {
		_indent = "";
		for (uint32_t i=0; i<_level; i++) {
			_indent += _fill;
		}
		_indent += _pfx;

		cont = "\n" + _indent;
	}
	uint32_t _level=0;
	string _pfx, _indent, _fill=" ";
};

#endif /* DBG_H */
