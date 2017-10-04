#include <cppunit/TextOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "CFG.h"
#include "CFR.h"

int
main(int argc, char** argv) {
	CppUnit::Test *suite =
		CppUnit::TestFactoryRegistry::getRegistry().makeTest();

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(suite);

	if (runner.run()) {
		return 0;
	}
	return 1;
}

#if 0
#include "CFR.h"
#include "DOTFactory.h"
#include "DOTfromCFR.h"

static bool
class_test() {
	
	return true;
}
	
	
static bool
basic_test() {
	CFG cfg;
	ListDigraph::Node b = cfg.addNode();
	ListDigraph::Node a = cfg.addNode();

	CFR cfr(cfg);
	b = cfr.addNode(a);


	return true;
}

static bool
dynamic_basic_test() {
	CFG cfg;
	ListDigraph::Node a = cfg.addNode();
	ListDigraph::Node b = cfg.addNode();

	/* Dynamic CFR */
	CFR *cfr = new CFR(cfg);
	ListDigraph::Node initial = cfr->addNode(b);
	delete cfr;
}
	

bool
CFR::test() {
	if (!class_test()) {
		return false;
	}
	if (!basic_test()) {
		return false;
	}
	return true;

	if (!dynamic_basic_test()) {
		return false;
	}
	
	CFG cfg;
	ListDigraph::Node a = cfg.addNode();
	ListDigraph::Node b = cfg.addNode();
	FunctionCall call("main", 0x200);
	cfg.setFunction(a, call);
	cfg.setFunction(b, call);
	cfg.setAddr(a, 0x400);
	cfg.setAddr(b, 0x404);
	cfg.addArc(a, b);
	cfg.setInitial(a);

	a = cfg.addNode();
	cfg.setFunction(a, call);
	cfg.setAddr(a, 0x408);
	cfg.addArc(b, a);

	/* Dynamic CFR */
	CFR *cfr = new CFR(cfg);
	delete cfr;

	cfr = new CFR(cfg); delete cfr;
	cfr = new CFR(cfg); delete cfr;
	cfr = new CFR(cfg); delete cfr;

	cfr = new CFR(cfg);
	ListDigraph::Node initial = cfr->addNode(b);
	cfr->setInitial(initial, b);
	ListDigraph::Node term = cfr->addNode(a);
	cfr->addArc(initial, term);

	DOTfromCFR cfr_dot(*cfr);
	cfr_dot.setPath("cfr-test.dot");
	cfr_dot.produce();
	delete cfr;

	return true; /* success */
}
#endif
