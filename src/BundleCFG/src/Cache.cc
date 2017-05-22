#include "Cache.h"
/*
 * CacheSet
 */
uint32_t CacheSet::offsetOf(t_address addr) const {
	uint32_t linesize = _cache->getLineSize();

	/* addr and linesize are both in bytes */
	uint32_t set = addr % linesize;
	return (set);
}


bool CacheSet::present(t_address addr) const {
	uint32_t offset = offsetOf(addr);
	uint32_t ways = _contents.size();
	for (uint32_t line=0; line < ways; line++) {
		if (_contents[line][offset] == addr) {
			return true;
		}
	}
	return false;
}

void CacheSet::insert(t_address addr) {
	ReplacementPolicy *p = _cache->getPolicy();
	uint32_t line = p->lineOf(*this, addr);
	uint32_t offset = offsetOf(addr);
	_contents[line][offset] = addr;
}

/**
 * Each call to lineOf updates the "recent-ness" of the address.
 *
 * The most recent value is placed in line[0]
 * The least recent value is placed in line[ways - 1]
 *
 * @return the line the address should be placed in.
 */
uint32_t PolicyLRU::lineOf(CacheSet &set, t_address addr) {
	uint32_t ways = set._contents.size();
	CacheLine ph; /* The new cache line */
	ph.reserve(ways);
	if (set.present(addr)) {
		/* Find the line, copy, and remove it */
		uint32_t offset = set.offsetOf(addr);
		vector<CacheLine>::iterator it;
		for (it = set._contents.begin(); it < set._contents.end(); it++) {
			if ((*it)[offset] == addr) {
				/* Found it */
				ph = *it; /* Copy */
				set._contents.erase(it); /* Remove */
				break;
			}
		}
	}
	/*
	 * Adds the place holder as the most recent line, it may be
	 * an empty cache line or have copied values from a present
	 * value. 
	 */
	set._contents.insert(set._contents.begin(), ph);

	/* If there is no room, remove the oldest lines	 */
	while (set._contents.size() >= ways) {
		set._contents.erase(set._contents.end() - 1);
	}

	/* Always place the value in the most recent position */
	return 0;
}

CacheSet* Cache::setOf(t_address addr) {
	t_address begin = addr - (addr % _linesize);
	uint32_t set = (begin / _linesize) % _nsets;

	map<int, CacheSet*>::iterator it = _sets.find(set);
	if (it != _sets.end()) {
		/* The set exists */
		return it->second;
	}
	/* The set does not exist */
	CacheSet *rval = new CacheSet(this);
	_sets[set] = rval;
	return rval;
}

Cache::~Cache() {
	if (_policy) {
		delete _policy;
	}
	map<int, CacheSet*>::iterator it;
	for (it = _sets.begin(); it != _sets.end(); it++) {
		delete it->second;
		_sets.erase(it);
	}
}
