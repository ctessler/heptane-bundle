#ifndef CACHELINE_H
#define CACHELINE_H
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <set>
#include "HeptaneStdTypes.h"

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
	bool present(t_address addr) const;
	/**
	 * Returns the size of the cache line in bytes
	 */
	uint32_t size(void) const { return _size; };
	/**
	 * "Stores" the data referred to by address in the cache line
	 */
	void store(t_address addr);
	/**
	 * Clears the cache line
	 */
	void clear();
	/**
	 * Finds the beginning address of the first byte of the block containing addr.
	 */
	t_address startAddress(t_address addr);
	t_address getStartAddress() { return _start; }
	/**
	 * Returns the beginning address of the last byte of the block containing addr.
	 */
	t_address endAddress(t_address addr);
	t_address getEndAddress() { return _end; }	
	/**
	 * Returns the offset from the starting address of the given address
	 */
	t_address offset(t_address addr);
	
private:
	bool _empty;
	uint32_t _size;
	t_address _start, _end;

	void _run_check() const;
};



#endif /* CACHELINE_H */
