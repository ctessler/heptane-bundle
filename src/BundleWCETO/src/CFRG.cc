#include "CFRG.h"
#include "CFRGDFS.h"

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

#define dout dbg.buf << dbg.start
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

	_order_new(source, distances, prev);
	
	//_order(source, target, distances, prev);
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		_gen[node] = distances[node];
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
CFRG::isHeadCFR(CFR *cfr) {
	return cfr->isHead(cfr->getInitial());
}

bool
CFRG::isTopHead(CFR* cfr) {
	ListDigraph::Node cfri = cfr->getInitial();
	ListDigraph::Node cfrh = cfr->getHead(cfri);
	return (cfrh == INVALID);
}
bool
CFRG::isTopHeadNode(ListDigraph::Node cfrg_node) {
	return isTopHead(findCFR(cfrg_node));
}

CFR*
CFRG::getHead(CFR* cfr) {
	ListDigraph::Node cfri = cfr->getInitial();
	ListDigraph::Node cfgh = cfr->getHead(cfri);

	if (cfgh == INVALID) {
		return NULL;
	}

	CFR *rval = findCFRbyCFGNode(cfgh);
	return rval;
}

ListDigraph::Node
CFRG::getHeadNode(ListDigraph::Node cfrg_node) {
	CFR *cfr = getHead(findCFR(cfrg_node));
	return findNode(cfr);
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
CFRG::inDerivedLoop(CFR *head, CFR *cfr) {
	bool found = (head == cfr);
	ListDigraph::Node cfg_head, cfg_other;
	cfg_head = head->toCFG(head->getInitial());
	cfg_other = cfr->toCFG(cfr->getInitial());
	while (!found) {
		ListDigraph::Node cfg_other_head = _cfg.getHead(cfg_other);
		if (cfg_other_head == INVALID) {
			/* Not found, fail */
			break;
		}
		if (cfg_other_head == cfg_head) {
			/* Found, success! */
			found = true;
			break;
		}
		cfg_other = cfg_other_head;
	}
	return found;
}

bool
CFRG::inDerivedLoopNode(ListDigraph::Node cfrg_head_node,
			ListDigraph::Node cfrg_node) {
	return inDerivedLoop(findCFR(cfrg_head_node), findCFR(cfrg_node));
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
	pqueue_t pqueue((NodeCompLT(distances)));
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

void CFRG::_order(ListDigraph::Node source,
		  ListDigraph::Node target, /* does not work for longest path */
		  ListDigraph::NodeMap<int> &distances,
		  node_map_t &prev) {
	dbg.inc("_order: ");
	pqueue_t pqueue((NodeCompLT(distances)));
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
			dout << "BEGIN interiorLoopOrder" << endl;
			dbg.flush(ord);
			interiorLoopOrder(node, pqueue, distances, prev);
			/* interiorLoopOrder updates successors */
			dout << "END interiorLoopOrder" << endl;
			dbg.flush(ord);
			continue;
		}
		if (!isHead(node) && isLoopPart(node)) {
			string msg = "Interior Loop Node: " +
				cfr_desc(*this, node);
			throw runtime_error(msg);
		}
		dbg.inc("succs: ", " → ");
		for (ListDigraph::OutArcIt ait(*this, node); ait != INVALID;
		     ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			dout << cfr_desc(*this, tgt) << " dist: "
			     << distances[tgt] << endl;

			int arc_cost = -1; /* Constant */
			int alt = arc_cost + distances[node];
			if (distances[tgt] > alt) {
				bool req = false;
				it = pqfind(pqueue, tgt);
				if (it != pqueue.end()) {
					pqueue.erase(it);
					req = true;
				}
				distances[tgt] = alt;
				prev[tgt] = node;
				if (req) {
					pqueue.insert(tgt);
				}
				dout << "updated node: "
				     << cfr_desc(*this, tgt) << " dist: "
				     << distances[tgt] << endl;
			}
		}
		dbg.dec();
	}
	dbg.dec();
}

