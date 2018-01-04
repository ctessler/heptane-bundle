#include "FunctionCallTest.h"
#include<iostream>

CPPUNIT_TEST_SUITE_REGISTRATION(FunctionCallTest);

void
FunctionCallTest::setUp()
{
}

void
FunctionCallTest::tearDown()
{
}

void
FunctionCallTest::copy_construct()
{
	CallStack start;
	start.push(0);
	start.push(12);
	start.push(18);

	CallStack end(start);

	CPPUNIT_ASSERT_MESSAGE("Call stacks are unequal", start == end);
}

void
FunctionCallTest::callstack_str()
{
	CallStack empty;
	CPPUNIT_ASSERT_MESSAGE("Empty stack should be 'T[]'",
			       empty.str().compare("T[]") == 0);
	CallStack three;
	three.push(12); three.push(14); three.push(45);
	CPPUNIT_ASSERT_MESSAGE("Three should be: T[0x2d, 0xe, 0xc]",
			       three.str().compare("T[0x2d, 0xe, 0xc]") == 0);
	CallStack copy(three);
	CPPUNIT_ASSERT_MESSAGE("Copy should be: T[0x2d, 0xe, 0xc]",
			       copy.str().compare("T[0x2d, 0xe, 0xc]") == 0);
			       
}

void
FunctionCallTest::basic()
{
	FunctionCall empty;
	CPPUNIT_ASSERT_MESSAGE("Empty is -NA-:T[]",
			       empty.str().compare("-NA-:T[]") == 0);
	FunctionCall start("main");
	start.stack().push(0x40000);
	CPPUNIT_ASSERT_MESSAGE("Main is main:T[0x40000]",
			       start.str().compare("main:T[0x40000]") == 0);
}

void
FunctionCallTest::push_const()
{
	FunctionCall start("main"); start.stack().push(0x40000);
	FunctionCall next(start, "sort", 0x40042);

	CPPUNIT_ASSERT_MESSAGE("next is sort:T[0x40042, 0x40000]",
	    next.str().compare("sort:T[0x40042, 0x40000]") == 0);

	FunctionCall last(next, "pew!", 0x50134);
	CPPUNIT_ASSERT_MESSAGE("last is pew!:T[0x50134, 0x40042, 0x40000]",
	    last.str().compare("pew!:T[0x50134, 0x40042, 0x40000]") == 0);
}

void
FunctionCallTest::stack_const()
{
	FunctionCall start("put", CallStack({0x4020, 0x4000}));
	CPPUNIT_ASSERT_MESSAGE("start is put:T[0x4020, 0x4000]",
	    start.str().compare("put:T[0x4020, 0x4000]") == 0);

	FunctionCall next(start, "loop", 0x5080);
	CPPUNIT_ASSERT_MESSAGE("next is loop:T[0x5080, 0x4020, 0x4000]",
	    next.str().compare("loop:T[0x5080, 0x4020, 0x4000]") == 0);
}

