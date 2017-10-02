#ifndef CACHELINE_H
#define CACHELINE_H
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <set>

class CacheLine;

#include "CFG.h"

class CacheLine {
public:
	/**
	 * Default constructor, not in a usable state.
	 */
	CacheLine();
	/**
	 * Parameterized Constructor
	 *
	 * @param[in] nbytes the number of bytes in the cache line
	 */
	CacheLine(uint32_t nbytes);
	/**
	 * Assignment / Copy
	 */
	CacheLine& operator=(const CacheLine& rhs);
	/**
	 * Is this cache line empty?
	 */
	bool isEmpty() const { return _empty; }
	/**
	 * Resizes the cache line to store nbytes, resizing (even to
	 * the same size) will clear the contents of the line.
	 */
	void resize(uint32_t nbytes);
	/**
	 * Returns true if the data referred to by address is in the cache line
	 *
	 * @param[in] addr address being checked
	 */
	bool present(iaddr_t addr) const;
	/**
	 * Returns the size of the cache line in bytes
	 */
	uint32_t size(void) const { return _size; };
	/**
	 * "Stores" the data referred to by address in the cache line
	 */
	void store(iaddr_t addr);
	/**
	 * Clears the cache line
	 */
	void clear();
	/**
	 * Finds the beginning address of the first byte of the block containing addr.
	 */
	iaddr_t startAddress(iaddr_t addr);
	iaddr_t getStartAddress() { return _start; }
	/**
	 * Returns the beginning address of the last byte of the block containing addr.
	 */
	iaddr_t endAddress(iaddr_t addr);
	iaddr_t getEndAddress() { return _end; }	
	/**
	 * Returns the offset from the starting address of the given address
	 */
	iaddr_t offset(iaddr_t addr);
	
private:
	bool _empty;
	uint32_t _size;
	iaddr_t _start, _end;

	void _run_check() const;
};



#endif /* CACHELINE_H */
