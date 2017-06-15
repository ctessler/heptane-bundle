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

CacheSet* Cache::setOf(t_address addr) {
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

uint32_t Cache::setIndex(t_address addr) const {
	t_address begin = addr - (addr % _linesize);
	uint32_t set = (begin / _linesize) % _nsets;

	return set;
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


