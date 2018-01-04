#ifndef FUNCTIONCALLTEST_H
#define FUNCTIONCALLTEST_H
#include <cppunit/extensions/HelperMacros.h>

#include "FunctionCall.h"

class FunctionCallTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(FunctionCallTest);
	CPPUNIT_TEST(copy_construct);
	CPPUNIT_TEST(callstack_str);
	CPPUNIT_TEST(basic);
	CPPUNIT_TEST(push_const);
	CPPUNIT_TEST(stack_const);	
	CPPUNIT_TEST_SUITE_END();
public:
	void setUp();
	void tearDown();

	void copy_construct();
	void callstack_str();
	void basic();
	void push_const();
	void stack_const();
};

#endif
