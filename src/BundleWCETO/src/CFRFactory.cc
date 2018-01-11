#include "CFRFactory.h"

void
NodeCFRMap::replace(ListDigraph::Node node, CFR *cfr) {
	NodeCFRMap::iterator it = find(node);
	if (it != end()) {
		erase(it);
	}
	insert(pair<ListDigraph::Node, CFR*>(node, cfr));
}

CFRFactory::~CFRFactory() {
	delete cfrg;
	map<ListDigraph::Node, CFR*>::iterator mit;
	for (mit = _cfrs.begin(); mit != _cfrs.end(); ++mit) {
		delete mit->second;
	}
	_cfrs.clear();
	xlog.close();
	bcfr.close();
	prdc.close();
	
}

#define dout dbg.buf << dbg.start
map<ListDigraph::Node, CFR*>
CFRFactory::produce() {
	dbg.inc("CFRFactory::produce: ");

	visitClear();
	markLoops();

	ListDigraph::Node cursor, initial = _cfg.getInitial();
	dout << "Initial node: " << _cfg.stringNode(initial) << endl;
	dbg.flush(xlog);

	/* First pass, assign instructions to CFRs */
	NodeList next_cfrs;
	next_cfrs.push_back(initial);
	dbg.inc("CFRFactory::produce loop: ");
	do {
		visitClear();
		cursor = next_cfrs.front(); next_cfrs.pop_front();
		dout << "Working on node: " << _cfg.stringNode(cursor) << endl;
		if (_cfg_to_cfr.find(cursor) != _cfg_to_cfr.end()) {
			dout << "Already handled, next" << endl;
			continue;
		}
		dbg.flush(xlog);
		/* This node starts a CFR, but it may be a loop head */
		CFR *cfr;
		if (newCFRTest(cursor)) {
			cfr = addCFR(cursor);
			dout << "Created a new CFR " << *cfr << endl;
		} else {
			cfr = getCFR(cursor);
			dout << "Using existing CFR " << *cfr << endl;
		}
		_initial[cursor] = true;
		ListDigraph::Node cfrg_node = cfrg->addNode(cfr);

		Cache copy(_cache);
		NodeList xflicts = assignToConflicts(cfr, cursor, copy);
		for (ListDigraph::Node &node : xflicts) {
			/* Critical! */
			_initial[node] = true;
			if (_cfg_to_cfr.find(node) != _cfg_to_cfr.end()) {
				continue;
			}
			next_cfrs.push_back(node);
		}
		dout << "Done with node: " << _cfg.stringNode(cursor) << endl;
		dbg.flush(xlog);
	} while (!next_cfrs.empty());
	dbg.dec();

	CFR *cfr = getCFR(initial);
	dout << "Initial CFR: " << *cfr << endl;
	cfrg->setInitialCFR(cfr);

	/*
	 * Second pass.
	 * CFR's have been created and contain only their first
	 * instruction. Now add the remaining instructions to the
	 * CFRs.
	 *
	 * However, CFRs may be "broken" during the uniqueness
	 * pass. These broken CFRs will create additional ones that
	 * also need to be filled.
	 * 
	 * This second pass finalizes the broken CFRs while placing
	 * instructions within them.
	 */
	CFRList clist;
	clist.push_front(cfr);
	map<CFR*, bool> cfr_visited;
	dbg.inc("CFRF-pass 2: ");
	dout << "beginning." << endl;
	dbg.flush(prdc);
	do {
		cfr = clist.front(); clist.pop_front();
		dout << "Working CFR: " << *cfr << endl;
		dbg.flush(prdc);
		if (cfr_visited[cfr]) {
			dout << "Already worked, continuing" << endl;
			continue;
		}
		dout << "Not worked" << endl;		
		cfr_visited[cfr] = true;
		CFRList nexts = buildCFR(cfr);
		clist.insert(clist.end(), nexts.begin(), nexts.end());
		dbg.flush(prdc);
	} while (!clist.empty());
	dbg.dec();
	dout << "finished." << endl;	

	/**
	 * Third pass.
	 *
	 * Links CFRs together to make the final CFRG
	 */
	cfr = getCFR(initial);
	clist.push_front(cfr);
	cfr_visited.clear();
	dbg.inc("CFRF-pass 3: ");
	dout << "beginning." << endl;
	do {
		cfr = clist.front(); clist.pop_front();
		if (cfr_visited[cfr]) {
			dout << *cfr << "Already worked, continuing" << endl;
			continue;
		}
		dout << "Not worked" << endl;		
		cfr_visited[cfr] = true;
		
		dout << "+arcs for " << *cfr << endl;
		dbg.inc("edges 3: ");
		dbg.flush(prdc);
		CFRList nexts = CFRsuccs(cfr);
		for (CFR* &kid_cfr : nexts) {
			dout << *cfr << " → " << *kid_cfr << endl;
			ensureArc(cfr, kid_cfr);
		}
		clist.insert(clist.end(), nexts.begin(), nexts.end());		
		dbg.flush(prdc);
		dbg.dec();
	} while (!clist.empty());
	dout << "finished." << endl;
	dbg.flush(prdc); dbg.dec();
	return _cfrs;
}
#undef dout