/**
 * Treats all nodes of a user loop as a single node while traversing
 * the CFRG assigning distances
 *
 * Performs the initial calculation of distances to the CFRG node that
 * is a loop head, the nodes interior to the loop, and updates the
 * nodes outside of the loop.
 *
 * cfrg_node must not be in the priority queue
 * distances[cfrg_node] must be set
 *
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
	dbg.inc("→ iLO: ");
	dout << "BEGIN " << cfr_desc(*this, cfrg_node) << endl;

	set<ListDigraph::Node> succ;
	ListDigraph::Node cfrg_cursor=cfrg_node;
	do {
		dout << "Selected node: " << cfr_desc(*this, cfrg_cursor)
		     << " dist: " << distances[cfrg_cursor] << endl;
		dout << "Successors: (⚲)" << endl;
		dbg.inc("⚲ ");
		dbg.flush(ilo);

		if (cfrg_node != cfrg_cursor && isHead(cfrg_cursor)) {
			dout << __LINE__ << " 《RECURSE "
			     << cfr_desc(*this, cfrg_cursor) << endl;
			dbg.flush(ilo);
			interiorLoopOrder(cfrg_cursor, pqueue,  distances, prev);
			dout << __LINE__ << " RECURSE》"
			     << cfr_desc(*this, cfrg_cursor) << endl;
			dbg.flush(ilo);
			goto nextone;
		}
		
		for (ListDigraph::OutArcIt ait(*this, cfrg_cursor);
		     ait != INVALID; ++ait) {
			ListDigraph::Node tgt = ListDigraph::target(ait);
			dout << cfr_desc(*this, tgt)
			     << " dist: " << distances[tgt] << endl;
			CFR *cfr = findCFR(tgt);
			if (tgt == cfrg_node) {
				dout << cfr_desc(*this, tgt) << " skipped, head"
				     << endl;
				continue;
			}
			if (!inDerivedLoopNode(cfrg_node, tgt)) {
				/* Not in this loop */
				dout << cfr_desc(*this, tgt)
				     << " skipped, added as succ " << endl;
				succ.insert(tgt);
				continue;
			}
			int arc_cost = -1; /* Constant */
			int alt = arc_cost + distances[cfrg_cursor];
			if (distances[tgt] > alt) {
				bool req=false;
				pqueue_t::iterator pit = pqfind(pqueue, tgt);
				if (pit != pqueue.end()) {
					pqueue.erase(pit);
					req=true;
				}
				distances[tgt] = alt;
				prev[tgt] = cfrg_cursor;
				dout << "updated node " << cfr_desc(*this, tgt)
				     << " dist: " << distances[tgt] << endl;
				if (req) {
					pqueue.insert(tgt);
				}
			}
			dbg.flush(ilo);
		}
	nextone:
		dbg.dec();
		dbg.flush(ilo);
		cfrg_cursor = smallestInLoop(cfrg_node, pqueue);
		dbg.flush(ilo);		
	} while(cfrg_cursor != INVALID);
	
	int min = distances[cfrg_node];
	int count=0;
	for (ListDigraph::InArcIt ait(*this, cfrg_node); ait != INVALID;
	     ++ait) {
		ListDigraph::Node pred = source(ait);
		count++;
		if (!inLoopNode(cfrg_node, pred)) {
			continue;
		}
		if (distances[pred] < distances[cfrg_node]) {
			min = distances[pred] - 1;
		}
	}
	distances[cfrg_node] = min;
	dout << "Selected distance from " << count << " successors" << endl;
	dout << "Distance: " << distances[cfrg_node] << endl;
	dbg.flush(ilo);	
	iloSuccessorUpdate(distances[cfrg_node] - 1, pqueue, distances,
			   prev, succ);
	dbg.dec();
	dbg.flush(ilo);
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
	dout << "Called with head: " << cfr_desc(*this, cfrg_node) << endl;
	string	_pre = "→ SIL: " + cfr_desc(*this, cfrg_node) + " " ;
	dbg.inc(_pre);
	CFR *cfr = findCFR(cfrg_node);
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_head = cfr->toCFG(cfr_initial);

	dout << "found CFG head: " << _cfg.stringNode(cfg_head) << endl;
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
		dout << "Loop is empty, returning INVALID" << endl;
		dbg.dec();
		dbg.flush(sil);
		return INVALID;
	}
	dout << "Removing " << cfr_desc(*this, cfrg_next_node)
	     << " from the queue" << endl;
	pqueue.erase(it);
	dout << "returning " << cfr_desc(*this, cfrg_next_node) << endl;
	dbg.dec();
	dbg.flush(sil);
	if (cfg_head != cfg_next_head) {
		throw runtime_error("Nodes are not in the same loop");
	}
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

