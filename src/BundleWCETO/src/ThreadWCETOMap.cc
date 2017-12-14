#include "ThreadWCETOMap.h"

std::ostream &
operator<<(std::ostream& stream, const ThreadWCETOMap& twmap) {
	stream << twmap.str();
	return stream;
}

uint32_t
ThreadWCETOMap::wceto(uint32_t thread) {
	ThreadWCETOMap::iterator twit;
	uint32_t wceto = 0;
	for(twit = begin(); twit != end(); ++twit) {
		wceto += twit->second;
	}
	return wceto;
	
}

bool
ThreadWCETOMap::fill(CFR *cfr, uint32_t threads) {
	insert(pair<uint32_t, uint32_t>(0, 0));
	for (uint32_t i=1; i <= threads; i++) {
		uint32_t wceto = cfr->wceto(i) - cfr->wceto(i -1);
		insert(pair<uint32_t, uint32_t>(i, wceto));
	}
	return true;
}

bool
ThreadWCETOMap::fill(ThreadWCETOMap& src) {
	clear();
	ThreadWCETOMap::iterator tit;
	for (tit = src.begin(); tit != src.end(); ++tit) {
		insert(make_pair(tit->first, tit->second));
	}
	return true;
}

string
ThreadWCETOMap::str(string pfx) const {
	stringstream ss;
	ThreadWCETOMap::const_iterator twit;
	int nthreads=0;
	if (empty()) {
		return "";
	} else {
		nthreads = size() - 1;
	}
	ss << pfx << "Thread\tWCETO";
	for (twit = begin(); twit != end(); ++twit) {
		uint32_t threads = twit->first;
		uint32_t wceto = twit->second;
		ss << endl << pfx << threads << "/" << nthreads << "\t" << wceto;
	}
	return ss.str();
}

ThreadWCETOMap::ThreadWCETOMap(ThreadWCETOMap &src) {
	ThreadWCETOMap::iterator twit;
	for (twit = src.begin(); twit != src.end(); ++twit) {
		insert(pair<uint32_t, uint32_t>(twit->first, twit->second));
	}
}

ThreadWCETOMap::ThreadWCETOMap(ThreadWCETOMap *src) {
	ThreadWCETOMap::iterator twit;
	for (twit = src->begin(); twit != src->end(); ++twit) {
		insert(pair<uint32_t, uint32_t>(twit->first, twit->second));
	}
}
