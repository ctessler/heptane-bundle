#ifndef SET_TEST_H
#define SET_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include <set>
#include <iostream>
using namespace std;
using namespace lemon;

class SetTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(SetTest);
	CPPUNIT_TEST(basic);
	CPPUNIT_TEST(reverse);
	CPPUNIT_TEST(node);
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void basic();
	void reverse();
	void node();
};

#endif /* SET_TEST_H */
