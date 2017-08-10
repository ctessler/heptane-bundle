#include "CacheLine.h"

void
CacheLine::_run_check() const {
	if (_empty && (_start != 0)) {
		throw runtime_error("Cannot be empty and have a start address");
	}
	if (_empty && (_end != 0)) {
		throw runtime_error("Cannot be empty and have an end address");
	}
}

CacheLine::CacheLine() {
	_size = _start = _end = 0;
	_empty = true;
	_run_check();
}

CacheLine::CacheLine(uint32_t nbytes) {
	_run_check();
	resize(nbytes);
}

void
CacheLine::resize(uint32_t nbytes) {
	_run_check();
	_size = nbytes;
	_start = _end = 0;
	_empty = true;
}

void
CacheLine::clear() {
	_run_check();
	resize(_size);
}

bool
CacheLine::present(t_address addr) const {
	_run_check();
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
	_run_check();
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
	_run_check();
	return addr - _start;
	
}

CacheLine& CacheLine::operator=(const CacheLine& rhs) {
	_start = rhs._start;
	_end = rhs._end;
	_empty = rhs._empty;
	_run_check();
	return (*this);
}

