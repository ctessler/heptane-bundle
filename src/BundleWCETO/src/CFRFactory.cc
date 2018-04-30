#include "CFRFactory.h"
#define PARANOIA

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
	preenlog.close();
}

#define dout dbg.buf << dbg.start

void
CFRFactory::produce_prep() {
	dout << "Clearing initial state" << endl;
	visitClear();
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		_cfr_addr[node] = INVALID;
	}
}

void
CFRFactory::produce_assign() {
	dbg.inc("CFRF-assn: ");
	dout << "begin" << endl;
	ListDigraph::Node initial = _cfg.getInitial();
	dbg.flush(prdc);
	
	NodeList next_cfrs;
	next_cfrs.push_back(initial);
	dbg.inc("◌ ");
	do {
		ListDigraph::Node cur = next_cfrs.front(); next_cfrs.pop_front();
		string nstr = _cfg.stringNode(cur);
		dout << nstr << " begin" << endl;
		if (visited(cur)) {
			dout << nstr << " already begins a CFR, done." << endl;
			dbg.flush(prdc);
			continue;
		}
		visit(cur);
		Cache copy(_cache);
		dbg.flush(prdc);
		NodeList xflicts = labelCFR(cur, copy);
		for (ListDigraph::Node &node : xflicts) {
			next_cfrs.push_back(node);
		}
		dout << nstr << " end" << endl;
	} while(!next_cfrs.empty());
	dbg.dec();
	dout << "end" << endl;
	dbg.dec(); dbg.flush(prdc);

	#ifdef PARANOIA
	bool puke=false;
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		if (_cfr_addr[node] == INVALID) {
			puke = true;
			dout << _cfg.stringNode(node) << " no CFR" << endl;
		}
	}
	if (puke) {
		throw runtime_error("Nodes without CFRs");
	}
	#endif /* PARANOIA */
}

void
CFRFactory::produce_create() {
	dbg.inc("CFRF-create: ");
	dout << "begin" << endl;
	ListDigraph::Node initial = _cfg.getInitial();
	dbg.flush(prdc);
	visitClear();
	
	NodeList next_cfrs;
	next_cfrs.push_back(initial);
	dbg.inc("⬍ ");
	do {
		ListDigraph::Node cur = next_cfrs.front(); next_cfrs.pop_front();
		string nstr = _cfg.stringNode(cur);
		dout << nstr << " begin" << endl;
		dbg.flush(prdc);
		if (visited(cur)) {
			dout << nstr << " already visited, done." << endl;
			dbg.flush(prdc);
			continue;
		}
		visit(cur);
		NodeList nexts = expandCFR(cur);
		for (ListDigraph::Node &node : nexts) {
			string nstr = _cfg.stringNode(node);
			dout << "+ " << nstr << endl;
			next_cfrs.push_back(node);
		}
		dout << nstr << " end" << endl;
	} while(!next_cfrs.empty());
	dbg.dec();

	dout << "end" << endl;
	dbg.dec(); dbg.flush(prdc);
}

void
CFRFactory::produce_link() {
	dbg.inc("CFRF-link: ");
	dout << "begin" << endl;
	ListDigraph::Node initial = _cfg.getInitial();
	dbg.flush(prdc);
	visitClear();
	
	NodeList next_cfrs;
	next_cfrs.push_front(initial);
	dbg.inc("❊ ");
	do {
		ListDigraph::Node cur = next_cfrs.front(); next_cfrs.pop_front();
		string nstr = _cfg.stringNode(cur);
		dbg.flush(prdc);
		if (visited(cur)) {
			dout << nstr << " already visited, done." << endl;
			dbg.flush(prdc);
			continue;
		}
		visit(cur);
		ListDigraph::Node cur_cfri = _cfr_addr[cur];
		CFR *cur_cfr = _cfrs[cur_cfri];
		if (cfrg->findNode(cur_cfr) == INVALID) {
			dout << "Adding to CFRG: " << *cur_cfr << endl;
			cfrg->addNode(cur_cfr);
		}

		ListDigraph::OutArcIt ait(_cfg, cur);
		for (; ait != INVALID; ++ait) {
			ListDigraph::Node succ = _cfg.runningNode(ait);
			ListDigraph::Node cfri = _cfr_addr[succ];
			CFR *succ_cfr = _cfrs[cfri];
			if (cfrg->findNode(succ_cfr) == INVALID) {
				dout << "Adding to CFRG" << *succ_cfr << endl;
				cfrg->addNode(succ_cfr);
			}
			next_cfrs.push_back(succ);
			if (_cfr_addr[cur] == _cfr_addr[succ]) {
				/* In the same CFR continue */
				continue;
			}
			ensureArc(cur_cfr, succ_cfr);
		}
	} while(!next_cfrs.empty());
	dbg.dec();

	cfrg->setInitialCFR(_cfrs[initial]);
	
	dout << "end" << endl;
	dbg.dec(); dbg.flush(prdc);
}
	
NodeCFRMap
CFRFactory::produce() {
	dbg.inc("CFRF-prod:" );
	produce_prep();

	produce_assign();
	produce_create();
	produce_link();

	dout << "end" << endl;
	dbg.dec(); dbg.flush(prdc);

	return _cfrs;
}
#undef dout

/**
 * Labels instructions starting with the given entry point.
 */
