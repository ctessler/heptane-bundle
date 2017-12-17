#include "ThreadECBMap.h"

std::ostream &
operator<<(std::ostream& stream, const ThreadECBMap& twmap) {
	stream << twmap.str();
	return stream;
}

void
ThreadECBMap::_empty() {
	ThreadECBMap::iterator it;
	for (it = begin(); it != end(); ++it) {
		delete it->second;
	}
}

ThreadECBMap::~ThreadECBMap() {
	_empty();
	clear();
}

ECBs*
ThreadECBMap::ecbs(uint32_t threads) {
	ThreadECBMap::iterator it;
	ECBs* rval = new ECBs();
	for(it = begin(); it != end(); ++it) {
		ECBs* tlevel = it->second;
		rval->insert(rval->begin(), tlevel->begin(), tlevel->end());
	}
	return rval;
	
}

bool
ThreadECBMap::fill(CFR *cfr, uint32_t threads) {
	_empty();
	clear();
	/* 0th thread, zero ECBs */
	insert(pair<uint32_t, ECBs *>(0, new ECBs()));

	if (threads < 1) {
		/* all done */
		return true;
	}

	/* 1st thread, only one with ECBs */
	cfr->calcECBs();
	insert(pair<uint32_t, ECBs*>(0, cfr->ECBs()));
	
	/* 2nd -> final thread, zero ECBs */
	for (uint32_t i=2; i <= threads; i++) {
		insert(pair<uint32_t, ECBs*>(i, new ECBs()));
	}
	return true;
}

bool
ThreadECBMap::fill(ThreadECBMap& src) {
	ThreadECBMap::iterator it;
	for (it = src.begin(); it != src.end(); ++it) {
		
		insert(make_pair(it->first, it->second));
	}
	return true;
}

string
ThreadECBMap::str(string pfx) const {
	stringstream ss;
	ThreadECBMap::const_iterator it;
	int nthreads=0;
	if (empty()) {
		return "";
	} else {
		nthreads = size() - 1;
	}
	ss << pfx << "Thread\tECBs";
	for (it = begin(); it != end(); ++it) {
		uint32_t threads = it->first;
		ss << endl << pfx << threads << "/" << nthreads << "\t";
		ECBs *ecbs = it->second;
		ECBs::iterator eit;
		for (eit = ecbs->begin(); eit != ecbs->end(); ++eit) {
			ss << (*eit) << " ";
		}
	}
	return ss.str();
}

ThreadECBMap::ThreadECBMap(ThreadECBMap &src) {
	ThreadECBMap::iterator twit;
	for (twit = src.begin(); twit != src.end(); ++twit) {
		ECBs *val = new ECBs(*(twit->second));
		insert(pair<uint32_t, ECBs*>(twit->first, val));
	}
}

ThreadECBMap::ThreadECBMap(ThreadECBMap *src) {
	ThreadECBMap::iterator twit;
	for (twit = src->begin(); twit != src->end(); ++twit) {
		ECBs *val = new ECBs(*(twit->second));
		insert(pair<uint32_t, ECBs*>(twit->first, val));
	}
}
