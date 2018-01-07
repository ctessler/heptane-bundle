#include "CFRG.h"

bool CFRGNodeComp::operator() (const ListDigraph::Node &lhs,
			       const ListDigraph::Node &rhs) const {
	if (_dist[lhs] < _dist[rhs]) {
		/* Least first */
		return true;
	}
	return false;
}


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
static string
debug_queue(pqueue_t &pqueue, CFRG &cfrg, ListDigraph::NodeMap<int> &dists) {
	pqueue_t::iterator it;
	stringstream ss;
	ss << "PRIORITY QUEUE SIZE: " << pqueue.size() << endl;
	for (it = pqueue.begin(); it != pqueue.end(); ++it) {
		ListDigraph::Node node = *it;
		ss << "Dist: " << dists[node]
		     << " -- " << cfr_desc(cfrg, node) << endl;
	}
	return ss.str();
}

ListDigraph::Node
CFRG::addNode(CFR *cfr) {
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

ListDigraph::Node
CFRG::findNode(CFR *cfr) {
	map<CFR*, ListDigraph::Node>::iterator mit =
		_from_cfr_to_node.find(cfr);
	if (mit == _from_cfr_to_node.end()) {
		return INVALID;
	}
	return mit->second;
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
	dbg.flush(ord);
}

void
CFRG::setInitialCFR(CFR *cfr) {
	_initial = cfr;
	ListDigraph::Node node = findNode(cfr);
	if (node == INVALID) {
		throw runtime_error("setInitialCFR Invalid CFR");
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
	if (head == cfr) {
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

bool
CFRG::inLoopNode(ListDigraph::Node cfrg_head_node, ListDigraph::Node cfrg_node) {
	return inLoop(findCFR(cfrg_head_node), findCFR(cfrg_node));
}

bool
CFRG::isLoopPart(ListDigraph::Node cfrg_node) {
	CFR *cfr = findCFR(cfrg_node);
	if (cfr->isHead(cfr->getInitial())) {
		return true;
	}
	ListDigraph::Node cfg_node = cfr->getHead(cfr->getInitial());
	if (cfg_node != INVALID) {
		return true;
	}
	return false;
}
bool
CFRG::isLoopPartCFR(CFR* cfr) {
	ListDigraph::Node node = findNode(cfr);
	return isLoopPart(node);
}

CFR*
CFRG::crown(CFR *cfr) {
	CFR *rval = NULL;
	if (cfr->isHead(cfr->getInitial())) {
		rval = cfr;
	}
	ListDigraph::Node cfg_head = cfr->getHead(cfr->getInitial());
	while (cfg_head != INVALID) {
		rval = findCFRbyCFGNode(cfg_head);
		cfg_head = rval->getHead(rval->getInitial());
	}
	return rval;
}

bool
CFRG::isLoopHead(CFR *cfr) {
	return cfr->isHead(cfr->getInitial());
}

CFRList*
CFRG::preds(CFR *cfr) {
	CFRList *list = new CFRList();
	ListDigraph::Node cfrg_node = findNode(cfr);
	for (ListDigraph::InArcIt iat(*this, cfrg_node); iat != INVALID; ++iat) {
		ListDigraph::Node pred_node = source(iat);
		CFR *pred_cfr = findCFR(pred_node);
		list->push_back(pred_cfr);
	}
	return list;
}

CFR *
CFRG::findCFRbyCFGNode(ListDigraph::Node node) {
	if (node == INVALID) {
		return NULL;
	}
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

#define dout dbg.buf << dbg.start
void CFRG::_order(ListDigraph::Node source,
		  ListDigraph::Node target, /* does not work for longest path */
		  ListDigraph::NodeMap<int> &distances,
		  node_map_t &prev) {
	dbg.inc("_order: ");
	pqueue_t pqueue((CFRGNodeComp(distances)));
	ListDigraph::Node cur;
	prev.clear();

	ListDigraph::NodeIt nit(*this);
	for ( ; nit != INVALID; ++nit) {
		cur = nit;
		distances[cur] = INT_MAX;
		pqueue.insert(cur);
		dout << "Added node " << cfr_desc(*this, cur) << " "
		     << distances[cur] << endl;
	}

	pqueue_t::iterator sit = pqfind(pqueue, source);
	pqueue.erase(sit);
	distances[source] = 0;
	pqueue.insert(source);
	
	while (!pqueue.empty()) {
		pqueue_t::iterator it = pqueue.begin();
		ListDigraph::Node node = *it;
		pqueue.erase(it);
		dout << "Selected node: " << cfr_desc(*this, node)
		     << " dist: " << distances[node] << endl;

		dbg.flush(ord);
		if (isHead(node)) {
			dout << "Calling interiorLoopOrder" << endl;
			interiorLoopOrder(node, pqueue, distances, prev);
		}
		    
		dbg.inc("succs: ", " → ");
		for (ListDigraph::OutArcIt ait(*this, node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			dout << cfr_desc(*this, tgt) << " dist: "
			     << distances[tgt] << endl;
			it = pqfind(pqueue, tgt);
			if (!isHead(tgt) && isLoopPart(tgt)) {
				dout << "Not head, in a loop, skipped" << endl;
				/* Loop nodes are handled specially */
				#if 0 /* XXX-ct suspect */
				if (it != pqueue.end()) {
					pqueue.erase(it);
				}
				#endif
				continue;
			}
			if (isHead(node) && inLoopNode(node, tgt)) {
				/* Target is in the loop of node */
				dout << "Interior loop node of "
				     << cfr_desc(*this, node) << endl;
				#if 0 /* XXX-ct suspect */
				if (it != pqueue.end()) {
					pqueue.erase(it);
				}
				#endif
				continue;
			}
			int arc_cost = -1; /* Constant */

			int alt = arc_cost + distances[node];
			if (distances[tgt] > alt) {
				bool req = false;
				if (it != pqueue.end()) {
					pqueue.erase(it);
					req = true;
				}
				distances[tgt] = alt;
				if (req) {
					pqueue.insert(tgt);
				}
				prev[tgt] = node;
				dout << "updated node: "
				     << cfr_desc(*this, tgt) << " dist: "
				     << distances[tgt] << endl;
			}
		}
		dbg.dec();
	}
	dbg.dec();
}
#undef dout


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
static int level=1;
#define dout dbg.buf << dbg.start
void
CFRG::interiorLoopOrder(ListDigraph::Node cfrg_node,
			pqueue_t &pqueue,
			ListDigraph::NodeMap<int> &distances,
			node_map_t &prev)
{
	string _pre = "→ iLO " + cfr_desc(*this, cfrg_node) + ": ";
	dbg.inc(_pre);
	dout << "BEGIN STAGE 1" << endl;
	set<ListDigraph::Node> succ;
	ListDigraph::Node cfg_head = getCFGLoopHead(findCFR(cfrg_node));
	/*
	 * Need a single pass of dijkstra, limited to within the loop to update
	 * distances correctly.
	 */
	dout << "Successor nodes (⚲)" << endl;
	for (ListDigraph::OutArcIt ait(*this, cfrg_node); ait != INVALID;
	     ++ait) {
		ListDigraph::Node tgt = ListDigraph::target(ait);
		dout << "⚲ " << cfr_desc(*this, tgt) << endl;
		pqueue_t::iterator pit = pqfind(pqueue, tgt);
		if (isHead(tgt) && inLoopNode(cfrg_node, tgt)) {
			dout << "recursive call for" << endl;
			dout << "    " << cfr_desc(*this, tgt) << endl;

			/* Distance to tgt needs to be updated first */
			pqueue_t::iterator pit = pqfind(pqueue, tgt);
			bool req=false;
			if (pit != pqueue.end()) {
				pqueue.erase(pit);
				req=true;
			}
			if (distances[tgt] > distances[cfrg_node] - 1) {
				distances[tgt] = distances[cfrg_node] - 1;
			}
			if (req) {
				pqueue.insert(tgt);
			}

			interiorLoopOrder(tgt, pqueue, distances, prev);
			/* Handled the loop, go back to the queue */
			--level;
			continue;
		}
		CFR *cfr = findCFR(tgt);
		if (cfg_head != _cfg.getHead(getCFGLoopHead(cfr))) {
			/* Outside of the loop */
			dout << cfr_desc(*this, tgt)
			     << " added to successors" << endl;
			succ.insert(tgt);
			continue;
		}

		/* Otherwise a normal update */
		int arc_cost = -1; /* Constant */
		
		int alt = arc_cost + distances[cfrg_node];
		if (distances[tgt] == INT_MAX || distances[tgt] > alt) {
			bool req=false;
			if (pit != pqueue.end()) {
				pqueue.erase(pit);
				req=true;
			}
			
			distances[tgt] = alt;
			if (req) {
				pqueue.insert(tgt);
			}
			prev[tgt] = cfrg_node;
			dout << "updated: " 
			     << cfr_desc(*this, tgt) << " dist: "
			     << distances[tgt] << endl;
		}
	}
	dout << "END STAGE 1" << endl;
	dout << "BEGIN STAGE 2" << endl;
	dbg.flush(ilo);
	ListDigraph::Node cfrg_cursor =	smallestInLoop(cfrg_node, pqueue);
	dbg.flush(ilo);
	while (cfrg_cursor != INVALID) {
		dout << "↬ : "
		     << cfr_desc(*this, cfrg_cursor)
		     << " dist: " << distances[cfrg_cursor] << endl;
		dbg.flush(ilo);
		pqueue_t::iterator pit = pqfind(pqueue, cfrg_cursor);
		if (pit != pqueue.end()) {
			pqueue.erase(pit);
			dout << "Removed " << cfr_desc(*this, cfrg_cursor)
			     << " from queue" << endl;
			dbg.flush(ilo);
		}

		if (isHead(cfrg_cursor)) {
			dout << __LINE__ << " recursive call" << endl;
			dbg.flush(ilo);
			interiorLoopOrder(cfrg_cursor, pqueue, distances, prev);
			dout << __LINE__ << " return from recursive call" << endl;
			dbg.flush(ilo);
			cfrg_cursor = smallestInLoop(cfrg_node, pqueue);
			continue;
		}

		dbg.inc("↓ " + cfr_desc(*this, cfrg_cursor) + " ");
		dout <<  "successors: (⚲)" << endl;
		dbg.inc("⚲ ");
		dbg.flush(ilo);
		for (ListDigraph::OutArcIt ait(*this, cfrg_cursor);
		     ait != INVALID; ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			dout << "⚲ " << cfr_desc(*this, tgt)
			     << " dist: " << distances[tgt] << endl;
			CFR *cfr = findCFR(tgt);
			if (cfr->getHead(cfr->getInitial()) != cfg_head) {
				dout << "skipping " << cfr_desc(*this, tgt)
				     << endl;
				/* Not in this loop */
				succ.insert(tgt);
				continue;
			}
			int arc_cost = -1; /* Constant */

			int alt = arc_cost + distances[cfrg_cursor];
			if (distances[tgt] == INT_MAX || distances[tgt] > alt) {
				bool req=false;
				pit = pqfind(pqueue, tgt);
				if (pit != pqueue.end()) {
					pqueue.erase(pit);
					req=true;
				}

				distances[tgt] = alt;
				prev[tgt] = cfrg_cursor;
				dout << "updated node "
				     << cfr_desc(*this, tgt) << " dist: "
				     << distances[tgt] << endl;
				if (req) {
					pqueue.insert(tgt);
				}
			}
			dbg.flush(ilo);
		}
		dbg.dec();
		dbg.dec();
		dbg.flush(ilo);
		cfrg_cursor = smallestInLoop(cfrg_node, pqueue);
		dout << __LINE__ << " ↓ in loop: " << cfr_desc(*this, cfrg_cursor)
		     << " dist: " << distances[cfrg_cursor] << endl;
	}
	dout << "END STAGE 2" << endl;
	dbg.flush(ilo);

	/*
	 * The distance to the head of the loop needs to be updated once it's
	 * been processed.
	 */
	int min = distances[cfrg_node];
	int count=0;
	dbg.inc("Min Select: ");
	for (ListDigraph::InArcIt ait(*this, cfrg_node); ait != INVALID;
	     ++ait) {
		ListDigraph::Node pred = source(ait);
		count++;
		if (cfg_head != _cfg.getHead(getCFGLoopHead(findCFR(pred)))) {
			continue;
		}
		dout << "Candidate " << cfr_desc(*this, pred)
		     << " dist: " << distances[pred] << endl;
		if (distances[pred] < distances[cfrg_node]) {
			min = distances[pred] - 1;
		}
	}
	dout << "Selected distance from " << count << " successors" << endl;
	/* 
	 * Set the distance to the head as the 
	 * longest to reach it + 1 more arc
	 */
	distances[cfrg_node] = min;
	dout << "Distance: " << distances[cfrg_node] << endl;
	iloSuccessorUpdate(distances[cfrg_node], pqueue, distances,
			   prev, succ);
	dbg.dec();
	dbg.flush(ilo);
}
#undef dout

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
#define dout dbg.buf << dbg.start
ListDigraph::Node
CFRG::smallestInLoop(ListDigraph::Node cfrg_node, pqueue_t &pqueue) {
	dout << "Called with head: " << cfr_desc(*this, cfrg_node) << endl;
	string	_pre = "→ SIL: " + cfr_desc(*this, cfrg_node) + " " ;
	dbg.inc(_pre);
	CFR *cfr = findCFR(cfrg_node);
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_head = cfr->toCFG(cfr_initial);

	dout << "found CFG head: " << endl
	     << "	" << _cfg.stringNode(cfg_head) << endl;
	dbg.flush(sil);
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
		if (it != pqueue.end()) {
			it++;
			cfrg_next_node = *it;
		}
	} while (it != pqueue.end());

	if (it == pqueue.end()) {
		cfrg_next_node = INVALID;
	}
	dout << "returning " << cfr_desc(*this, cfrg_next_node) << endl;
	dbg.dec();
	dbg.flush(sil);
	return cfrg_next_node;
}

void
CFRG::iloSuccessorUpdate(int new_distance,
			 pqueue_t &pqueue,
			 ListDigraph::NodeMap<int> &distances,
			 node_map_t &prev,
			 set<ListDigraph::Node> &succ)
{
	dbg.inc("→ iLOsu: ");
	dout << "begin, successors: " << succ.size()
	     << " new distance: " << new_distance << endl;
	set<ListDigraph::Node>::const_iterator sit;
	for (sit = succ.begin(); sit != succ.end(); sit++) {
		ListDigraph::Node next = *sit;
		dout << "succ: " << cfr_desc(*this, next) << " distance: "
		     << distances[next] << endl;
		if (distances[next] > new_distance) {
			pqueue_t::iterator it =	pqfind(pqueue, next);
			bool req=false;
			if (it != pqueue.end()) {
				pqueue.erase(it);
				req=true;
			}
			distances[next] = new_distance;
			dout << "updated: " << cfr_desc(*this, next)
			     << " distance: " << distances[next] << endl;
			if (req) {
				pqueue.insert(next);
			}
		}
	}
	dout << "end" << endl;
	dbg.dec();
}

#undef dout
