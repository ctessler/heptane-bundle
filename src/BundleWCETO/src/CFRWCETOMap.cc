#include "CFRWCETOMap.h"
#define PARANOIA

ThreadWCETOMap*
CFRWCETOMap::present(CFR *cfr) {
	CFRWCETOMap::iterator cwit = find(cfr);
	if (cwit != end()) {
		return cwit->second;
	}
	return NULL;
}

ThreadWCETOMap*
CFRWCETOMap::request(CFR *cfr) {
	ThreadWCETOMap *twmap = present(cfr);
	if (twmap) {
		return twmap;
	}
	twmap = new ThreadWCETOMap();
	insert(pair<CFR*, ThreadWCETOMap*>(cfr, twmap));

#ifdef PARANOIA
	CFRWCETOMap::iterator cwit = find(cfr);
	if (cwit->second != twmap) {
		throw runtime_error("CFRWCETOMap::request mismatch in rval");
	}
	cout << "CFRWECTOMap::request inserted a new thread map ("
	     << cfr << ", " << twmap << ")" << endl;
	cout << "CFRWCETOMap::request " << *cfr << endl;
	cout << twmap->str("CFRWCETOMap::request ") << endl;
#endif
	return twmap;
}
CFRWCETOMap::~CFRWCETOMap() {
	for (CFRWCETOMap::iterator cwit = begin(); cwit != end(); ++cwit) {
		delete cwit->second;
	}
}

std::ostream &
operator<<(std::ostream& stream, const CFRWCETOMap& cwmap) {
	stream << cwmap.str();
	return stream;
}

string
CFRWCETOMap::str(string pfx) const {
	stringstream ss;
	CFRWCETOMap::const_iterator twit;
	int tws=0;
	if (!empty()) {
		tws = size();
	}
	int count = 1;
	for (twit = begin(); twit != end(); ++twit) {
		CFR *cfr = twit->first;
		ThreadWCETOMap *twmap = twit->second;
		if (count > 1) {
			ss << endl;
		}
		ss << pfx << cfr << " " << count << "/" << tws
		   << " " << *cfr << endl
		   << twmap->str(pfx);
		count++;
	}
	return ss.str();
}
