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
	cout << "CFRFactory::destructor" << endl;
	delete cfrg;
	map<ListDigraph::Node, CFR*>::iterator mit;
	for (mit = _cfrs.begin(); mit != _cfrs.end(); ++mit) {
		delete mit->second;
	}
	_cfrs.clear();
}


map<ListDigraph::Node, CFR*>
CFRFactory::produce() {
	dbg.inc("CFRFactory::produce: ");
	#define dout dbg.buf << dbg.start

	visitClear();
	markLoops();

	ListDigraph::Node cursor, initial = _cfg.getInitial();
	dout << "Initial node: " << _cfg.stringNode(initial) << endl;

	/* First pass, assign instructions to CFRs */
	NodeList next_cfrs;
	next_cfrs.push_back(initial);
	do {
		dbg.inc("CFRFactory::produce loop: ");
		visitClear();
		cursor = next_cfrs.front(); next_cfrs.pop_front();
		dout << "Working on node: " << _cfg.stringNode(cursor) << endl;
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
			if (_cfg_to_cfr.find(node) != _cfg_to_cfr.end()) {
				continue;
			}
			next_cfrs.push_back(node);
		}
		
		dout << "Done with node: " << _cfg.stringNode(cursor) << endl;
		dbg.dec();
	} while (!next_cfrs.empty());

	CFR *cfr = getCFR(initial);
	dout << "Initial CFR: " << *cfr << endl;
	cfrg->setInitialCFR(cfr);

	/* Second pass, create CFR objects place instructions in CFRs and link
	   CFRs in the CFRG */
	CFRList clist;
	clist.push_front(cfr);
	map<CFR*, bool> cfr_visited;
	do {
		dbg.inc("CFRFactory::produce pass 2: ");
		cfr = clist.front(); clist.pop_front();
		if (cfr_visited[cfr]) {
			continue;
		}
		cfr_visited[cfr] = true;
		dout << "Building CFR: " << *cfr << endl;
		CFRList nexts = buildCFR(cfr);
		for (CFR* &kid_cfr : nexts) {
			ensureArc(cfr, kid_cfr);
		}
		clist.insert(clist.end(), nexts.begin(), nexts.end());
		dbg.dec();
	} while (!clist.empty());


	dbg.flush(cout); dbg.dec();
	return _cfrs;
	#undef dout
}

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
	if (_cfg_to_cfr[node] == NULL) {
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

map<ListDigraph::Node, CFR*>
CFRFactory::old_produce() {
	_debug.str("");
	string prefix = _indent + "CFRFactory::produce ";
	string indent_save = _indent; _indent += " ";
	visitClear();
	markLoops();

	ListDigraph::NodeMap<bool> starter(_cfg);
	list<ListDigraph::Node> next_cfrs;
	
	ListDigraph::Node initial = _cfg.getInitial();
	if (initial == INVALID) {
		return _cfrs;
	}
	next_cfrs.push_front(initial);
	do {
		visitClear();
		ListDigraph::Node cursor = next_cfrs.front();
		next_cfrs.pop_front();
		_debug << prefix << _cfg.stringNode(cursor) << endl;

		CFR *cfr = addCFR(cursor);
		_debug << prefix << _cfg.stringNode(cursor) << " has CFR "
		       << cfr << endl; 
		_initial[cursor] = true;
		starter[cursor] = true;
		ListDigraph::Node cfr_node = cfrg->addNode(cfr);
		_debug << prefix << _cfg.stringNode(cursor) << " has node "
		       << cfrg->id(cfr_node) << endl;
		
		Cache copy(_cache);
		list<ListDigraph::Node> xflicts =
			addToConflicts(cursor, cursor, copy);
		for (list<ListDigraph::Node>::iterator lit = xflicts.begin();
		     lit != xflicts.end(); lit++) {
			CFR *next_cfr = addCFR(*lit);
			
			ListDigraph::Node next_cfr_node = cfrg->addNode(next_cfr);
			if (cfr_node != next_cfr_node) {
				cfrg->addArc(cfr_node, next_cfr_node);
			}
			if (starter[*lit]) {
				/* Already handled */
				continue;
			}
			next_cfrs.push_back(*lit);
			starter[*lit] = true;
		}
	} while (!next_cfrs.empty());
	CFR *cfr = getCFR(initial);
	cfrg->setInitialCFR(cfr);

	cout << _debug.str();
	_indent = indent_save;
	return _cfrs;
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
		//addCFR(node);
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
			//addCFR(succ);
			continue;
		}
		if (visited(succ)) {
			continue;
		}
 		/* Recurse */
		markLoopExits(succ);
	}
}

