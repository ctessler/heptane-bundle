#ifndef CACHE_SET_H
#define CACHE_SET_H
#include "CacheLine.h"
#include "Cache.h"
#include "CFG.h"
#include <map>
#include <vector>
#include <stdexcept>

/**
 * Represents an individual Cache Set
 *
 * Each cache set is indexed from 0->(N-1) where N is the number of
 * ways in the cache. A 2 way associative cache has 2 ways, or sets
 * with lines = {0,1}
 */
class CacheSet {
	friend class PolicyLRU;
public:
	CacheSet(Cache* cache);
	/**
	 * Specialized Copy Constructor
	 */
	CacheSet(CacheSet &other);
	/**
	 * Destructor
	 */
	~CacheSet();
	/**
	 * Returns true if the address is present in the set.
	 */
	bool present(iaddr_t addr);
	/**
	 * Returns true if placing the address in the set would cause
	 * an eviction
	 */
	bool evicts(iaddr_t addr);
	/**
	 * Adds an address to the set, possibly evicting other values
	 * based on the ReplacementPolicy of the parenting Cache.
	 */
	void insert(iaddr_t addr);
	/**
	 * Clears the cache set
	 */
	void clear();
	/**
	 * Returns true if the cache set is empty
	 */
	bool empty();
	/**
	 * Returns true if the cache ses is full
	 */
	bool isFull();
	/**
	 * Returns a pointer to the cache line at the index
	 */
	CacheLine* cacheLine(uint32_t index);
	/**
	 * Returns the number of ways in the cache set
	 */
	uint32_t ways() { return _ways; }
private:
	/**
	 * Storage for the Cache Set, each of the ways is given a CacheLine
	 */
	map<uint32_t, CacheLine*> _storage;
	/**
	 * The number of lines in the cache set, the associativity
	 */
	uint32_t _ways;
	/**
	 * Cache this set belongs to
	 */
	const Cache *_cache;
};
#endif /* CACHE_SET_H */
