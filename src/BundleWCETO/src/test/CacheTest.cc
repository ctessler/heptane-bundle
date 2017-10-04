#include "CacheTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CacheTest);

void
CacheTest::setUp()
{
}

void
CacheTest::tearDown()
{
}

void
CacheTest::destructor()
{
	return;
	PolicyLRU lru;
	Cache original(32, 2, 32, 10, &lru);

	original.insert(0x4000);
}

void
CacheTest::setInsert()
{
	int line_size = 32;
	
	/* Test the method first */
	map<uint32_t, CacheLine*> _storage;
	iaddr_t addr= 0x4000;
	int cache_line = addr % line_size;
	CacheLine *cline = new CacheLine;
	_storage.insert(make_pair(cache_line, cline));
	_storage[cache_line]->resize(line_size);

	map<uint32_t, CacheLine*>::iterator it;
	for (it = _storage.begin(); it != _storage.end(); it++) {
		delete it->second;
	}
	
	PolicyLRU lru;
	Cache original(32, 2, line_size, 10, &lru);
	CacheSet set(&original);

	set.insert(0x4000);
	set.clear();
}

void
CacheTest::lineInsert()
{
	int line_size = 32;
	CacheLine line;
	line.resize(line_size);
	line.store(0x4000);
}

