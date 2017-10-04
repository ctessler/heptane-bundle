#ifndef POLICYLRU_H
#define POLICYLRU_H
#include "ReplacementPolicy.h"
#include "CFG.h"
#include <iostream>

/**
 * LRU Least Recently Used
 */
class PolicyLRU : public ReplacementPolicy {
public:
	/**
	 * Returns the index cache line the address will be stored in.
	 *
	 * Does not modify the contents of the cache set
	 *
	 * @param[in] set the CacheSet where the @addr will be placed
	 * @param[in] addr the address of the value
	 */
	uint32_t lineOf(CacheSet& set, iaddr_t addr);
	/**
	 * Stores the block that addr belongs to in the cache
	 *
	 * MODIFIES the cache set
	 *
	 * @param[in] set the CacheSet where the block containing
	 * 	      @addr will be placed 
	 * @param[in] addr the address (and block) being stored
	 */
	void store(CacheSet& set, iaddr_t addr);
	/**
	 * Returns true if storing the address in the cache set would
	 * evict another value
	 *
	 * @param[in] set the CacheSet
	 * @param[in] addr the address that may cause an eviction
	 */
	bool evicts(CacheSet& set, iaddr_t addr);
	
	/**
	 * Returns the name of the replacement policy
	 */
	string typeName() {
		return "LRU";
	}
	~PolicyLRU() {
	}
};


#endif /* POLICYLRU_H */
