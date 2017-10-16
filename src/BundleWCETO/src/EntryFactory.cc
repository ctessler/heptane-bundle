#include "EntryFactory.h"

void
EntryFactory::produce() {
	ofstream ofile(_path.c_str());
	ofile << hex;
	ListDigraph::NodeIt nit(_cfrg);
	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		CFR *cfr = _cfrg.findCFR(node);

		ListDigraph::Node initial = cfr->getInitial();
		iaddr_t addr = cfr->getAddr(initial);

		ofile << "0x" << addr << endl;
	}
	ofile << dec;

	ofile.close();
}
