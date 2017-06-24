#include "PolicyLRU.h"


uint32_t
PolicyLRU::lineOf(CacheSet& set, t_address addr) {
	/*
	 * Least Recently Used is decayed first, so the youngest item
	 * will be stored at index 0 always.
	 */
	return 0;
}

void
PolicyLRU::store(CacheSet& set, t_address addr) {
	uint32_t line;
	for (line=0; line < set._ways; line++) {
		if (set._storage[line].present(addr)) {
			/* Found it! */
			break;
		}
	}
	if (line != set._ways && line != 0) {
		/*
		 * The address is present in the cache, everything
		 * else needs to be moved down.
		 */
		for (uint32_t tgt = line; tgt > 0; tgt--) {
			set._storage[tgt] = set._storage[tgt - 1];
		}
	}
	set._storage[0].store(addr);
}


bool
PolicyLRU::evicts(CacheSet& set, t_address addr) {
	uint32_t count=0;
	map<uint32_t, CacheLine>::iterator it;	
	for (it = set._storage.begin(); it != set._storage.end(); it++) {
		if (it->second.present(addr)) {
			/* Present, won't evict */
			return false;
		}
	}
	if ((count + 1) == set._ways) {
		/* No empty line */
		return true;
	}
	return false;
}
