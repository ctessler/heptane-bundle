#include "CacheLine.h"

CacheLine::CacheLine() {
	_size = _start = _end = 0;
	_empty = true;
}

CacheLine::CacheLine(uint32_t nbytes) {
	resize(nbytes);
}

void
CacheLine::resize(uint32_t nbytes) {
	_size = nbytes;
	_start = _end = 0;
	_empty = true;
}

void
CacheLine::clear() {
	resize(_size);
}

bool
CacheLine::present(t_address addr) const {
	if (_empty) {
		return false;
	}

	/* If the address is within the range, it's stored */
	return (addr >= _start) && (addr <= _end);
}

void
CacheLine::store(t_address addr) {
	if (_size <= 0) {
		throw runtime_error("Cannot call store without setting a size");
	}
	_start = startAddress(addr);
	_end = endAddress(addr);
	_empty = false;
}

t_address
CacheLine::startAddress(t_address addr) {
	if (_size <= 0) {
		throw runtime_error("Cannot call startAddress without setting a size");
	}
	return addr - (addr % _size);
}

t_address
CacheLine::endAddress(t_address addr) {
	if (_size <= 0) {
		throw runtime_error("Cannot call endAddress without setting a size");
	}
	return addr - (addr % _size) + _size - 1;
}

t_address
CacheLine::offset(t_address addr) {
	return addr - _start;
	
}

CacheLine& CacheLine::operator=(const CacheLine& rhs) {
	_start = rhs._start;
	_end = rhs._end;
	_empty = rhs._end;

	return (*this);
}

