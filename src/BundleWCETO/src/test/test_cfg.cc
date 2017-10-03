#include "test_cfg.h"

CFG::CFG() : ListDigraph()
{
}

CFG::CFG(CFG &other) : ListDigraph()
{
		       
}

std::ostream&
operator<< (std::ostream &stream, const CFG& cfg) {
	ListDigraph::Node initial = INVALID;
	stream << "("
	       << countNodes(cfg) << "v, "
	       << countArcs(cfg) << "e, "
	       << "INVALID" 
	       << ")";

	return stream;
}


ListDigraph::Node
CFG::addNode() {
	ListDigraph::Node rv = ListDigraph::addNode();

	return rv;
}
