#include "CFRFactory.h"


map<ListDigraph::Node, LemonCFG*>
CFRFactory::separateCFRs(LemonCFG &cfg) {
	map<ListDigraph::Node, LemonCFG*> rval;

	ListDigraph::Node root = cfg.getRoot();

	ListDigraph::NodeIt nit(cfg, root);
	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node cfr_head = cfg.getCFR(nit);
		if (rval.find(cfr_head) != rval.end()) {
			/* The CFR has been processed, next */
			continue; 
		}
		LemonCFG *cfr = new LemonCFG(cfg);
		rval[cfr_head] = cfr;

		/* Use the CFR head from the new CFG, not the old one */
		unsigned long addr = cfg.getStartLong(cfr_head);
		cfr_head = cfr->getNode(addr);
		cfr->setRoot(cfr_head);

		map<ListDigraph::Node, bool> eraseme;
		ListDigraph::NodeIt cfr_nit(*cfr);
		for (; cfr_nit != INVALID; ++cfr_nit) {
			ListDigraph::Node cursor = cfr_nit;
			if (cfr->getCFR(cursor) != cfr_head) {
#ifdef DBG_SEPARATECFRS
				cout << "CFR [" << cfr->getStartString(cfr_head) << "] marking "
				     << cfr->getStartString(cursor) << " for removal" << endl;
#endif /* DBG_SEPARATECFRS */
				eraseme[cursor] = true;
			}
		}
		for (map<ListDigraph::Node, bool>::iterator mit = eraseme.begin();
		     mit != eraseme.end(); ++mit) {
			ListDigraph::Node rhead = mit->first;
#ifdef DBG_SEPARATECFRS
			cout << "CFR [" << cfr->getStartString(cfr_head)
			     << "] removing node: "
			     << cfr->getStartString(rhead) << endl;
#endif /* DBG_SEPARATECFRS */			
			cfr->erase(rhead);
		}
	}
	return rval;
}
