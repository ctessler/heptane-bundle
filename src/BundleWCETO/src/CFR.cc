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
	string s = "CFR::toCFG invalid CFR node ";
	if (node == INVALID) {
		throw runtime_error(s + "INVALID");
	}
	if (!valid(node)) {
		throw runtime_error(s + "bad value");
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
	stream << cfr.str();
	return stream;
}

string
CFR::str() const {
	stringstream stream;

	ListDigraph::Node initial = getInitial();
	ListDigraph::Node cfg_initial = toCFG(initial);

	stream << "("
	       << countNodes(*this) << "v, "
	       << countArcs(*this) << "e, "
	       << (initial == INVALID ?
		   "INVALID" : getCFG()->stringNode(cfg_initial))
	       << ")";
	return stream.str();
}

uint32_t
CFR::wceto(uint32_t threads) {
	#define dout dbg.buf << dbg.start
	dbg.inc("wceto: ");
	
	if (threads == 0) { return 0; }
	calcTerminal();

	uint32_t loadcost = loadCost();
	uint32_t exe = exeCost();

	uint32_t wceto = loadcost + threads * exeCost();
	dout << *this << " wcet: " << wceto << endl;
	dbg.flush(cout);
	dbg.dec();
	return wceto;
}

uint32_t
CFR::exeCost() {
	calcTerminal();
	ListDigraph::ArcMap<int> lengthMap(*this);
	for (ListDigraph::ArcIt ait(*this); ait != INVALID; ++ait) {
		ListDigraph::Arc a = ait;
		lengthMap[a] = -1;
	}
	Dijkstra<ListDigraph> dijk_loaded(*this, lengthMap);
	Dijkstra<ListDigraph>::DistMap dist(*this);
	dijk_loaded.distMap(dist);
	dijk_loaded.run(getInitial());

	int longest_path = dist[getTerminal()] * - 1 + 1;
	uint32_t exe = longest_path * _cache->latency();

	return exe;
}

/**
 * Returns the maximum load cost
 */
uint32_t
CFR::loadCost() {
	uint32_t loads = maxLoads();
	return loads * _cache->memLatency();
}

uint32_t
CFR::maxLoads() {
	return calcECBs();
}

uint32_t
CFR::calcECBs() {
	if (_ecbs.size() != 0) {
		return _ecbs.size();
	}
	_ecbs.clear();
	Cache scratch(_cache->getSets(), _cache->getWays(),
		      _cache->getLineSize(), _cache->latency(),
		      _cache->memLatency(),  _cache->getPolicy());
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		iaddr_t addr = getAddr(node);
		if (!scratch.present(addr)) {
			uint32_t index = scratch.setIndex(addr);
			_ecbs.push_back(index);
		}
		scratch.insert(addr);
	}

	_ecbs.sort();
 	return _ecbs.size();
}

ECBs*
CFR::getECBs() {
	ECBs *ecbs = new ECBs(_ecbs);
	return ecbs;
}
