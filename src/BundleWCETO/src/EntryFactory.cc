#include "EntryFactory.h"

void
EntryFactory::produce() {
	_cfrg.dupeCheck();
	
	ofstream ofile(_path.c_str());
	ofile << "cfrs = (" << endl;
	ofile << " // (CFR addr, priority, ((succ addr, prio),"
	      << " (succ addr, prio) ... )" << endl;
	ListDigraph::NodeIt nit(_cfrg);
	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		CFR *cfr = _cfrg.findCFR(node);
		/** Skip pass through CFRs */
		if (!cfr->getSwitching()) {
			continue;
		}
		/** Skip loop interior nodes */
		if (_cfrg.isLoopPartCFR(cfr)) {
			CFR *crown = _cfrg.crown(cfr);
			if (crown != cfr) {
				continue;
			}
		}

		/** Collect switching successors */
		CFRList *switches = nextSwitches(node);
		stringstream bs;
		bs << "(";
		for (CFRList::iterator cit = switches->begin() ;
		     cit != switches->end() ; ++cit) {
			CFR *succ = *cit;
			ListDigraph::Node cfrg_node = _cfrg.findNode(succ);
			if (cit != switches->begin()) {
				bs << ", ";
			}
			
			bs << "(0x" << hex << succ->getAddr(succ->getInitial())
			   << dec << ", "
			   << _cfrg.getGeneration(cfrg_node)
			   << ")";
		}
		bs << ")";
		delete switches;

		/** Output the CFR entry item */
		ListDigraph::Node initial = cfr->getInitial();
		iaddr_t addr = cfr->getAddr(initial);

		ofile << "    (0x" << hex << addr << ", "
		      << dec << _cfrg.getGeneration(node)
		      << ", " << bs.str()
		      << ")";

		/** Following comma check */
		ListDigraph::NodeIt tit(_cfrg);
		tit = nit;
		++tit;
		if (tit != INVALID) {
			ofile << ",";
		}
		ofile << endl;
	}
	ofile << ");" << endl;
	ofile.close();
}

CFRList *
EntryFactory::nextSwitches(ListDigraph::Node cfrg_node) {
	CFRList *list = new CFRList();
	CFR *cfr = _cfrg.findCFR(cfrg_node);
	stack<ListDigraph::Node> s;
	ListDigraph::NodeMap<bool> visited(_cfrg);
	s.push(cfrg_node);

	while (!s.empty()) {
		ListDigraph::Node node = s.top(); s.pop();
		visited[node] = true;
		
		ListDigraph::OutArcIt ait(_cfrg, cfrg_node);
		for ( ; ait != INVALID; ++ait) {
			ListDigraph::Node succ_node = _cfrg.target(ait);
			if (visited[succ_node]) {
				continue;
			}
			CFR *succ_cfr = _cfrg.findCFR(succ_node);
			if (succ_cfr->getSwitching()) {
				list->push_back(succ_cfr);
				visited[succ_node] = true;
				continue;
			}
			cout << *cfr << " pushing " << *succ_cfr << endl;			
			s.push(succ_node);
		}
	}
	return list;
}
