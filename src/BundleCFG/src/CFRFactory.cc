#include "CFRFactory.h"


map<ListDigraph::Node, LemonCFG*>
CFRFactory::separateCFRs(LemonCFG &cfg) {
	map<ListDigraph::Node, LemonCFG*> rval;

	ListDigraph::Node root = cfg.getRoot();

	ListDigraph::NodeIt nit(cfg, root);
	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node cfr_head = cfg.getCFR(nit);
		if (rval.find(cfr_head) != rval.end()) {
			/* The CFR has been processed, next */
			continue; 
		}
		LemonCFG *cfr = new LemonCFG(cfg);
		rval[cfr_head] = cfr;

		/* Use the CFR head from the new CFG, not the old one */
		unsigned long addr = cfg.getStartLong(cfr_head);
		cfr_head = cfr->getNode(addr);
		cfr->setRoot(cfr_head);

		map<ListDigraph::Node, bool> eraseme;
		ListDigraph::NodeIt cfr_nit(*cfr);
		for (; cfr_nit != INVALID; ++cfr_nit) {
			ListDigraph::Node cursor = cfr_nit;
			if (cfr->getCFR(cursor) != cfr_head) {
#ifdef DBG_SEPARATECFRS
				cout << "CFR [" << cfr->getStartString(cfr_head) << "] marking "
				     << cfr->getStartString(cursor) << " for removal" << endl;
#endif /* DBG_SEPARATECFRS */
				eraseme[cursor] = true;
			}
		}
		for (map<ListDigraph::Node, bool>::iterator mit = eraseme.begin();
		     mit != eraseme.end(); ++mit) {
			ListDigraph::Node rhead = mit->first;
#ifdef DBG_SEPARATECFRS
			cout << "CFR [" << cfr->getStartString(cfr_head)
			     << "] removing node: "
			     << cfr->getStartString(rhead) << endl;
#endif /* DBG_SEPARATECFRS */			
			cfr->erase(rhead);
		}
	}
	return rval;
}

map<ListDigraph::Node, CFR*>
CFRFactory::produce() {
	_debug.str("");
	visitClear();
	markLoops();

	ListDigraph::NodeMap<bool> starter(_cfg);

	list<ListDigraph::Node> next_cfrs;
	ListDigraph::Node initial = _cfg.getInitial();
	next_cfrs.push_front(initial);
	do {
		visitClear();
		ListDigraph::Node cursor = next_cfrs.front(); next_cfrs.pop_front();
		CFR *cfr = addCFR(cursor);
		_from_cfg_to_cfr[cursor] = cfr;
		_initial[cursor] = true;
		starter[cursor] = true;
		ListDigraph::Node cfr_node = cfrg->addNode(cfr);

		Cache copy(_cache);
		list<ListDigraph::Node> xflicts = addToConflicts(cursor, copy);
		for (list<ListDigraph::Node>::iterator lit = xflicts.begin(); lit != xflicts.end(); lit++) {
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
	CFR *cfr = _from_cfg_to_cfr[initial];
	cfrg->setInitialCFR(cfr);
	
	cout << _debug.str();
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
		addCFR(node);
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
			addCFR(succ);
			continue;
		}
		if (visited(succ)) {
			continue;
		}
		/* The CFR must exist by now */
		CFR *cfr = getCFR(head);

		
 		/* Recurse */
		markLoopExits(succ);
	}
}

list<ListDigraph::Node>
CFRFactory::addToConflicts(ListDigraph::Node node, Cache &cache) {
	list<ListDigraph::Node> xflicts;
	string pre = _indent + "addToConflicts ";
	string indent_save = _indent; _indent += " ";
	_debug << pre << _cfg.stringNode(node) << endl;
	
	if (conflicts(node, cache)) {
		_debug << pre << _cfg.stringNode(node) << " conflicts, stopping" << endl;
		/* Node is a conflict, time to stop */
		xflicts.push_back(node);
		return xflicts;
	}
	/* Add this node to the cache */
	cache.insert(_cfg.getAddr(node));
	CFR *cfr = _from_cfg_to_cfr[node];
	_debug << pre << _cfg.stringNode(node) << " in CFR " << *cfr << endl;

	ListDigraph::Node cfr_node = cfr->find(_cfg.getAddr(node), _cfg.getFunction(node));
	if (cfr_node == INVALID) {
		throw runtime_error("Could not find node in CFR");
	}
	for (ListDigraph::OutArcIt a(_cfg, node); a != INVALID; ++a) {
		ListDigraph::Node kid = _cfg.runningNode(a);
		if (visited(kid)) {
			_debug << pre << " skipped node " << _cfg.stringNode(kid) << " already visited" << endl;
			continue;
		}
		if (_initial[kid]) {
			_debug << pre << " skipped node " << _cfg.stringNode(kid) << " is initial" << endl;
			/* This child begins a conflict free region */
			xflicts.push_back(kid);
			continue;
		}
		/* This child is not a conflict nor an initial instruction, it
		   belongs to this conflict free region. */
		ListDigraph::Node cfr_kid = cfr->addNode(kid);
		cfr->addArc(cfr_node, cfr_kid);
		_from_cfg_to_cfr[kid] = cfr;
		_debug << pre << "added " << _cfg.stringNode(kid) << " to CFR " << *cfr << endl;

		list<ListDigraph::Node> mflicts = addToConflicts(kid, cache);
		xflicts.insert(xflicts.end(), mflicts.begin(), mflicts.end());
	}

	for (list<ListDigraph::Node>::iterator lit = xflicts.begin(); lit != xflicts.end(); ++lit) {
		_debug << pre << " returning: " << _cfg.stringNode(*lit) << endl;
	}
		
	_indent = indent_save;
	return xflicts;
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
	new_cfr->setInitial(initial, cfg_node);

	_cfrs[cfg_node] = new_cfr;
	_from_cfg_to_cfr[cfg_node] = new_cfr;
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
