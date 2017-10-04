#include "CFRTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(CFRTest);

void
CFRTest::setUp()
{
}

void
CFRTest::tearDown()
{
}

void
CFRTest::corruption()
{
	CFG cfg;
	ListDigraph::Node a = cfg.addNode();
	ListDigraph::Node b = cfg.addNode();

	CFR cfr(cfg);
	ListDigraph::Node c = cfr.addNode(b);
}