/**
 * An assistance function to produce, determines if a new CFR should
 * be created for this node.
 *
 * @param[in] node the node being tested
 *
 * @return true if a new CFR should be created, false if the node
 * belongs to a proper CFR (or starts one)
 */
bool
CFRFactory::newCFRTest(ListDigraph::Node node) {
	NodeCFRMap::iterator it = _cfg_to_cfr.find(node);
	if (it == _cfg_to_cfr.end()) {
		/* Has no CFR, definitely needs a new one */
		return true;
	}
	CFR *cfr = _cfg_to_cfr[node];
	ListDigraph::Node cfr_node = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_node);
	if (node == cfg_node) {
		/* It has a CFR, which is itself, no need to create a
		   new one. */
		return false;
	}
	/* This node belongs to a CFR, but will become a new CFR */
	return true;
}


void
CFRFactory::markLoops() {
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		if (!_cfg.isHead(node)) {
			continue;
		}
		/* Have a loop head */
		_initial[node] = true;
		markLoopExits(node);
	}
}

void
CFRFactory::markLoopExits(ListDigraph::Node node) {
	ListDigraph::Node head = _cfg.getHead(node);
	if (_cfg.isHead(node)) {
		head = node;
	}
	if (head == INVALID) {
		/* Not part of a loop */
		return;
	}
	visit(node, true);

	for (ListDigraph::OutArcIt a(_cfg, node); a != INVALID; ++a) {
		ListDigraph::Node succ = _cfg.runningNode(a);
		ListDigraph::Node succ_head = _cfg.getHead(succ);

		if (_cfg.isHead(succ)) {
			/* Will have been marked initial by markLoops() */
			continue;
		}
		if (head != succ_head) {
			/* succ is not in node's loop */
			_initial[succ] = true;
			continue;
		}
		if (visited(succ)) {
			continue;
		}
 		/* Recurse */
		markLoopExits(succ);
	}
}


/**
 * Assigns instructions from the CFG to CFRs, however the instructions are not
 * added to the CFR. Only their assignment is recorded in _cfg_to_cfr
 */
