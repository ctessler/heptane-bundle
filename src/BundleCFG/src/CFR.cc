#include "CFR.h"

ListDigraph::Node
CFR::addNode(ListDigraph::Node from_cfg) {
	ListDigraph::Node rv = CFG::addNode();

	ListDigraph::Node cfg_head = _cfg.getHead(from_cfg);
	FunctionCall call = _cfg.getFunction(from_cfg);

	setAddr(rv, _cfg.getAddr(from_cfg));
	markHead(rv, _cfg.isHead(from_cfg));
	setIters(rv, _cfg.getIters(from_cfg));
	setHead(rv, cfg_head);
	setFunction(rv, call);
	_to_cfg[rv] = from_cfg;

	return rv;
}

ListDigraph::Node
CFR::toCFG(ListDigraph::Node node) {
	if (valid(node)) {
		return _to_cfg[node];
	}
	return INVALID;
}

void
CFR::setInitial(ListDigraph::Node cfr_initial, ListDigraph::Node cfg_initial) {
	_membership = cfg_initial;
	CFG::setInitial(cfr_initial);
}

string
CFR::stringNode(ListDigraph::Node node) const {
	if (node == INVALID) {
		return "INVALID";
	}
	stringstream ss;
	ss << stringAddr(node);
	if (isHead(node)) {
		ss << "[" << getIters(node) << "]";
	}
	ss << "(" << getFunction(node) << ", ";

	ListDigraph::Node head = getHead(node);
	ss << "head:" << _cfg.stringAddr(head)
	   << ")";
	return ss.str();
}

std::ostream&
operator<< (std::ostream &stream, const CFR& cfr) {
	ListDigraph::Node initial = cfr.getInitial();
	stream << "("
	       << countNodes(cfr) << "v, "
	       << countArcs(cfr) << "e, "
	       << (initial == INVALID ?
		   "INVALID" : cfr.stringNode(initial))
	       << ")";

	return stream;
}
