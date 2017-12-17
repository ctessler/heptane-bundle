#include "CFRECBs.h"

std::ostream &
operator<<(std::ostream& stream, const ECBs& ecbs) {
	stream << ecbs.str();
	return stream;
}

string
ECBs::str(string pfx) const {
	stringstream ss;
	ECBs::const_iterator it;
	ss << pfx << "(";
	for (it = begin(); it != end(); ++it) {
		ss << (*it);
		if (it != (--end())) {
			ss << ", ";
		}
	}
	ss << ")";
	return ss.str();
}

ECBs::ECBs() {
}

ECBs::ECBs(ECBs &src) {
	insert(begin(), src.begin(), src.end());
}

ECBs::ECBs(ECBs *src) {
	insert(begin(), src->begin(), src->end());
}