#define dout dbg.buf << dbg.start
NodeList
CFRFactory::assignToConflicts(CFR *cfr, ListDigraph::Node cursor,
    Cache &cache) {
	dout << _cfg.stringNode(cursor) << " assignments" << endl;
	dbg.inc("+toX: ");
	NodeList xflicts;
	NodeList nexts;

	nexts.push_back(cursor);
	while (!nexts.empty()) {
		ListDigraph::Node cur = nexts.front(); nexts.pop_front();
		visit(cur);
		if (conflicts(cur, cache)) {
			dout << _cfg.stringNode(cur)
			     << " xflicts, preening" << endl;
			preen(cur);
			/* Since preening occured at cur, create a CFR
			   if needed */
			if (newCFRTest(cur)) {
				NodeCFRMap::iterator it = _cfg_to_cfr.find(cur);
				if (it != _cfg_to_cfr.end()) {
					_cfg_to_cfr.erase(it);
				}
				it = _cfrs.find(cur);
				if (it != _cfrs.end()) {
					_cfrs.erase(it);
				}
			}
			xflicts.push_back(cur);
			continue;
		}
		cache.insert(_cfg.getAddr(cur));
		for (ListDigraph::OutArcIt a(_cfg, cur); a != INVALID; ++a) {
			ListDigraph::Node succ = _cfg.runningNode(a);
			string s = _cfg.stringNode(succ);
			if (visited(succ)) {
				dout << s << " already visited, skipping."
				     << endl;
				continue;
			}
			if (_initial[succ]) {
				dout << s << " CFR starter, added to xflicts"
				     << endl;
				xflicts.push_back(succ);
				continue;
			}
			if (!_cfg.sameLoop(succ, cursor)) {
				dout << s << " outside of this loop, xflict."
				     << endl;
				xflicts.push_back(succ);
				continue;
			}
			dout << s << " added" << endl;
			_cfg_to_cfr.replace(succ, cfr);
			nexts.push_back(succ);
		}
	}
	dbg.dec();
	dout << _cfg.stringNode(cursor) << " assigned" << endl;	
	return xflicts;
}
#undef dout


class PreenData {
public:
	PreenData(ListDigraph::Node s, CFR *scfr, NodeCFRMap &cc,
		  ListDigraph::NodeMap<bool> &p) :
		cfgtocfr(cc), protect(p) {
		cfr = scfr;
		starter = s;
	}
	CFR *cfr;
	NodeCFRMap &cfgtocfr;
	ListDigraph::Node starter;
	ListDigraph::Node cfr_term;
	ListDigraph::NodeMap<bool> &protect;
};

static bool
preen_protect_mask(CFG &cfg, ListDigraph::Node node, void *userdata) {
	PreenData *data = (PreenData *) userdata;
	NodeCFRMap::iterator it = data->cfgtocfr.find(node);
	if (it == data->cfgtocfr.end()) {
		/* Not yet assigned a CFR */
		return false;
	}
	if (it->second != data->cfr) {
		/* Not assigned to this CFR */
		return false;
	}
	/* Assigned to this CFR */
	return true;
}
static bool
preen_protect_work(CFG &cfg, ListDigraph::Node node, void *userdata) {
	PreenData *data = (PreenData *) userdata;
	/*
	 * If this node is being worked, two things are known:
	 * 1.) It's in the CFR of interest
	 * 2.) It is *before* the starter.
	 *
	 * Clearly it needs protection from removal
	 */
	data->protect[node] = true;
	return true;
}

static bool
preen_clear_mask(CFG &cfg, ListDigraph::Node node, void *userdata) {
	PreenData *data = (PreenData *) userdata;
	NodeCFRMap::iterator it = data->cfgtocfr.find(node);
	if (it == data->cfgtocfr.end()) {
		/* Not yet assigned a CFR */
		return false;
	}
	if (it->second != data->cfr) {
		/* Not assigned to this CFR */
		return false;
	}
	return true;
}
static bool
preen_clear_work(CFG &cfg, ListDigraph::Node node, void *userdata) {
	PreenData *data = (PreenData *) userdata;
	/*
	 * If this node is being worked, we know
	 * 1.) It's in the CFR of interest
	 * 2.) It is *not* protected
	 * 3.) It follows the starter
	 *
	 * Clearly it needs to be removed
	 */
	NodeCFRMap::iterator it = data->cfgtocfr.find(node);
	data->cfgtocfr.erase(it);
	
	return true;
}

static bool
preen_clear_sel(CFG &cfg, ListDigraph::Node node, void *userdata) {
	PreenData *data = (PreenData *) userdata;
	NodeCFRMap::iterator it = data->cfgtocfr.find(node);
	if (it == data->cfgtocfr.end()) {
		/* No CFR, no need to clear */
		return true;
	}
	CFR *cfr = it->second;
	if (cfr != data->cfr)  {
		/* Not in *this* CFR, no need to clear */
		return false;
	}
	/* In *this* CFR */
	if (data->protect[node]) {
		/* Protected, cannot clear */
		return true;
	}
	return false;
}

