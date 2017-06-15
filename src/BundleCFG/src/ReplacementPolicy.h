#ifndef REPLACEMENTPOLICY_H
#define REPLACEMENTPOLICY_H

#include <stdint.h>
#include <string>
#include "HeptaneStdTypes.h"
using namespace std;

/**
 * Forward declarations
 */
class CacheSet;

/**
 * A parenting class for more specific replacement policies
 */
class ReplacementPolicy {
public:
	/**
	 * Returns the index cache line the address will be stored in.
	 *
	 * Does not modify the contents of the cache set
	 *
	 * @param[in] set the CacheSet where the @addr will be placed
	 * @param[in] addr the address of the value
	 */
	virtual uint32_t lineOf(CacheSet& set, t_address addr) = 0;
	/**
	 * Stores the block that addr belongs to in the cache
	 *
	 * MODIFIES the cache set
	 *
	 * @param[in] set the CacheSet where the block containing
	 * 	      @addr will be placed 
	 * @param[in] addr the address (and block) being stored
	 */
	virtual void store(CacheSet& set, t_address addr) = 0;
	/**
	 * Returns true if storing the address in the cache set would
	 * evict another value
	 *
	 * @param[in] set the CacheSet
	 * @param[in] addr the address that may cause an eviction
	 */
	virtual bool evicts(CacheSet& set, t_address addr) = 0;
	
	/**
	 * Returns the name of the replacement policy
	 */
	virtual string typeName() = 0;
};

#include "CacheSet.h"

#endif /* REPLACEMENTPOLICY_H */
