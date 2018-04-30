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


uint32_t
ECBs::dupeCount() {
	uint32_t rval = 0;
	ECBs u(*this);

	sort();
	u.unique();
	u.sort();
	ECBs::iterator eit = begin();
	for (ECBs::iterator uit = u.begin(); uit != u.end(); ++uit) {
		uint32_t value = *uit;
		uint32_t count = 0;
		while (eit != end() && value == *eit) {
			count++;
			eit++;
		}
		if (count > 1) {
			rval += count;
		}
	}
	return rval;
}

void
ECBs::dupesOnly() {
	ECBs u(*this);
	ECBs dupes;
	sort();
	u.unique();
	u.sort();
	ECBs::iterator eit = begin();
	for (ECBs::iterator uit = u.begin(); uit != u.end(); ++uit) {
		uint32_t value = *uit;
		uint32_t count = 0;
		while (eit != end() && value == *eit) {
			count++;
			eit++;
		}
		for (int i=0; count > 1 && i < count; i++) {
			dupes.push_back(value);
		}
	}
	clear();
	insert(begin(), dupes.begin(), dupes.end());
}
