#include "CFRFactory.h"
#include "DOTFactory.h"
#include "JPGFactory.h"
#include "PolicyLRU.h"
#include "DOTfromCFR.h"

static void
init_cfg(CFG &cfg) {
	ListDigraph::Node u,v,head;

	head = u = INVALID;
	FunctionCall call("main", 0x200);
	for (int i=0; i<10; i++) {
		v = cfg.addNode();
		cfg.setAddr(v, 0x400 + i * 4);
		cfg.setFunction(v, call);

		if (head == INVALID) {
			head = v;
			cfg.markHead(head, true);
			cfg.setIters(head, 20);
		} else {
			cfg.setHead(v, head);
		}

		if (u != INVALID) {
			cfg.addArc(u, v);
		} else {
			cfg.setInitial(v);
		}
		u = v;
	}
}

static void
init_cache(Cache &cache) {
	
}

bool
CFRFactory::test() {
	CFG cfg;
	init_cfg(cfg);

	Cache cache(4, 1, 8, 10, new PolicyLRU());

	DOTFactory dot(cfg);
	dot.setCache(&cache);
	dot.setPath("CFRFactory_test_cfg.dot");
	dot.produce();

	JPGFactory jpg(dot);
	jpg.produce();
	return true;
	
	CFRFactory cfr_fact(cfg, cache);
	map<ListDigraph::Node, CFR*> cfrs = cfr_fact.produce();
	map<ListDigraph::Node, CFR*>::iterator cfr_it;

	for (cfr_it = cfrs.begin(); cfr_it != cfrs.end(); ++cfr_it) {
		CFR *cfr = cfr_it->second;
		ListDigraph::Node initial = cfr->getInitial();
		DOTfromCFR cfr_dot(*cfr);
		cfr_dot.setPath("cfr-" + cfr->stringAddr(initial) + ".dot");
		cfr_dot.setCache(&cache);
		cfr_dot.produce();

		JPGFactory jpg(cfr_dot);
		jpg.produce();
	}

	return true;
}

