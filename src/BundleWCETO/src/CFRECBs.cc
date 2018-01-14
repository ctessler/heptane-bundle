#include "CFRECBs.h"

std::ostream &
operator<<(std::ostream& stream, const ECBs& ecbs) {
	stream << ecbs.str();
	return stream;
}

string
ECBs::str(string pfx) const {
	stringstream ss;
	ECBs::const_iterator it, sit;
	ss << pfx << "(";
	uint32_t lcount = 0;
	for (it = begin(); it != end(); it++) {
		uint32_t cur = *it;
		if (it != begin()) {
			ss << ", ";
		}
		ss << cur;
		uint32_t count=0;
		while (it != end()) {
			count++;
			sit = it;
			sit++;
			if ((*sit) != cur) {
				break;
			}
			it = sit;
		} 
		if (count > 1) {
			ss << "⨯" << count;
		}
		if (it == end()) {
			break;
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

