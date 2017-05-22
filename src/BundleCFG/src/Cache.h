#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <stdint.h>
#include "HeptaneStdTypes.h"
#include <vector>
#include <map>
#include <algorithm>

/**
 * Reference information:
 *
 * MIPS32 74K Level 1 Cache Configuration
 *   32 *byte* line size
 *   4-way associative
 *   0,16,32, or 64 kilobytes in size
 */


class Cache;
typedef vector <t_address> CacheLine;

/**
 * Represents an individual Cache Set
 *
 * Each cache set is indexed from 0->(N-1) where N is the number of
 * ways in the cache. A 2 way associative cache has 2 ways, or sets
 * with lines = {0,1}
 *
 */
class CacheSet {
	friend class PolicyLRU;
public:
	CacheSet(Cache *cache) : _cache(cache) { };
	/**
	 * Specialized Copy Constructor
	 */
	 CacheSet(Cache *cache, CacheSet &other) : _cache(cache) {
		vector<CacheLine>::iterator it = other._contents.begin();
		for (; it != other._contents.end(); it++) {
			/* it is a pointer to a vector of t_addresses */
			CacheLine line;
			line.reserve(it->size());
			copy(it->begin(), it->end(), back_inserter(line));
			_contents.push_back(line);
		}
	}
	/**
	 * Returns true if the address is present in the set.
	 */
	bool present(t_address addr) const;
	/**
	 * Returns the index of the starting byte the address would be cached in
	 */
	uint32_t offsetOf(t_address addr) const;
	/**
	 * Adds an address to the set, possibly evicting other values
	 * based on the ReplacementPolicy of the parenting Cache.
	 */
	void insert(t_address addr);
	/**
	 * Clears the cache set
	 */
	void clear();
private:
	/**
	 * Cache this set belongs to
	 * Provides the number of ways and line size
	 */
	const Cache *_cache;
	/**
	 * The cached values
	 */
	vector <CacheLine> _contents;
};

/**
 * A parenting class for more specific replacement policies
 */
class ReplacementPolicy {
public:
	/**
	 * Called by CacheSet before any read or write of the
	 * address. Always returns the cache line where the address
	 * should be found. 
	 *
	 * The replacement policy modifies the cache set, and *MUST*
	 * fully prepare the cache line the address will be stored
	 * into.  
	 * 
	 *
	 * @param[in] set the CacheSet where the @addr will be placed
	 * @param[in] addr the address of the value
	 *
	 * @return the index of the cache line that will be replaced by addr.
	 */
	virtual uint32_t lineOf(CacheSet &set, t_address addr) = 0;
	/**
	 * Returns the name of the replacement policy
	 */
	virtual string typeName() = 0;
	virtual ~ReplacementPolicy() { };
};

/**
 * LRU Least Recently Used
 */
class PolicyLRU : public ReplacementPolicy {
public:
	uint32_t lineOf(CacheSet &set, t_address addr);
	string typeName() { return "LRU"; }
	~PolicyLRU() {}
};
class Cache {
public:
	Cache(int nSets, int nWays, int lineSize, int latency, ReplacementPolicy *p = NULL) {
		_policy = p;
		_nsets = nSets;
		_ways = nWays;
		_linesize = lineSize;
		_latency = latency;
	};  
	~Cache();
	/**
	 * Copy Constructor
	 */
	Cache(Cache &other) {
		_policy = other._policy;
		_nsets = other._nsets;
		_ways = other._ways;
		_linesize = other._linesize;
		_latency = other._latency;

		map<int, CacheSet *>::iterator it = other._sets.begin();
		for (; it != other._sets.end(); it++) {
			_sets[it->first] = new CacheSet(this, *(it->second));
		}
	}
	/**
	 * Returns a reference to the replacement policy
	 */
	ReplacementPolicy *getPolicy() const { return _policy; };
	/**
	 * Sets the ReplacementPolicy
	 */
	void setPolicy(ReplacementPolicy *p) {
		_policy = p;
	}
	/**
	 * Returns the number of cache sets in the cache.
	 */
	uint32_t getSets() const { return _nsets; };
	/**
	 * Returns the associativity of the cache
	 *
	 * The term ways is used to be consistent with the rest of Heptane.
	 */
	uint32_t getWays() const { return _ways; };
	/**
	 * Returns the size of each cache line in bytes
	 */
	uint32_t getLineSize() const { return _linesize; }
	/**
	 * Returns the cache set associated with the address
	 *
	 * This is the cache set being used, modification will effect
	 * the cache contents.
	 */
	CacheSet *setOf(t_address addr);
private:
	map <int, CacheSet *> _sets;
	ReplacementPolicy *_policy;
	uint32_t _nsets;
	uint32_t _ways;
	uint32_t _linesize; /** Size of each cache line in bytes */
	uint32_t _latency; /** The access time of this cache */
};

#endif /* CACHE_H */
