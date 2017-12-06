#include "CFRG.h"

static string
cfr_desc(CFRG &cfrg, ListDigraph::Node cfr_node) {
	if (cfr_node == INVALID) {
		return "INVALID";
	}
	CFG *cfg = cfrg.getCFG();
	CFR *cfr = cfrg.findCFR(cfr_node);
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_initial = cfr->toCFG(cfr_initial);
	
	return cfg->stringNode(cfg_initial);
}
static void
debug_queue(pqueue_t &pqueue, CFRG &cfrg, ListDigraph::NodeMap<int> &dists) {
	pqueue_t::iterator it;
	cout << "PRIORITY QUEUE SIZE: " << pqueue.size() << endl;
	for (it = pqueue.begin(); it != pqueue.end(); ++it) {
		ListDigraph::Node node = *it;
		cout << "Dist: " << dists[node]
		     << " -- " << cfr_desc(cfrg, node) << endl;
	}
}

ListDigraph::Node
CFRG::addNode(CFR *cfr) {
	cout << "CFRG::addNode " << *cfr << endl;
	map<CFR*, ListDigraph::Node>::iterator mit =
		_from_cfr_to_node.find(cfr);
	if (mit != _from_cfr_to_node.end()) {
		return mit->second;
	}
	ListDigraph::Node rv = ListDigraph::addNode();
	_from_cfr_to_node[cfr] = rv;
	_from_node_to_cfr[rv] = cfr;
	_gen[rv] = 0;
	return rv;
}

void
CFRG::order() {
	ListDigraph::NodeMap<int> distances(*this);
	node_map_t prev;
	ListDigraph::Node source = getInitial();
	ListDigraph::Node target = getTerminal();

	_order(source, target, distances, prev);
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		_gen[node] = -1 * distances[node];
	}
}

bool
CFRG::isHead(ListDigraph::Node cfrg_node) {
	CFR *cfr = findCFR(cfrg_node);
	return cfr->isHead(cfr->getInitial());
}

bool
CFRG::sameLoopNode(ListDigraph::Node a, ListDigraph::Node b) {
	return sameLoop(findCFR(a), findCFR(b));
}

bool
CFRG::sameLoop(CFR* a, CFR *b) {
	cout << "CFRG::sameLoop: " << *a << " vs " << endl
	     << "CFRG::sameLoop: " << *b << endl;
	ListDigraph::Node init_a, init_b;
	init_a = a->getInitial();
	init_b = b->getInitial();

	ListDigraph::Node cfg_a, cfg_b;
	cfg_a = a->toCFG(init_a);
	cfg_b = b->toCFG(init_b);

	return _cfg.sameLoop(cfg_a, cfg_b);
}

bool
CFRG::inLoop(CFR* head, CFR* cfr) {
	string prefix = "CFRG::inLoop: ";
	if (head == cfr) {
		cout << prefix << "same CFR returning true" << endl;
		return true;
	}
	ListDigraph::Node headi, cfri;
	headi = head->getInitial();
	cfri = cfr->getInitial();

	ListDigraph::Node cfgh, cfgc;
	cfgh = head->toCFG(headi);
	cfgc = cfr->toCFG(cfri);

	return _cfg.inLoop(cfgh, cfgc);
}

CFR *
CFRG::findCFRbyCFGNode(ListDigraph::Node node) {
	iaddr_t src_addr = _cfg.getAddr(node);
	FunctionCall src_func = _cfg.getFunction(node);
	
	map<CFR*, ListDigraph::Node>::iterator mit = _from_cfr_to_node.begin();

	for ( ; mit != _from_cfr_to_node.end(); ++mit) {
		CFR *cursor = mit->first;
		ListDigraph::Node cfri = cursor->getInitial();
		ListDigraph::Node cfgi = cursor->toCFG(cfri);
		iaddr_t cur_addr = _cfg.getAddr(cfgi);
		if (cur_addr != src_addr) {
			continue;
		}
		FunctionCall cur_func = _cfg.getFunction(cfgi);
		if (cur_func != src_func) {
			continue;
		}
		return cursor;
	}
	return NULL;
}