list<ListDigraph::Node>
CFRFactory::addToConflicts(ListDigraph::Node cfr_initial,
    ListDigraph::Node node, Cache &cache) {
	list<ListDigraph::Node> xflicts;
	string pre = _indent + "addToConflicts ";
	string indent_save = _indent; _indent += " ";
	_debug << pre << _cfg.stringNode(node) << endl;

	if (conflicts(node, cache)) {
		_debug << pre << _cfg.stringNode(node) << " conflicts, stopping"
		       << endl;
		/* Node is a conflict, time to stop */
		xflicts.push_back(node);
		return xflicts;
	}
	
	/* Add this node to the cache */
	cache.insert(_cfg.getAddr(node));
	CFR *cfr = addCFR(cfr_initial);
	if (!cfr) {
		throw runtime_error("No CFR");
	}
	_debug << pre << _cfg.stringNode(node) << " in CFR " << *cfr << endl;
	visit(node);

	ListDigraph::Node cfr_node = cfr->find(_cfg.getAddr(node),
					       _cfg.getFunction(node));
	if (cfr_node == INVALID) {
		throw runtime_error("Could not find node in CFR");
	}

	for (ListDigraph::OutArcIt a(_cfg, node); a != INVALID; ++a) {
		ListDigraph::Node kid = _cfg.runningNode(a);
		if (visited(kid)) {
			_debug << pre << " skipped node " << _cfg.stringNode(kid)
			       << " already visited" << endl;
			continue;
		}
		if (_initial[kid]) {
			_debug << pre << " skipped node " << _cfg.stringNode(kid)
			       << " is initial" << endl;
			/* This child begins a conflict free region */
			xflicts.push_back(kid);
			continue;
		}
		/* This child is not a conflict nor an initial instruction, it
		   belongs to this conflict free region. */
		ListDigraph::Node cfr_kid = cfr->addNode(kid);
		cfr->addArc(cfr_node, cfr_kid);
		
		_debug << pre << "added " << _cfg.stringNode(kid) << " to CFR "
		       << *cfr << endl;

		list<ListDigraph::Node> mflicts = addToConflicts(cfr_initial, kid,
								 cache);
		xflicts.insert(xflicts.end(), mflicts.begin(), mflicts.end());
	}

	for (list<ListDigraph::Node>::iterator lit = xflicts.begin();
	     lit != xflicts.end(); ++lit) {
		_debug << pre << " returning: " << _cfg.stringNode(*lit) << endl;
	}
		
	_indent = indent_save;
	return xflicts;
}


/**
 * Assigns instructions from the CFG to CFRs, however the instructions are not
 * added to the CFR. Only their assignment is recorded in _cfg_to_cfr
 *
 */
NodeList
CFRFactory::assignToConflicts(CFR *cfr, ListDigraph::Node cursor,
    Cache &cache) {
	dbg.inc("assignToConflicts: ");
	#define dout dbg.buf << dbg.start
	NodeList xflicts;

	if (conflicts(cursor, cache)) {
		dout << _cfg.stringNode(cursor) << " conflicts, stopping" << endl;
		xflicts.push_back(cursor);
		dbg.dec();
		return xflicts;
	}
	/* Add this node to the cache */
	cache.insert(_cfg.getAddr(cursor));
	visit(cursor);
	_cfg_to_cfr.replace(cursor, cfr);
	dout << _cfg.stringNode(cursor) << " assigned to CFR " << *cfr << endl;

	for (ListDigraph::OutArcIt a(_cfg, cursor); a != INVALID; ++a) {
		ListDigraph::Node kid = _cfg.runningNode(a);
		dout << _cfg.stringNode(kid);
		if (visited(kid)) {
			dbg.buf << " already visited, skipping." << endl;
			continue;
		}
		if (_initial[kid]) {
			dbg.buf << " a CFR starter, added to conflicts." << endl;
			xflicts.push_back(kid);
			continue;
		}
		if (!_cfg.sameLoop(kid, cursor)) {
			xflicts.push_back(kid);
			continue;
		}
		dbg.buf << " candidate for " << *cfr << endl;
		NodeList mflicts = assignToConflicts(cfr, kid, cache);
		xflicts.insert(xflicts.end(), mflicts.begin(), mflicts.end());
	}

	dbg.dec();
	#undef dout
	return xflicts;
}

CFRList
CFRFactory::buildCFR(CFR *cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("buildCFR: ");
	CFRList cfr_nexts;

	ListDigraph::Node cfr_node = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_node);

	NodeList cfg_nodes;
	cfg_nodes.push_front(cfg_node);
	visitClear();
	do {
		cfg_node = cfg_nodes.front(); cfg_nodes.pop_front();
		if (visited(cfg_node)) {
			continue;
		}
		visit(cfg_node);
		for (ListDigraph::OutArcIt ait(_cfg, cfg_node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node cfg_kid = _cfg.runningNode(ait);
			if (_cfg_to_cfr[cfg_kid] != cfr) {
				/* Belongs to a different CFR */
				cfr_nexts.push_back(_cfg_to_cfr[cfg_kid]);
				continue;
			}
			/* In this CFR */
			ListDigraph::Node cfr_kid = cfr->addNode(cfg_kid);
			cfr_node = cfr->find(_cfg.getAddr(cfg_node),
					     _cfg.getFunction(cfg_node));
			dout << *cfr << " adding arc "
			     << cfr->stringNode(cfr_node)
			     << " --> " << cfr->stringNode(cfr_kid) << endl;
			cfr->addArc(cfr_node, cfr_kid);
			cfg_nodes.push_back(cfg_kid);
		}
	} while (!cfg_nodes.empty());

	dbg.dec();
	return cfr_nexts;
	#undef dout
}

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
		return;
	}

	dout << "Adding CFR arc: " << *source << " --> " << *target << endl; 
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

	_cfrs.insert(make_pair(cfg_node, new_cfr));
	_cfg_to_cfr.insert(pair<ListDigraph::Node, CFR*>(cfg_node, new_cfr));

	#ifdef PARANOIA
	map<ListDigraph::Node, CFR*>::iterator it = _cfrs.find(cfg_node);
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
	map<ListDigraph::Node, CFR*>::iterator cfrit = _cfrs.find(cfg_node);
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
