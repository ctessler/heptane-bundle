#ifndef CACHETEST_H
#define CACHETEST_H
#include <cppunit/extensions/HelperMacros.h>

#include "Cache.h"
#include "PolicyLRU.h"

class CacheTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CacheTest);
	CPPUNIT_TEST(destructor);
	CPPUNIT_TEST(setInsert);
	CPPUNIT_TEST(lineInsert);
	CPPUNIT_TEST(copySet);
	CPPUNIT_TEST(cacheCopy);		
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void destructor();
	void setInsert();
	void lineInsert();
	void copySet();
	void cacheCopy();
};

#endif
