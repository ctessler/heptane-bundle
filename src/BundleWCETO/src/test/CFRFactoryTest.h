#ifndef CFRFACTORYTEST_H
#define CFRFACTORYTEST_H

#include "CFRFactory.h"
#include "PolicyLRU.h"

#include <cppunit/extensions/HelperMacros.h>

class CFRFactoryTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CFRFactoryTest);
	CPPUNIT_TEST(basic);
	CPPUNIT_TEST(produceLeak);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void produceLeak();
	void basic();	
};

#endif
