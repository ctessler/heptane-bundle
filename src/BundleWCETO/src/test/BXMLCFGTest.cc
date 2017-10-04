#include "BXMLCFGTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(BXMLCFGTest);

void
BXMLCFGTest::setUp()
{
}

void
BXMLCFGTest::tearDown()
{
}

void
BXMLCFGTest::read()
{
	/* Test the method first */
	PolicyLRU *lru = new PolicyLRU;

	Cache cache(32, 2, 32, 10);
	cache.setPolicy(lru);
	Cache copy(cache);
	delete lru;

	BXMLCfg cfg("BXMLCFGTest.xml");
}

