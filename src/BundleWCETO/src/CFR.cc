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
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node terminal = nit;
		if (countOutArcs(*this, terminal) > 0) {
			continue;
		}
		setTerminal(terminal);
	}

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
	#define dout dbg.buf << dbg.start
	dbg.inc("exeCost: ");

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
	dout << *this << " longest path length: " << longest_path;
	uint32_t exe = longest_path * _cache->latency();
	dout << *this << " execution cost (per thread): " << exe << endl;

	dbg.dec();
	return exe;
	#undef dout
}

/**
 * Returns the maximum load cost
 */
uint32_t
CFR::loadCost() {
	#define dout dbg.buf << dbg.start
	dbg.inc("loadCost: ");

	uint32_t loads = maxLoads();

	dbg.dec();
	return loads * _cache->memLatency();
	#undef dout
}

uint32_t
CFR::maxLoads() {
	#define dout dbg.buf << dbg.start
	dbg.inc("maxLoads: ");

	/**
	 * This function depends on all instructions mapping to unique
	 * cache lines. 
	 */
	Cache scratch(_cache->getSets(), _cache->getWays(),
		      _cache->getLineSize(), _cache->latency(),
		      _cache->memLatency(),  _cache->getPolicy());
	uint32_t loads=0;
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		iaddr_t addr = getAddr(node);
		if (!scratch.present(addr)) {
			loads++;
		}
		scratch.insert(addr);
	}
	dout << *this << " maximum loads: " << loads << endl;
	dbg.dec();
	#undef dbg
	return loads;
}

uint32_t
CFR::calcECBs() {
	#define dout dbg.buf << dbg.start
	dbg.inc("CFR::calcECBs: ");
	if (_ecbs.size() != 0) {
		dout << " already calculated" << endl;
		return _ecbs.size();
	}
	_ecbs.clear();
	Cache scratch(_cache->getSets(), _cache->getWays(),
		      _cache->getLineSize(), _cache->latency(),
		      _cache->memLatency(),  _cache->getPolicy());
	dout << *this << " adding ECBS:";
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		iaddr_t addr = getAddr(node);
		if (!scratch.present(addr)) {
			uint32_t index = scratch.setIndex(addr);
			dbg.buf << " " << index;
			_ecbs.push_back(index);
		}
		scratch.insert(addr);
	}
	dbg.buf << endl;

	list<uint32_t>::iterator it;
	_ecbs.sort();
	dout << *this << " ECBS:";
	for (it = _ecbs.begin(); it != _ecbs.end(); ++it) {
		dbg.buf << " " << *it;
	}
	dbg.buf << endl;
	
	dbg.dec();
	#undef dout
 	return _ecbs.size();
}

list<uint32_t>*
CFR::ECBs() {
	list<uint32_t> *rval = new list<uint32_t>(_ecbs);

	return rval;
}