class OrderData {
public:
	OrderData(ListDigraph::NodeMap<int> &d) :
		distances(d), pqueue((NodeCompGR(d))) {
	}
	int finish=0;
	ListDigraph::NodeMap<int> &distances;
	pqueue_gr_t pqueue;
};

static bool
order_dfs_fin(CFRG &cfrg, CFR *cfr, void *userdata) {
	OrderData *data = (OrderData *) userdata;
	ListDigraph::Node cfrg_node = cfrg.findNode(cfr);
	data->distances[cfrg_node] = data->finish;
	data->finish += 1;
	data->pqueue.insert(cfrg_node);

	return true;
}


void CFRG::_order_new(ListDigraph::Node source,
		      ListDigraph::NodeMap<int> &distances,
		      node_map_t &prev) {
	dbg.inc("→ _ordern: ");
	dout << "Source: " << cfr_desc(*this, source) << endl;
	OrderData data(distances);
	CFRGDFS dfs(*this);
	dfs.setUserData(&data);
	dfs.setFin(order_dfs_fin);
	dfs.recSearch(findCFR(source));

	pqueue_t result(distances);
	pqueue_gr_t::iterator git;
	dout << "Initial order: " << endl;
	dbg.inc("→ _ordern: ");
	int c=0;
	for (git = data.pqueue.begin(); git != data.pqueue.end(); ++git, ++c) {
		ListDigraph::Node cfrg_node = *git;
		dout << cfr_desc(*this, cfrg_node) << " dist: "
		     << distances[cfrg_node] << endl;
		distances[cfrg_node] = c;
		result.insert(cfrg_node);
	}
	dbg.dec();

	pqueue_t::iterator it;
	dout << "Topological order: " << endl;
	dbg.inc("→ _ordern: ");
	for (it = result.begin(); it != result.end(); ++it) {
		ListDigraph::Node cfrg_node = *it;
		distances[cfrg_node] = 0;
		dout << cfr_desc(*this, cfrg_node) << " dist: "
		     << distances[cfrg_node] << endl;
	}
	dbg.dec();
	dbg.flush(ord);

	dout << "Adjustment Pass 1 : " << endl;
	dbg.inc("→ _ordern: ");
	for (it = result.begin(); it != result.end(); ++it) {
		ListDigraph::Node cfrg_node = *it;
		if (!isHead(cfrg_node)) {
			continue;
		}
		/* Only heads */
		if (!isTopHeadNode(cfrg_node)) {
			continue;
		}
		/* Only top-most heads */
		topoAdj(result, cfrg_node, distances);
		dout << cfr_desc(*this, cfrg_node) << " adjustment: "
		     << distances[cfrg_node] << endl;
	}
	dbg.dec();

	dout << "Longest Paths Pass 1 : " << endl;
	dbg.inc("→ _ordern: ");
	for (it = result.begin(); it != result.end(); ++it) {
		ListDigraph::Node cfrg_node = *it;
		int max = topoMax(cfrg_node, distances);
		int weight = 1;
		if (!isTopHeadNode(cfrg_node)) {
			continue;
		}
		if (distances[cfrg_node] < max + weight) {
			distances[cfrg_node] = max + weight;
		}
		dout << cfr_desc(*this, cfrg_node) << " dist: "
		     << distances[cfrg_node] << endl;
	}
	dbg.dec();

	/* Sort by distance */
	ListDigraph::NodeMap<int> sortd(*this);
	pqueue_t sorting(sortd);
	for (it = result.begin(); it != result.end(); ++it) {
		sortd[*it] = distances[*it];
		sorting.insert(*it);
	}

	dout << "Uniqueness Pass: " << endl;
	dbg.inc("→ _ordern: ");
	int uadj=0;
	for (it = sorting.begin(); it != sorting.end(); ++it) {
		ListDigraph::Node cfrg_node = *it;
		if (!isTopHeadNode(cfrg_node)) {
			continue;
		}
		dout << cfr_desc(*this, cfrg_node) << " dist: "
		     << distances[cfrg_node] << " → " ;
		distances[cfrg_node] += uadj;
		if (isHead(cfrg_node)) {
			uadj += 1;
		}
		dbg.buf << distances[cfrg_node] << endl;
	}
	dbg.dec();

	dbg.dec(); dbg.flush(ord);
}

