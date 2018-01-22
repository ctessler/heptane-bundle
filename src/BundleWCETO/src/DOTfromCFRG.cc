#include "DOTfromCFRG.h"

static string
CFRDOTid(CFR &cfr) {
	CFG *cfg = cfr.getCFG();
	ListDigraph::Node cfr_initial = cfr.getInitial();
	ListDigraph::Node cfg_node = cfr.toCFG(cfr_initial);


	stringstream id;
	FunctionCall call = cfg->getFunction(cfg_node);
	string fstr = DOTFactory::functionString(call);
	id << fstr << "_" << cfg->stringAddr(cfg_node);

	return id.str();
}

static string
CFRDOT(CFR &cfr, WCETOFactory &fact, int generation, unsigned int threads) {
	CFG *cfg = cfr.getCFG();
	ListDigraph::Node cfr_initial = cfr.getInitial();
	ListDigraph::Node cfg_node = cfr.toCFG(cfr_initial);
	string switching = "Yes";
	if (!cfr.getSwitching()) {
		switching = "No";
	}

	stringstream label, node;
	label << "<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">"
	      << endl << "\t"
	      << "<TR><TD COLSPAN=\"2\">CFR: " << cfg->stringNode(cfg_node)
	      << "</TD></TR>" << endl
	      << "<TR><TD>Gen. (Switch)</TD>"
	      << "<TD>" << generation << " (" << switching
	                << ")</TD></TR>" << endl
	      << "<TR><TD>Stack Top</TD>"
	      << "<TD>" << cfg->getFunction(cfg_node) << "</TD></TR>" << endl
	      << "<TR><TD>isHead (iters)</TD>"
	      << "<TD>" << cfr.isHead(cfr_initial) << "("
	                << cfr.getIters(cfr_initial) << ")" << "</TD></TR>"
	                << endl
	      << "<TR><TD>Head</TD>"
	      << "<TD>" << cfg->stringNode(cfr.getHead(cfr_initial))
	      << "</TD></TR>" << endl
	      << "<TR><TD>(Threads) WCET+O</TD><TD>" << "("
	      << fact.getThreads() << ") " << fact.value(&cfr)
	      << "</TD></TR>" << endl;
	CFRDemand *dmnd = fact.getDemand(&cfr);
	ECBs *ecbs = cfr.getECBs();
	string cfr_ecb = "()";
	if (dmnd) {
		cfr_ecb = dmnd->getECBs().str();
	}
	label << "<TR><TD>Single EXE</TD><TD>" << (dmnd ? dmnd->getEXE() :0)
	      << "</TD></TR>" << endl
	      << "<TR><TD>Load Cost</TD><TD>" << (dmnd ? dmnd->getLoad() : 0)
	      << "</TD></TR>" << endl
	      << "<TR><TD>Demand ECBs</TD><TD>" << cfr_ecb 
	      << "</TD></TR>" << endl
	      << "<TR><TD>Local ECBs</TD><TD>" << *ecbs << "</TD></TR>" << endl
	      << "</TABLE>>";
	delete ecbs;
				    
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
		dot << CFRDOT(*cfr, _fact, _cfrg.getGeneration(cfr_node),
		    threads);
	}

	for (ListDigraph::ArcIt ait(_cfrg); ait != INVALID; ++ait) {
		ListDigraph::Node cfrn_source = _cfrg.source(ait);
		ListDigraph::Node cfrn_dest = _cfrg.target(ait);

		CFR *cfr_source = _cfrg.findCFR(cfrn_source);
		CFR *cfr_dest = _cfrg.findCFR(cfrn_dest);

		dot << CFRDOTid(*cfr_source) << " -> " << CFRDOTid(*cfr_dest)
		    << endl;
	}
	
	dot << "} // end digraph G" << endl;
}

