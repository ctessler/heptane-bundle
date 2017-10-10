#include "DOTfromCFRG.h"

static string
CFRDOTid(CFR &cfr) {
	CFG *cfg = cfr.getCFG();
	ListDigraph::Node cfr_initial = cfr.getInitial();
	ListDigraph::Node cfg_node = cfr.toCFG(cfr_initial);


	stringstream id;
	FunctionCall call = cfg->getFunction(cfg_node);
	id << call.getName() << "_" << call.getCallSite() << "_" << cfg->stringAddr(cfg_node);

	return id.str();
}

static string
CFRDOT(CFR &cfr, unsigned int threads) {
	CFG *cfg = cfr.getCFG();
	ListDigraph::Node cfr_initial = cfr.getInitial();
	ListDigraph::Node cfg_node = cfr.toCFG(cfr_initial);

	stringstream label, node;
	label << "<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">" << endl << "\t"
	      << "<TR><TD COLSPAN=\"2\">CFR</TD></TR>" << endl
	      << "<TR><TD>" << cfg->stringAddr(cfg_node) << "</TD>"
	      << "<TD>" << cfg->getFunction(cfg_node) << "</TD></TR>"
	      << "<TR><TD> Threads:" << threads << "</TD>"
	      << "<TD>WCET+O: " << cfr.wcet(threads) << "</TD></TR>"
	      << "<TR><TD>isHead: " << cfr.isHead(cfr_initial) << "</TD>"
	      << "<TD>Head: " << cfg->stringNode(cfr.getHead(cfr_initial)) << "</TD></TR>"
	      << "</TABLE>>";
				    
	node << CFRDOTid(cfr) << "[shape=plaintext]" << endl
	     << CFRDOTid(cfr) << "[ label=" << label.str() << " ];" << endl;

	return node.str();
}

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

