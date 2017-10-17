#ifndef PQUEUE_TEST_H
#define PQUEUE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include <queue>
#include <iostream>
using namespace std;
using namespace lemon;

class PQueueTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(PQueueTest);
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

#endif /* PQUEUE_TEST_H */
