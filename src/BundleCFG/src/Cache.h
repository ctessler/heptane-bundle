#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <stdint.h>
#include "HeptaneStdTypes.h"
#include <vector>
#include <map>
#include <algorithm>

/**
 * Forward declarations
 */
class CacheSet;
class ReplacementPolicy;

/**
 * Reference information:
 *
 * MIPS32 74K Level 1 Cache Configuration
 *   32 *byte* line size
 *   4-way associative
 *   0,16,32, or 64 kilobytes in size
 */
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
	Cache(Cache &other);
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
	/**
	 * Returns the index of the set the address *would* be cached in
	 */
	uint32_t setIndex(t_address addr) const;
	/**
	 * Returns true if the address is currently cached.
	 */
	bool present(t_address addr);
private:
	map <int, CacheSet *> _sets;
	ReplacementPolicy *_policy;
	uint32_t _nsets;
	uint32_t _ways;
	uint32_t _linesize; /** Size of each cache line in bytes */
	uint32_t _latency; /** The access time of this cache */
};

#include "CacheSet.h"
#include "ReplacementPolicy.h"

#endif /* CACHE_H */
