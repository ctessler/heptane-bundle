#include "CFRGFactory.h"

CFRGFactory::CFRGFactory(LemonCFG &cfg, Cache &cache) : 
	_cfg(cfg), _cache(cache) {
}

CFRGFactory::~CFRGFactory() {
}

LemonCFRG*
CFRGFactory::run() {
	LemonCFRG *cfrg = new LemonCFRG(_cfg);

	/* This modifies the CFG, permanently */
	_cfg.cacheAssign(&_cache);
	_cfg.getCFRMembership(&_cache);

	_cfrs = CFRFactory::separateCFRs(_cfg);
	for (_cfrit = _cfrs.begin(); _cfrit != _cfrs.end(); _cfrit++) {
		/* First is the node from the CFG */
		ListDigraph::Node cfg_node = _cfrit->first;
		/* Second is the new LemonCFG */
		LemonCFG* subcfg = _cfrit->second;

		/* Create a new CFR in the CFRG */
		ListDigraph::Node cfr_node = cfrg->addNode(cfg_node);
		cout << "CFRG added node: " << cfrg->getStartString(cfr_node) << endl;
		cfrg->setWCET(cfr_node, 1, subcfg->CFRWCET(subcfg->getRoot(), 1, _cache));
		cfrg->setWCET(cfr_node, 2, cfrg->getWCET(cfr_node, 1) -
			      subcfg->CFRWCET(subcfg->getRoot(), 2, _cache));
	}

	for (_cfrit = _cfrs.begin(); _cfrit != _cfrs.end(); _cfrit++) {
		ListDigraph::Node cfg_node = _cfrit->first;
		ListDigraph::Node cfr_node = cfrg->findNode(cfg_node);
		cout << "CFRGFactory CFR[" << _cfg.getStartString(cfg_node) << "]" << endl;
		

		list<ListDigraph::Node> next_cfrs = succ(_cfg, cfg_node);
		list<ListDigraph::Node>::iterator cfrs_it;
		for (cfrs_it = next_cfrs.begin(); cfrs_it != next_cfrs.end(); cfrs_it++) {
			ListDigraph::Node next_cfg_node = *cfrs_it;
			ListDigraph::Node next_cfr_node = cfrg->findNode(next_cfg_node);
			cout << "\tCFRG adding arc: " << cfrg->getStartString(cfr_node) << " -> "
			     << _cfg.getStartString(next_cfg_node) << endl;
			cfrg->addArc(cfr_node, next_cfr_node);
		}
	}

	ListDigraph::Node cfg_root = _cfg.getRoot();
	ListDigraph::Node cfr_root = cfrg->findNode(cfg_root);
	cfrg->setRoot(cfr_root);

	for (_cfrit = _cfrs.begin(); _cfrit != _cfrs.end(); _cfrit++) {
		delete _cfrit->second;
	}

	return cfrg;
}

/**
 * node should be the CFR initial instruction
 */
list<ListDigraph::Node>
CFRGFactory::succ(LemonCFG &cfg, ListDigraph::Node node) {
	list<ListDigraph::Node> cfrs;

	/* Depth first search */
	ListDigraph::NodeMap<bool> visited(cfg);
	stack<ListDigraph::Node> kids;
	kids.push(node);
	while (!kids.empty()) {
		ListDigraph::Node u = kids.top(); kids.pop();
		visited[u] = true;

		ListDigraph::Node cfr_initial = cfg.getCFR(u);
		if (cfr_initial != node) {
			list<ListDigraph::Node>::iterator lit;
			bool inserted=false;
			for (lit = cfrs.begin(); lit != cfrs.end(); lit++) {
				if (*lit == cfr_initial) {
					inserted = true;
				}
			}
			if (!inserted) {
				cfrs.push_back(cfr_initial);
			}
			continue;
		}

		for (ListDigraph::OutArcIt ait(cfg, u); ait != INVALID; ++ait) {
			ListDigraph::Node kid = cfg.runningNode(ait);
			if (visited[kid]) {
				continue;
			}
			kids.push(kid);
		}
	}
	
	return cfrs;
}
	

	
