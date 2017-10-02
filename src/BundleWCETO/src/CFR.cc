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
	_from_cfg[from_cfg] = rv;
	
	return rv;
}

ListDigraph::Node
CFR::toCFG(ListDigraph::Node node) {
	if (valid(node)) {
		return _to_cfg[node];
	}
	return INVALID;
}

ListDigraph::Node
CFR::fromCFG(ListDigraph::Node node) {
	if (valid(node)) {
		return _from_cfg[node];
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

unsigned long int
CFR::wcet(unsigned int threads) {
	int exe = 1; // MIPS constant execution time
	int brt = _cache->latency();
	if (threads == 0) {
		return 0;
	}
	
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node terminal = nit;
		if (countOutArcs(*this, terminal) > 0) {
			continue;
		}
		setTerminal(terminal);
	}

	/*
	 * Going with a simplistic version of costs:
	 *   Loaded: 1st thread loads all possible values
	 *   Unloaded: no BRT for any thread;
	 */
	ListDigraph::ArcMap<int> lengthMap(*this);
	for (ListDigraph::ArcIt ait(*this); ait != INVALID; ++ait) {
		ListDigraph::Arc a = ait;
		lengthMap[a] = -1 * brt - exe;
	}
	Dijkstra<ListDigraph> dijk_loaded(*this, lengthMap);
	Dijkstra<ListDigraph>::DistMap dist(*this);
	dijk_loaded.distMap(dist);
	dijk_loaded.run(getInitial());

	int loaded = dist[getTerminal()] * -1;
	/* That's a longest path for the 1st thread, we're using all
	   nodes for the moment */ 
	loaded = countNodes(*this) * (brt) + dist[getTerminal()] * exe;

	for (ListDigraph::ArcIt ait(*this); ait != INVALID; ++ait) {
		ListDigraph::Arc a = ait;
		lengthMap[a] = -1 * exe;
	}
	Dijkstra<ListDigraph> dijk_unloaded(*this, lengthMap);
	dijk_unloaded.distMap(dist);
	dijk_unloaded.run(getInitial(), getTerminal());
	int unloaded = dist[getTerminal()] * -1;

	unsigned long int wceto = loaded + (threads -1) * unloaded;
	return wceto;
	
}

