#include "EntryFactory.h"

void
EntryFactory::produce() {
	ofstream ofile(_path.c_str());
	ofile << "cfrs = (" << endl;
	ListDigraph::NodeIt nit(_cfrg);
	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		CFR *cfr = _cfrg.findCFR(node);
		if (!cfr->getSwitching()) {
			continue;
		}
		if (_cfrg.isLoopPartCFR(cfr)) {
			CFR *crown = _cfrg.crown(cfr);
			if (crown != cfr) {
				continue;
			}
		}

		ListDigraph::Node initial = cfr->getInitial();
		iaddr_t addr = cfr->getAddr(initial);

		ofile << "    (0x" << hex << addr << ", "
		      << dec << _cfrg.getGeneration(node)
		      << ")";
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
