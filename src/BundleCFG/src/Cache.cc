#include "Cache.h"

Cache::Cache(Cache& other) {
	_policy = other._policy;
	_nsets = other._nsets;
	_ways = other._ways;
	_linesize = other._linesize;
	_latency = other._latency;

	map<int, CacheSet*>::iterator it = other._sets.begin();
	for (; it != other._sets.end(); it++) {
		_sets[it->first] = new CacheSet(*(it->second));
	}
}

CacheSet* Cache::setOf(iaddr_t addr) {
	uint32_t set = setIndex(addr);

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

uint32_t Cache::setIndex(iaddr_t addr) const {
	iaddr_t begin = addr - (addr % _linesize);
	uint32_t set = (begin / _linesize) % _nsets;

	return set;
}

void Cache::insert(iaddr_t addr) {
	CacheSet *cs = setOf(addr);
	cs->insert(addr);
}

Cache::~Cache() {
	/* One policy pointer per policy type is leaked.
	 * This is because the copy constructor cannot instance the
	 * derived type. It can be fixed, for the moment the single
	 * object is leaked.
	if (_policy) {
		delete _policy;
		_policy = NULL;
	}
	*/
	map<int, CacheSet*>::iterator it;
	for (it = _sets.begin(); it != _sets.end(); it++) {
		it->second->clear();
		delete it->second;
	}
	_sets.clear();
}


