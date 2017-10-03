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
	       << (initial == INVALID ?
		   "INVALID" : cfg.stringNode(initial))
	       << ")";

	return stream;
}


ListDigraph::Node
CFG::addNode() {
	ListDigraph::Node rv = ListDigraph::addNode();

	return rv;
}

string
CFG::stringNode(ListDigraph::Node node) const {
	if (node == INVALID) {
		return "INVALID";
	}
	stringstream ss;
	ss << "(";
	ss << "head:"
	   << ")";
	return ss.str();
}