int
CFRG::topoMax(ListDigraph::Node cfrg_node, ListDigraph::NodeMap<int> &dist) {
	dbg.inc("→ topoMax: ");

	int max=0;
	ListDigraph::Node max_pred=INVALID;
	ListDigraph::InArcIt ait(*this, cfrg_node);
	for (; ait != INVALID; ++ait) {
		ListDigraph::Node cfrg_pred = source(ait);
		/* Predecessor is in a loop, find the correct head */
		if (isLoopPart(cfrg_pred)) {
			while (!sameLoopNode(cfrg_pred, cfrg_node)) {
				CFR *pred_cfr = findCFR(cfrg_pred);
				ListDigraph::Node pred_cfgh =
					_cfg.getHead(getCFGLoopHead(pred_cfr));
				pred_cfr = findCFRbyCFGNode(pred_cfgh);
				cfrg_pred = findNode(pred_cfr);
			}
			
		}
		if (dist[cfrg_pred] > max) {
			max = dist[cfrg_pred];
			max_pred = cfrg_pred;
		}
	}
	dbg.dec();
	return max;
}

/**
 * cfrg_node - the CFRG loop head
 */
void
CFRG::topoAdj(pqueue_t &pq, ListDigraph::Node cfrg_node,
	      ListDigraph::NodeMap<int> &dist) {
	pqueue_t::iterator it = pq.begin();
	while (*it != cfrg_node) {
		it++;
	}
	/* it now at cfrg_node */
	int weight = 1;
	while (++it != pq.end()) {
		ListDigraph::Node next = *it;
		if (cfrg_node != getHeadNode(next)) {
			continue;
		}
		/* next is in *a* loop below cfrg_node */
		if (isHead(next)) {
			topoAdj(pq, next, dist);
		}
		/* 
		 * next's innermost head is cfrg_node, it's time to
		 * update it's distance
		 */
		int max = 0;
		ListDigraph::InArcIt ait(*this, next);
		for ( ; ait != INVALID; ++ait) {
			ListDigraph::Node cfrg_pred = source(ait);
			ListDigraph::Node cfrg_pred_head = getHeadNode(cfrg_pred);
			while (cfrg_pred != cfrg_node &&
			       cfrg_pred_head != cfrg_node) {
				cfrg_pred = cfrg_pred_head;
				cfrg_pred_head = getHeadNode(cfrg_pred);
			}
			if (dist[cfrg_pred] > max) {
				max = dist[cfrg_pred];
			}
		}
		if ((max + weight) > dist[next] ) {
			dist[next] = max + weight;
		}
	}
	/* All of the members of the loop have been adjusted, now
	   adjust the loop head */
	ListDigraph::InArcIt ait(*this, cfrg_node);
	ListDigraph::Node max_pred = INVALID;
	int max=0;
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node pred = source(ait);
		if (!inDerivedLoopNode(cfrg_node, pred)) {
			continue;
		}
		if (dist[pred] > max) {
			max = dist[pred];
			max_pred = pred;
		}
	}
	if (dist[cfrg_node] < (max + weight)) {
		dout << cfr_desc(*this, cfrg_node) << " max pred "
		     << cfr_desc(*this, max_pred) << " dist: " << max << endl;
		dist[cfrg_node] = max + weight;
	}
		
}
