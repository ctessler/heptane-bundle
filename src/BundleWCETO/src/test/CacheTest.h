#ifndef CACHETEST_H
#define CACHETEST_H

#include "CFG.h"
#include "CFR.h"

#include <cppunit/extensions/HelperMacros.h>

class CacheTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CacheTest);
	CPPUNIT_TEST(destructor);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void destructor();
};

#endif