#define dout dbg.buf << dbg.start
NodeList
CFRFactory::labelCFR(ListDigraph::Node entry, Cache &cache) {
	dbg.inc("labelCFR ✇: ");
	string estr = _cfg.stringNode(entry);
	dout << estr << " begin" << endl;
	dbg.inc("✇: ");

	/*
	 * Two cases for this CFR assignment
	 *   Case 1: It is a new CFR
	 *   Case 2: The CFR is being broken
	 */
	ListDigraph::Node marker = INVALID; /* Case 1 */
	if (_cfr_addr[entry] != INVALID) {  /* Case 2 */
		dout << estr << " breaks CFR "
		     << _cfg.stringNode(_cfr_addr[entry]) << endl;
		marker = _cfr_addr[entry];
	}

	/* This CFR may also be a loop head */
	bool loopt = false;
	if (_cfg.isHead(entry)) {
		dout << estr << " is a loop head" << endl;
		loopt = true;
	}

	NodeList xflicts, nexts;
	ListDigraph::NodeMap<bool> v(_cfg);
	nexts.push_front(entry);
	do {
		ListDigraph::Node cur = nexts.front(); nexts.pop_front();
		string cstr = _cfg.stringNode(cur);
		if (v[cur]) {
			dout << cstr << " already visited, skipping." << endl;
			continue;
		}
		v[cur] = true;
		if (loopt && !_cfg.inLoop(entry, cur)) {
			dout << cstr << " out of loop, added to xflicts" << endl;
			xflicts.push_back(cur);
			continue;
		}
		if (_cfg.isHead(cur) && cur != entry) {
			dout << cstr << " is a loop, added to xflicts" << endl;
			xflicts.push_back(cur);
			continue;
		}
		if (_cfr_addr[cur] != marker) {
			/* Not in our CFR scope */
			string cfrstr = _cfg.stringNode(_cfr_addr[cur]);
			dout << cstr << " in different CFR " << cfrstr << endl;
			dout << cstr << " added to xflicts" << endl;
			xflicts.push_back(cur);
			continue;
		}
		if (conflicts(cur, cache)) {
			dout << cstr << " conflicts, adding to xflicts." << endl;
			xflicts.push_back(cur);
			continue;
		}
		cache.insert(_cfg.getAddr(cur));
		dout << "+ " << cstr << endl;
		_cfr_addr[cur] = entry;
		for (ListDigraph::OutArcIt a(_cfg, cur); a != INVALID; ++a) {
			ListDigraph::Node kid = _cfg.runningNode(a);
			nexts.push_back(kid);
		}
	} while(!nexts.empty());

	dbg.dec();
	dout << estr << " end" << endl;
	dbg.dec(); dbg.flush(xlog);

	return xflicts;
}
#undef dout

#define dout dbg.buf << dbg.start
NodeList
CFRFactory::expandCFR(ListDigraph::Node entry) {
	dbg.inc("expandCFR ⬍: ");
	string estr = _cfg.stringNode(entry);
	dout << estr << " begin" << endl;
	dbg.inc("⬍: ");

	CFR *cfr = addCFR(entry);
	ListDigraph::Node cfr_entry = cfr->getInitial();

	NodeList next_cfr, nexts;
	ListDigraph::NodeMap<bool> v(_cfg);
	nexts.push_front(entry);
	do {
		ListDigraph::Node cur = nexts.front(); nexts.pop_front();
		ListDigraph::Node cfr_node =
		    cfr->find(_cfg.getAddr(cur), _cfg.getFunction(cur));
		string cstr = _cfg.stringNode(cur);
		if (v[cur]) {
			dout << cstr << " already visited, skipping." << endl;
			continue;
		}
		v[cur] = true;
		for (ListDigraph::OutArcIt a(_cfg, cur); a != INVALID; ++a) {
			ListDigraph::Node kid = _cfg.runningNode(a);
			string ks = _cfg.stringNode(kid);
			if (_cfr_addr[kid] != entry) {
				dout << ks << " in next CFR." << endl;
				next_cfr.push_back(kid);
				continue;
			}
			ListDigraph::Node cfr_kid;
			if (kid == entry) {
				/* Finishing a loop */
				cfr_kid = cfr_entry;
			} else {
				cfr_kid = cfr->fromCFG(kid);
				if (cfr_kid == INVALID) {
					cfr_kid = cfr->addNode(kid);
				}
			}
			dout << cstr << " → " << ks << endl;
			cfr->addArc(cfr_node, cfr_kid);
			nexts.push_back(kid);
		}
	} while(!nexts.empty());

	dbg.dec();
	dout << *cfr << " end" << endl;
	dbg.dec(); dbg.flush(xlog);

	return next_cfr;
}

void
CFRFactory::ensureArc(CFR* source, CFR* target) {
	#define dout dbg.buf << dbg.start
	dbg.inc("⌒: ");
	ListDigraph::Node cfrgs, cfrgt;
	cfrgs = cfrg->findNode(source);
	cfrgt = cfrg->findNode(target);
	ListDigraph::Arc a=INVALID;

	a = findArc(*cfrg, cfrgs, cfrgt);
	if (a != INVALID) {
		dbg.dec();
		return;
	}

	dout << *source << " → " << *target << endl; 
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