void
CFRFactory::preen(ListDigraph::Node starter) {
	NodeCFRMap::iterator it = _cfg_to_cfr.find(starter);
	if (it == _cfg_to_cfr.end()) {
		return;
	}
	CFR *scfr = it->second;
	ListDigraph::Node cfgi = scfr->toCFG(scfr->getInitial());
	ListDigraph::NodeMap<bool> protect(_cfg);

	/*
	 * Step 1, create a topological sort of the (entire) CFR
	 */
	PreenData pdata(starter, scfr, _cfg_to_cfr, protect);
	CFGTopSort topo(_cfg, &pdata, preen_protect_mask);
	topo.sort(cfgi);

	pqueue_t::iterator pit;
	for (pit = topo.result.begin(); pit != topo.result.end(); ++pit) {
		ListDigraph::Node cfg_node = *pit;
		cout << "  " << _cfg.stringNode(cfg_node) << endl;
	}
}

#define dout dbg.buf << dbg.start
CFRList
CFRFactory::buildCFR(CFR *cfr) {
	dbg.inc("BCFR: ");
	CFRList cfr_nexts;

	ListDigraph::Node cfr_node = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_node);

	NodeList cfg_nodes;
	cfg_nodes.push_front(cfg_node);
	visitClear();
	dout << "Building " << *cfr << endl;
	dbg.inc("B: ");
	do {
		cfg_node = cfg_nodes.front(); cfg_nodes.pop_front();
		if (visited(cfg_node)) {
			continue;
		}
		visit(cfg_node);
		for (ListDigraph::OutArcIt ait(_cfg, cfg_node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node cfg_kid = _cfg.runningNode(ait);
			CFR *kid_cfr = _cfg_to_cfr[cfg_kid];
			string s = _cfg.stringNode(cfg_kid);
			if (kid_cfr == NULL) {
				dout << "succ: " << s << " no CFR" << endl;
				dbg.flush(bcfr);
				throw runtime_error("Every node should have a CFR");
			}
			if (kid_cfr != cfr) {
				dout << s << " ∈ " << *kid_cfr 
				     << " adding to nexts" << endl;
				/* Belongs to a different CFR */
				cfr_nexts.push_back(kid_cfr);
				continue;
			}
			/* In this CFR */
			ListDigraph::Node cfr_kid = cfr->addNode(cfg_kid);
			cfr_node = cfr->find(_cfg.getAddr(cfg_node),
					     _cfg.getFunction(cfg_node));
			dout << s << " → " << cfr->stringNode(cfr_kid) << endl;
			cfr->addArc(cfr_node, cfr_kid);
			cfg_nodes.push_back(cfg_kid);
		}
	} while (!cfg_nodes.empty());
	dbg.dec();

	dout << *cfr << " returning " << cfr_nexts.size() << " next cfrs" << endl;
	dbg.dec();
	dbg.flush(bcfr);
	return cfr_nexts;
	#undef dout
}

#define dout dbg.buf << dbg.start
CFRList
CFRFactory::CFRsuccs(CFR *cfr) {
	dbg.inc("Succs: ");
	CFRList cfr_nexts;

	ListDigraph::Node cfr_node = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_node);

	NodeList cfg_nodes;
	cfg_nodes.push_front(cfg_node);
	visitClear();
	dout << "For " << *cfr << endl;
	dbg.inc("S: ");
	do {
		cfg_node = cfg_nodes.front(); cfg_nodes.pop_front();
		if (visited(cfg_node)) {
			continue;
		}
		visit(cfg_node);
		for (ListDigraph::OutArcIt ait(_cfg, cfg_node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node cfg_kid = _cfg.runningNode(ait);
			CFR *kid_cfr = _cfg_to_cfr[cfg_kid];
			string s = _cfg.stringNode(cfg_kid);
			if (kid_cfr == NULL) {
				dout << "succ: " << s << " no CFR" << endl;
				dbg.flush(bcfr);
				throw runtime_error("Every node should have a CFR");
			}
			if (kid_cfr != cfr) {
				dout << s << " ∈ " << *kid_cfr 
				     << " adding to nexts" << endl;
				/* Belongs to a different CFR */
				cfr_nexts.push_back(kid_cfr);
				continue;
			}
			/* In this CFR */
			cfg_nodes.push_back(cfg_kid);
		}
	} while (!cfg_nodes.empty());
	dbg.dec();

	dout << *cfr << " returning " << cfr_nexts.size() << " next cfrs" << endl;
	dbg.dec();
	dbg.flush(bcfr);
	return cfr_nexts;
}
#undef dout