void
CFRG::doLoopOffsets(ListDigraph::NodeMap<int> &loopOffset) {
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		
	}
}

/**
 * The pqueue_t may contain more than one node with the same distance (which
 * determines its key value). This method returns an iterator to the exact node
 * passed in.
 *
 * @param[in] pqueue the queue being searched
 * @param[in] target the node being searched for
 *
 * @return an iterator pointing to the target, or an iterator that is equal to
 *  pqueue.end() 
 */
static pqueue_t::iterator 
pqfind(pqueue_t &pqueue, ListDigraph::Node &target) {
	pqueue_t::iterator pit;

	pit = pqueue.find(target);
	while (pit != pqueue.end() && *pit != target) {
		++pit;
	}

	return pit;
}

/**
 * Finds the longest path from the source to the target
 *
 * The result is stored in distances, a mapping from node to it's
 * distance from the source.
 */
void
CFRG::dijk(ListDigraph::Node source, ListDigraph::Node target,
	   ListDigraph::NodeMap<int> &distances,
	   node_map_t &prev) {
	pqueue_t pqueue((CFRGNodeComp(distances)));
	ListDigraph::Node cur;
	prev.clear();

	ListDigraph::NodeIt nit(*this);
	for ( ; nit != INVALID; ++nit) {
		cur = nit;
		distances[cur] = INT_MAX;
		pqueue.insert(cur);
	}

	pqueue_t::iterator sit = pqfind(pqueue, source);
	pqueue.erase(sit);
	distances[source] = 0;
	pqueue.insert(source);
	
	while (!pqueue.empty()) {
		pqueue_t::iterator it = pqueue.begin();
		ListDigraph::Node node = *it;
		pqueue.erase(it);
		cout << "Selected node: " << cfr_desc(*this, node) << " dist: "
		     << distances[node] << endl;

		if (target == node) {
			/* Found the target, search is complete */
			return;
		}

		for (ListDigraph::OutArcIt ait(*this, node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			it = pqfind(pqueue, tgt);
			if (it == pqueue.end()) {
				/* This node has been handled */
				continue; 
			}
			cout << "  kid: " << cfr_desc(*this, tgt)
			     << " dist: " << distances[tgt] << endl;
			int arc_cost = -1; /* Constant */

			bool insert = false;
			int alt = arc_cost + distances[node];
			if (distances[tgt] == INT_MAX) {
				distances[tgt] = alt;
				insert = true;
			}
			if (distances[tgt] > alt) {
				distances[tgt] = alt;
				insert = true;
			}
			if (insert) {
				pqueue.erase(it);
				pqueue.insert(tgt);
				prev[tgt] = node;
			}
		}
	}
}

/**
 * CFR must begin with the loop head instruction
 */
static ListDigraph::Node
getCFGLoopHead(CFR *cfr) {
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_head = cfr->toCFG(cfr_initial);

	return cfg_head;
}

void CFRG::_order(ListDigraph::Node source,
		  ListDigraph::Node target,
		  ListDigraph::NodeMap<int> &distances,
		  node_map_t &prev) {
	pqueue_t pqueue((CFRGNodeComp(distances)));
	ListDigraph::Node cur;
	prev.clear();

	ListDigraph::NodeIt nit(*this);
	for ( ; nit != INVALID; ++nit) {
		cur = nit;
		distances[cur] = INT_MAX;
		pqueue.insert(cur);
	}

	pqueue_t::iterator sit = pqfind(pqueue, source);
	pqueue.erase(sit);
	distances[source] = 0;
	pqueue.insert(source);
	
	while (!pqueue.empty()) {
		pqueue_t::iterator it = pqueue.begin();
		ListDigraph::Node node = *it;
		pqueue.erase(it);
		if (target == node) {
			/* Found the target, search is complete */
			cout << "_order reached target, done." << endl;
			return;
		}
		cout << "_order Selected node: " << cfr_desc(*this, node)
		     << " dist: " << distances[node] << endl;

		if (isHead(node)) {
			interiorLoopOrder(node, pqueue, distances, prev);
		}
		    
		for (ListDigraph::OutArcIt ait(*this, node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			it = pqfind(pqueue, tgt);
			if (it == pqueue.end()) {
				/* This node has been handled */
				continue; 
			}
			int arc_cost = -1; /* Constant */

			int alt = arc_cost + distances[node];
			if (distances[tgt] == INT_MAX || distances[tgt] > alt) {
				pqueue.erase(it);

				distances[tgt] = alt;
				pqueue.insert(tgt);
				prev[tgt] = node;
				cout << "_order updated node: "
				     << cfr_desc(*this, tgt) << " dist: "
				     << distances[tgt] << endl;
			}
		}
	}
}


/**
 * Treats all nodes of a user loop as a single node while traversing
 * the CFRG assigning distances
 *
 * @param[in] cfrg_node the node in the CFRG that is a CFR which is
 * the loop head being treated as a single node.
 * @param[in|out] pqueue the priority queue ordered by distance to the
 * CFRs 
 * @param[in|out] distances the mapping of CFRs to their distances
 * @param[in|out] prev the mapping from a node to its predecessor
 * along the shortest path
 * @param[out] succ on return, those nodes which immediately follow
 * the loop 
 */
void
CFRG::interiorLoopOrder(ListDigraph::Node cfrg_node,
			pqueue_t &pqueue,
			ListDigraph::NodeMap<int> &distances,
			node_map_t &prev)
{
	indentInc();
	string _pre = _indent + "interiorLoopOrder " + cfr_desc(*this, cfrg_node)
		+ " ";

	cout << _pre << "BEGIN STAGE 1" << endl;
	set<ListDigraph::Node> succ;
	ListDigraph::Node cfg_head = getCFGLoopHead(findCFR(cfrg_node));
	/*
	 * Need a single pass of dijkstra, limited to within the loop to update
	 * distances correctly.
	 */
	for (ListDigraph::OutArcIt ait(*this, cfrg_node); ait != INVALID;
	     ++ait) {
		ListDigraph::Node tgt = ListDigraph::target(ait);
		pqueue_t::iterator pit = pqfind(pqueue, tgt);
		if (pit == pqueue.end()) {
			/* previously handled */
			continue;
		}
		if (isHead(tgt)) {
			cout << _pre << "recursive call for" << endl
			     << _indent << "    " << cfr_desc(*this, tgt) << endl;
			
			/* Distance to tgt needs to be updated first */
			pqueue_t::iterator pit = pqfind(pqueue, tgt);
			pqueue.erase(pit);
			if (distances[tgt] > distances[cfrg_node] - 1) {
				distances[tgt] = distances[cfrg_node] - 1;
			}

			interiorLoopOrder(tgt, pqueue, distances, prev);
			/* Handled the loop, go back to the queue */
			continue;
		}
		CFR *cfr = findCFR(tgt);
		if (cfg_head != _cfg.getHead(getCFGLoopHead(cfr))) {
			/* Outside of the loop */
			cout << _pre << endl << _indent << "     "
			     << cfr_desc(*this, tgt)
			     << " added to successors" << endl;
			succ.insert(tgt);
			continue;
		}
		/* Otherwise a normal update */
		int arc_cost = -1; /* Constant */

		int alt = arc_cost + distances[cfrg_node];
		if (distances[tgt] == INT_MAX || distances[tgt] > alt) {
			pqueue.erase(pit);

			distances[tgt] = alt;
			pqueue.insert(tgt);
			prev[tgt] = cfrg_node;
			cout << _pre << " updated node: " << endl
			     << cfr_desc(*this, tgt) << " dist: "
			     << distances[tgt] << endl;
		}
	}
	cout << _pre << "END STAGE 1" << endl;	
	
	ListDigraph::Node cfrg_cursor =	smallestInLoop(cfrg_node, pqueue);
	while (cfrg_cursor != INVALID) {
		cout << _pre << endl << _indent << "    "
		     << "smallest in loop: " << cfr_desc(*this, cfrg_cursor)
		     << endl;
		pqueue_t::iterator pit = pqfind(pqueue, cfrg_cursor);
		pqueue.erase(pit);

		if (isHead(cfrg_cursor)) {
			interiorLoopOrder(cfrg_cursor, pqueue, distances, prev);
		}

		for (ListDigraph::OutArcIt ait(*this, cfrg_cursor);
		     ait != INVALID; ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			pit = pqfind(pqueue, tgt);
			if (pit == pqueue.end()) {
				/* This node has been handled */
				continue; 
			}
			CFR *cfr = findCFR(tgt);
			if (cfr->getHead(cfr->getInitial()) != cfg_head) {
				cout << _pre << endl << _indent << "    "
				     << "skipping " << cfr_desc(*this, tgt)
				     << endl;
				/* Not in this loop */
				succ.insert(tgt);
				continue;
			}
			int arc_cost = -1; /* Constant */

			int alt = arc_cost + distances[cfrg_cursor];
			if (distances[tgt] == INT_MAX || distances[tgt] > alt) {
				pqueue.erase(pit);

				distances[tgt] = alt;
				pqueue.insert(tgt);
				prev[tgt] = cfrg_cursor;
				cout << _pre << endl << _indent << "    "
				     << "updated node "
				     << cfr_desc(*this, tgt) << " dist: "
				     << distances[tgt] << endl;
			}
		}
		cfrg_cursor = smallestInLoop(cfrg_node, pqueue);
	}

	/*
	 * The distance to the head of the loop needs to be updated once it's
	 * been processed.
	 */
	int min = distances[cfrg_node];
	for (ListDigraph::InArcIt ait(*this, cfrg_node); ait != INVALID;
	     ++ait) {
		ListDigraph::Node pred = source(ait);
		if (cfg_head != _cfg.getHead(getCFGLoopHead(findCFR(pred)))) {
			continue;
		}
		if (distances[pred] < distances[cfrg_node]) {
			min = distances[pred] - 1;
		}
	}
	/* 
	 * Set the distance to the head as the 
	 * longest to reach it + 1 more arc
	 */
	distances[cfrg_node] = min;
	iloSuccessorUpdate(distances[cfrg_node] - 1, pqueue, distances,
			   prev, succ);
	
	indentDec();
}

/**
 * Finds the CFRG node with the smallest distance in the pqueue which is in the
 * same loop as the cfrg_node.
 *
 * @param[in] cfrg_node the node that starts a loop.
 * @param[in] pqueue the queue with associated distances
 *
 * @return the node in the CFRG which is contained within the loop with the
 * smallest distance.
 */       
ListDigraph::Node
CFRG::smallestInLoop(ListDigraph::Node cfrg_node, pqueue_t &pqueue) {
	indentInc();
	string	_pre = _indent + "smallestInLoop " + cfr_desc(*this, cfrg_node)
		+ "\n" + _indent + "    ";
	CFR *cfr = findCFR(cfrg_node);
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_head = cfr->toCFG(cfr_initial);

	#if 0
	cout << _pre << "found CFG head: " << _cfg.stringNode(cfg_head) << endl;
	#endif

	pqueue_t::iterator it = pqueue.begin();
	ListDigraph::Node cfrg_next_node = *it;
	ListDigraph::Node cfg_next_head;
	do {
		CFR *cfr_next = findCFR(cfrg_next_node);
		cfg_next_head = _cfg.getHead(getCFGLoopHead(cfr_next));

		if (cfg_next_head == cfg_head) {
			/* It's in the loop, bail */
			break;
		}
		it++;
		cfrg_next_node = *it;
	} while (it != pqueue.end());
	indentDec();

	if (it == pqueue.end()) {
		cfrg_next_node = INVALID;
	}
	cout << _pre << "returning "
	     << cfr_desc(*this, cfrg_next_node) << endl;
	return cfrg_next_node;
}

void
CFRG::iloSuccessorUpdate(int new_distance,
			 pqueue_t &pqueue,
			 ListDigraph::NodeMap<int> &distances,
			 node_map_t &prev,
			 set<ListDigraph::Node> &succ)
{
	set<ListDigraph::Node>::const_iterator sit;
	for (sit = succ.begin(); sit != succ.end(); sit++) {
		ListDigraph::Node next = *sit;
		if (distances[next] > new_distance) {
			pqueue_t::iterator it =	pqfind(pqueue, next);
			pqueue.erase(it);
			distances[next] = new_distance;
			pqueue.insert(next);
		}
	}
}

