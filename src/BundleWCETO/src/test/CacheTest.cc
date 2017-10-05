#include "CacheTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CacheTest);

#define LINE_SIZE 32
#define NSETS 32
#define NWAYS 2
#define LATENCY 10

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
	Cache original(NSETS, NWAYS, LINE_SIZE, LATENCY, &lru);

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
	Cache original(NSETS, NWAYS, LINE_SIZE, LATENCY, &lru);
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

void
CacheTest::copySet()
{
	PolicyLRU lru;
	Cache original(NSETS, NWAYS, LINE_SIZE, LATENCY, &lru);
	CacheSet *set = new CacheSet(&original);
	set->insert(0x4000);
	set->insert(0x4004);
	set->insert(0x4008);
	
	Cache *cache_copy = new Cache(original);
	Cache *copy_two = new Cache(*cache_copy);
	

	CacheSet *copy = new CacheSet(*set);
	delete set;
	delete copy;

	delete cache_copy;
	delete copy_two;
}

void
CacheTest::cacheCopy()
{
	PolicyLRU lru;
	//Cache *orig = new Cache(NSETS, NWAYS, LINE_SIZE, LATENCY, &lru);

	map<int, Cache*> omap;
	map<int, Cache*>::iterator mit;
	for (int i = 0; i < 3; i++) {
		Cache *c = new Cache(NSETS, NWAYS, LINE_SIZE, LATENCY, &lru);
		omap.insert(make_pair(i, c));
	}

	for (mit = omap.begin(); mit != omap.end(); ++mit) {
		delete mit->second;
	}
}