void
CFRFactory::ensureArc(CFR* source, CFR* target) {
	#define dout dbg.buf << dbg.start
	dbg.inc("ensureArc: ");
	ListDigraph::Node cfrgs, cfrgt;
	cfrgs = cfrg->findNode(source);
	cfrgt = cfrg->findNode(target);
	ListDigraph::Arc a=INVALID;

	a = findArc(*cfrg, cfrgs, cfrgt);
	if (a != INVALID) {
		dbg.dec();
		return;
	}

	dout << "+arc: " << *source << " --> " << *target << endl; 
	cfrg->addArc(cfrgs, cfrgt);
	dbg.dec();
	#undef dout
}


void
CFRFactory::visit(ListDigraph::Node node, bool yes) {
	_visited[node] = yes;
}
bool
CFRFactory::visited(ListDigraph::Node node) {
	return _visited[node];
}
void
CFRFactory::visitClear() {
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		visit(node, false);
	}
}


#define PARANOIA
CFR*
CFRFactory::addCFR(ListDigraph::Node cfg_node) {
	CFR *new_cfr = getCFR(cfg_node);
	if (new_cfr != NULL) {
		return new_cfr;
	}
	new_cfr = new CFR(_cfg);
	ListDigraph::Node initial = new_cfr->addNode(cfg_node);
	new_cfr->setInitial(initial);
	new_cfr->setCache(&_cache);

	#ifdef PARANOIA
	NodeCFRMap::iterator it = _cfg_to_cfr.find(cfg_node);
	if (it != _cfg_to_cfr.end()) {
		throw runtime_error("Existing CFR present");
	}
	#endif

	_cfrs.insert(pair<ListDigraph::Node, CFR*>(cfg_node, new_cfr));
	_cfg_to_cfr.insert(pair<ListDigraph::Node, CFR*>(cfg_node, new_cfr));

	#ifdef PARANOIA
	it = _cfrs.find(cfg_node);
	if (it->second != new_cfr) {
		throw runtime_error("Mismatched CFR");
	}

	it = _cfg_to_cfr.find(cfg_node);
	if (it->second != new_cfr) {
		throw runtime_error("Mismatched CFG -> CFR");
	}
	#endif
	return new_cfr;
}

CFR*
CFRFactory::getCFR(ListDigraph::Node cfg_node) {
	NodeCFRMap::iterator cfrit = _cfrs.find(cfg_node);
	if (cfrit != _cfrs.end()) {
		return cfrit->second;
	}
	return NULL;
}

bool
CFRFactory::conflicts(ListDigraph::Node node, Cache &cache) {
	unsigned long addr = _cfg.getAddr(node);
	CacheSet *cs = cache.setOf(addr);

	if (cs->present(addr)) {
		/* Definitely not an eviction, could have been cached by an
		   earlier load of this block */
		return false;
	}
	if (cs->evicts(addr)) {
		/* Definitely an eviction */
		return true;
	}
	if (!cs->empty()) {
		/* It would be tempting to say that because this cache set is
		 * not full there is room for the value.
		 * However, there may be multiple paths of unknown length
		 * leading to this instruction. If there is any value in the
		 * set, then it could be a conflict.
		 */
		return true;
	}

	return false;
}
