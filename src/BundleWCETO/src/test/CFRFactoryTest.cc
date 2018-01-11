#include "CFRFactoryTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CFRFactoryTest);

void
CFRFactoryTest::setUp()
{
}

void
CFRFactoryTest::tearDown()
{
}

void
CFRFactoryTest::produceLeak()
{
	PolicyLRU lru;
	Cache cache(32, 2, 32, 10, 100, &lru);
	CFG cfg;

	ListDigraph::Node a = cfg.addNode();
	cfg.setInitial(a);
	cfg.setAddr(a, 0x4000);

	ListDigraph::Node b = cfg.addNode();
	cfg.addArc(a, b);
	cfg.setAddr(b, 0x4004);

	CFRFactory fact(cfg, cache);
	fact.produce();
}

void
CFRFactoryTest::basic()
{
	cout << "CFRFactoryTest::basic" << endl;
	PolicyLRU lru;
	Cache cache(32, 2, 32, 10, 100, &lru);
	CFG cfg;

	ListDigraph::Node a = cfg.addNode();
	cfg.setInitial(a);
	cfg.setAddr(a, 0x4000);

	CFRFactory fact(cfg, cache);
	fact.produce();
}

