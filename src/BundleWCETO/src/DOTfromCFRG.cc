void
DOTfromCFRG::produce(unsigned int threads) {
	ofstream dot(_path.c_str());

	dot << "digraph G {" << endl;

	for (ListDigraph::NodeIt nit(_cfrg); nit != INVALID; ++nit) {
		ListDigraph::Node cfr_node = nit;
		CFR *cfr = _cfrg.findCFR(cfr_node);
		dot << CFRDOT(*cfr, threads);
	}

	for (ListDigraph::ArcIt ait(_cfrg); ait != INVALID; ++ait) {
		ListDigraph::Node cfrn_source = _cfrg.source(ait);
		ListDigraph::Node cfrn_dest = _cfrg.target(ait);

		CFR *cfr_source = _cfrg.findCFR(cfrn_source);
		CFR *cfr_dest = _cfrg.findCFR(cfrn_dest);

		dot << CFRDOTid(*cfr_source) << " -> " << CFRDOTid(*cfr_dest) << endl;
	}
	
	dot << "} // end digraph G" << endl;
}

