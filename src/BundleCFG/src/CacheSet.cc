#include "CacheSet.h"

CacheSet::CacheSet(Cache* cache) : _cache(cache) {
	_ways = cache->getWays();;
}

CacheSet::CacheSet(CacheSet &other) : _cache(other._cache) {
	_ways = other._ways;
	for (uint32_t line=0; line < _ways; line++) {
		_storage[line] = other._storage[line];
	}
}

bool CacheSet::present(t_address addr) {
	for (uint32_t line=0; line < _ways; line++) {
		if (_storage[line].present(addr)) {
			return true;
		}
	}
	return false;
}

bool CacheSet::evicts(t_address addr) {
	ReplacementPolicy *p = _cache->getPolicy();
	return p->evicts(*this, addr);
}

void CacheSet::insert(t_address addr) {
	ReplacementPolicy *p = _cache->getPolicy();
	uint32_t line = p->lineOf(*this, addr);
	_storage[line].resize(_cache->getLineSize());
	_storage[line].store(addr);

}

void CacheSet::clear() {
	map<uint32_t, CacheLine>::iterator it;
	for (it = _storage.begin(); it != _storage.end(); it++) {
		it->second.clear();
	}
}
