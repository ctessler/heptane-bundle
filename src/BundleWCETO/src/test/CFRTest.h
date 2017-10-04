#ifndef CFRTEST_H
#define CFRTEST_H

#include "CFG.h"
#include "CFR.h"

#include <cppunit/extensions/HelperMacros.h>

class CFRTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(CFRTest);
	CPPUNIT_TEST(corruption);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void corruption();
};

#endif
