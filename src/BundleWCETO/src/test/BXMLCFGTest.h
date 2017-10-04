#ifndef BXMLCFGTEST_H
#define BXMLCFGTEST_H
#include <cppunit/extensions/HelperMacros.h>

#include "BXMLCfg.h"

class BXMLCFGTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(BXMLCFGTest);
	CPPUNIT_TEST(read);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void read();
};

#endif
