#include "CFR.h"

ListDigraph::Node
CFR::addNode(ListDigraph::Node from_cfg) {
	ListDigraph::Node rv = CFG::addNode();

	_to_cfg[rv] = from_cfg;
	_from_cfg.insert(make_pair(from_cfg, rv));
	
	return rv;
}

ListDigraph::Node
CFR::toCFG(ListDigraph::Node node) const {
	if (!valid(node)) {
		throw runtime_error("CFR::toCFG invalid CFR node");
	}
	return _to_cfg[node];
}

ListDigraph::Node
CFR::fromCFG(ListDigraph::Node cfg_node) {
	if (cfg_node == INVALID) {
		return INVALID;
	}
	map<ListDigraph::Node, ListDigraph::Node>::iterator mit;
	mit = _from_cfg.find(cfg_node);
	if (mit != _from_cfg.end()) {
		return mit->second;
	}
	return INVALID;
}

void
CFR::setInitial(ListDigraph::Node cfr_initial) {
	ListDigraph::Node cfg_initial = toCFG(cfr_initial);
	if (cfr_initial == INVALID) {
		throw runtime_error("CFR::setInitial invalid initial node in the CFG");
	}
	_membership = cfg_initial;
	CFG::setInitial(cfr_initial);
}

/* Gets and sets the function associated with the instruction */
FunctionCall
CFR::getFunction(ListDigraph::Node node) const {
	ListDigraph::Node cfg_node = toCFG(node);
	return _cfg.getFunction(cfg_node);
}

iaddr_t
CFR::getAddr(ListDigraph::Node node) const {
	ListDigraph::Node cfg_node = toCFG(node);
	return _cfg.getAddr(cfg_node);
}

string
CFR::stringAddr(ListDigraph::Node node) const {
	ListDigraph::Node cfg_node = toCFG(node);
	return _cfg.stringAddr(cfg_node);
}

string
CFR::stringNode(ListDigraph::Node node) const {
	ListDigraph::Node cfg_node = toCFG(node);
	return _cfg.stringNode(cfg_node);
}

ListDigraph::Node
CFR::find(iaddr_t addr, FunctionCall const &fcall) {
	ListDigraph::Node cfg_node = _cfg.find(addr, fcall);
	if (cfg_node == INVALID) {
		return INVALID;
	}
	ListDigraph::Node cfr_node = fromCFG(cfg_node);
	return cfr_node;
}

list<ListDigraph::Node>
CFR::find(iaddr_t addr) {
	list<ListDigraph::Node> cfg_nodes = _cfg.find(addr);
	list<ListDigraph::Node> cfr_nodes;
	for (list<ListDigraph::Node>::iterator lit = cfg_nodes.begin();
	     lit != cfg_nodes.end(); ++lit) {
		ListDigraph::Node cfr_node = fromCFG(*lit);
		cfr_nodes.push_back(cfr_node);
	}
	return cfr_nodes;
}

ListDigraph::Node
CFR::getHead(ListDigraph::Node node) const {
	ListDigraph::Node cfg_node = toCFG(node);
	ListDigraph::Node cfg_head = _cfg.getHead(cfg_node);
	return cfg_head;

	#if 0
	map<ListDigraph::Node, ListDigraph::Node>::const_iterator mit;
	mit = _cfg_head.find(node);
	if (mit != _cfg_head.end()) {
		return mit->second;
	}
	return INVALID;
	#endif
}

void
CFR::setHead(ListDigraph::Node cfr_node, ListDigraph::Node cfg_node) {
	_cfg_head.insert(make_pair(cfr_node, cfg_node));
}

bool
CFR::isHead(ListDigraph::Node node) const {
	ListDigraph::Node cfg_node = toCFG(node);
	return _cfg.isHead(cfg_node);
}

unsigned int
CFR::getIters(ListDigraph::Node head) const {
	ListDigraph::Node cfg_node = toCFG(head);
	return _cfg.getIters(cfg_node);
}



std::ostream&
operator<< (std::ostream &stream, const CFR& cfr) {
	ListDigraph::Node initial = cfr.getInitial();
	ListDigraph::Node cfg_initial = cfr.toCFG(initial);

	stream << "("
	       << countNodes(cfr) << "v, "
	       << countArcs(cfr) << "e, "
	       << (initial == INVALID ?
		   "INVALID" : cfr.getCFG()->stringNode(cfg_initial))
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

