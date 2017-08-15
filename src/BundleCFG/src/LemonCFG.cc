#include "LemonCFG.h"

#define L_NODE_SIZE .1
#define L_NODE_SHAPE 1 /* SQUARE */
#define L_TEXT_SIZE 10
#define L_ARROW_WIDTH .1
#define L_ARROW_LENGTH .2
#define L_SPACING 1

/**
 * Copies the maps from the src to the destination.
 *
 * When making a copy of a LemonCFG, the node and arc maps need to be
 * copied to the new object.
 */
void
LemonCFG::copyMaps(DigraphCopy<ListDigraph, ListDigraph> &dc,
		       LemonCFG &src, LemonCFG &dst) {
	dc.nodeMap(src._saddr, dst._saddr);
	dc.nodeMap(src._haddr, dst._haddr);
	dc.nodeMap(src._iwidth, dst._iwidth);
	dc.nodeMap(src._asm, dst._asm);
	dc.nodeMap(src._function, dst._function);
	dc.nodeMap(src._loop_head, dst._loop_head);
	dc.nodeMap(src._loop_bound, dst._loop_bound);
	dc.nodeMap(src._is_loop_head, dst._is_loop_head);
	dc.nodeMap(src._cache_set, dst._cache_set);
	dc.nodeMap(src._node_color, dst._node_color);
	dc.nodeMap(src._node_cfr, dst._node_cfr);	
	dst._addr2node = src._addr2node;
}

LemonCFG::LemonCFG() : ListDigraph(), _saddr(*this), _haddr(*this),
		       _iwidth(*this), _asm(*this), _function(*this),
		       _loop_head(*this), _loop_bound(*this), _is_loop_head(*this),
		       _cache_set(*this), _node_color(*this), _node_cfr(*this) {
	_root = INVALID;
}

LemonCFG::LemonCFG(LemonCFG &src) : ListDigraph(), _saddr(*this), _haddr(*this),
				    _iwidth(*this), _asm(*this), _function(*this),
				    _loop_head(*this), _loop_bound(*this), _is_loop_head(*this),
				    _cache_set(*this), _node_color(*this), _node_cfr(*this) {
	DigraphCopy<ListDigraph, ListDigraph> dc(src, *this);
	ListDigraph::NodeMap<ListDigraph::Node> map(src);
	copyMaps(dc, src, *this);
	dc.nodeRef(map);
	dc.run();

	ListDigraph::Node o_root = src.getRoot();
	setRoot(map[o_root]);
}

ListDigraph::Node
LemonCFG::addNode(void) {
	ListDigraph::Node rv = ListDigraph::addNode();
	_loop_head[rv] = INVALID;
	_loop_bound[rv] = 0;
	_is_loop_head[rv] = false;
	_cache_set[rv] = 0;
	_node_cfr[rv] = INVALID;
	return rv;
}

void LemonCFG::toJPG(string path) {
	string temp_path = path + ".dot";

	LemonCFG::toDOT(temp_path);
	string command = "dot -Tjpg " + temp_path + " > " + path;
	system(command.c_str());
	command = "rm " + temp_path;
	system(command.c_str());
}

void
LemonCFG::toDOT(string path) {
	ofstream os (path.c_str());
	os << "digraph G {" << endl;
	ListDigraph::Node root = getRoot();
	ListDigraph::NodeMap<bool> visited(*this, false);
	stack<ListDigraph::Node> calls, subsq;
	stack<string> edges;
#ifdef DBG_TODOT
	cout << "toDOT begins with: " << getStartString(root) << endl;
#endif /* DBG_TODOT */
	calls.push(root);
	while (!calls.empty()) {
		/* New function call */
		ListDigraph::Node entry = calls.top(); calls.pop();
		if (visited[entry]) {
			/* This function has been seen */
			continue;
		}
		string fname = _function[entry];
		os << "subgraph cluster_" << fname << " {" << endl
		   << "\tgraph [label = \"" << fname << "\"];" << endl;

		subsq.push(entry);
		while (!subsq.empty()) {
			ListDigraph::Node bb_start = subsq.top(); subsq.pop();
			if (visited[bb_start]) {
				continue;
			}
			if (isLoopHead(bb_start)) {
#ifdef DBG_TODOT
				cout << "toDOT " << getStartString(bb_start) << " is a loop head" << endl;
#endif /* DBG_TODOT */
				loopDOT(bb_start, os, calls, subsq, visited);
				continue;
			}
			visited[bb_start] = true;

			/* Print this node, and all entries that are in the same BB */
#ifdef DBG_TODOT
			cout << "toDOT Calling nodeDOT with " << getStartString(bb_start) << endl;
#endif /* DBG_TODOT */
			ListDigraph::Node bb_last = nodeDOT(os, bb_start);

			/* Find the instructions immediately following the last one */
			stack<ListDigraph::Node> followers;
			findFollowers(bb_last, followers);

			while (!followers.empty()) {
				ListDigraph::Node bb_next = followers.top(); followers.pop();
#ifdef DBG_TODOT
				cout << "toDOT Pushing edge " << edgeDOT(bb_start,
				    bb_last, bb_next, bb_next) << endl;
#endif /* DBG_TODOT */
				edges.push(edgeDOT(bb_start, bb_last, bb_next, bb_next));
				if (!sameFunc(bb_start, bb_next)) {
					/* next is a function entry point */
					calls.push(bb_next);
					continue;
				}
				subsq.push(bb_next);
			}
		}
		os << "}" << endl; // function cluster is done
	}

	while (!edges.empty()) {
		string edge = edges.top(); edges.pop();
		os << edge << endl;
	}
	os << "}" << endl;
	os.close();
}

string
LemonCFG::nodeLabel(ListDigraph::Node node) {
	string label = "i" + _haddr[node];
	return label;
}

ListDigraph::Node
LemonCFG::nodeDOT(ofstream &os, ListDigraph::Node node) {
	os << nodeDOTstart(node) << endl;
	os << nodeDOTrow(node) << endl;
	ListDigraph::Node last = node;

#ifdef DBG_NODEDOT
	int count = countOutArcs(*this, last);
	cout << "nodeDOT " << getStartString(node) << " out arcs: " << count << endl;
#endif /* DBG_NODEDOT */
	while (countOutArcs(*this, last) == 1) {
		ListDigraph::OutArcIt a(*this, last);
		ListDigraph::Node next = runningNode(a);
#ifdef DBG_NODEDOT
		cout << "node DOT " << getStartString(last) << " -> " << getStartString(next) << " ";
#endif /* DBG_NODEDOT */
		if (!sameFunc(last, next)) {
			/* Function call */
#ifdef DBG_NODEDOT
			cout << "preserved function call" << endl;
#endif /* DBG_NODEDOT */
			break;
		}
		if ( _saddr[next] - _saddr[last] != 4) {
			/* Not consecutive */
#ifdef DBG_NODEDOT
			cout << "preserved non-consecutive" << endl;
#endif /* DBG_NODEDOT */
			break;
		}
		if (countInArcs(*this, next) > 1) {
			/* Multiple ingressing egdes */
#ifdef DBG_NODEDOT
			cout << "preserved next node has an incoming edge" << endl;
#endif /* DBG_NODEDOT */
			break;
		}
#ifdef DBG_NODEDOT
		cout << "contracted" << endl;
#endif /* DBG_NODEDOT */
		os << nodeDOTrow(next) << endl;
		last = next;
	}

	os << nodeDOTend(node) << endl;
	return last;
}


string
LemonCFG::nodeDOTstart(ListDigraph::Node node) {
	string nlbl = nodeLabel(node);
	string rv;

	rv = "\t" + nlbl + "[shape=plaintext]\n"
		+ "\t" + nlbl
		+ "[label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n"
		+ "<TR><TD>Address</TD><TD>Cache Set</TD><TD>CFR</TD></TR>";

	return rv;
}

string LemonCFG::nodeDOTend(ListDigraph::Node node) {
	string rv = "\t</TABLE>> ];";
	return rv;
}

string
LemonCFG::nodeDOTrow(ListDigraph::Node node) {
	string label = nodeLabel(node);
	uint32_t set = cacheSet(node);
	stringstream ss ;
	ss << set;

	string color = "#FFFFFF";
	if (_node_color[node].compare("") != 0) {
		color = _node_color[node];
	}


	string text = "\t\t<TR><TD BGCOLOR=\""
		+ color + "\" PORT=\"" + label + "\">"
		+ _haddr[node] + "</TD>"
		+ "<TD>" + ss.str() + "</TD>"
		+ "<TD>" + getStartString(_node_cfr[node]) + "</TD>"
		+ "</TR>";

	return text;
}

/**
 * Produces the DOT of an edge between nodes
 *
 * Since nodes can be contracted, the addresses are identified by their ports.
 *
 * @param[in] u the starting vertex of the edge
 * @param[in] port_u the port within the vertex to start the edge from
 * @param[in] v the ending vertex of the edge
 * @param[in] port_v the port within the vertex to end the edge
 *
 */
string
LemonCFG::edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		  ListDigraph::Node v, ListDigraph::Node port_v ) {
	string ulabel, vlabel, uplabel, vplabel;
	ulabel = nodeLabel(u);
	uplabel = nodeLabel(port_u);
	vlabel = nodeLabel(v);
	vplabel = nodeLabel(port_v);

	return ulabel + ":" + uplabel + " -> " +
		vlabel + ":" + vplabel + ";";
}

void
LemonCFG::loopDOT(ListDigraph::Node head, ofstream &os,
    stack<ListDigraph::Node> &calls, stack<ListDigraph::Node> &subsq,
    ListDigraph::NodeMap<bool> &visited) {
	stack<ListDigraph::Node> kids;
	stack<string> edges;

	if (visited[head]) {
		return;
	}

	os << "subgraph cluster_loop_" << getStartString(head) << "{" << endl
	   << "graph [label =\"loop [" << getLoopBound(head) << "]\"];" << endl;

	kids.push(head);
	while (!kids.empty()) {
		ListDigraph::Node u = kids.top(); kids.pop();
		if (visited[u]) {
			continue;
		}
#ifdef DBG_LOOPDOT
		cout << "loopDOT working loop " << getStartString(head)
		     <<	" node: " << getStartString(u) << endl;
#endif /* DBG_LOOPDOT */
		if (u != head && isLoopHead(u)) {
			/*
			 * Case: An embedded loop starting with u
			 * If u is a loop header embedded in this
			 * loop, it's innermost loop header will be
			 * head
			 */
			loopDOT(u, os, calls, subsq, visited);
			continue;
		}
		ListDigraph::Node inner_head = getLoopHead(u);
		if (u != head && inner_head != head) {
			if (inner_head != INVALID && !visited[inner_head]) {
				/*
				 * Case: A node with an innermost header that
				 * is not head. Call recursively with its head.
				 */
				loopDOT(inner_head, os, calls, subsq, visited);
			}
			if (!visited[u]) {
				/*
				 * Case: A node reachable but outside
				 * of this loop starting with head
				 */
				subsq.push(u);
			}
			continue;
		}
		visited[u] = true;

		/* Display this node */
		ListDigraph::Node bb_last = nodeDOT(os, u);

		/* Find the instructions immediately following the last one */
		stack<ListDigraph::Node> followers;
		findFollowers(bb_last, followers);

		while (!followers.empty()) {
			ListDigraph::Node bb_next = followers.top(); followers.pop();
#ifdef DBG_LOOPDOT
			cout << "loopDOT Pushing edge " << edgeDOT(u, bb_last, bb_next, bb_next) << endl;
#endif /* DBG_LOOPDOT */
			edges.push(edgeDOT(u, bb_last, bb_next, bb_next));
			if (!sameFunc(u, bb_next)) {
				/* next is a function entry point */
				calls.push(bb_next);
				continue;
			}
			kids.push(bb_next);
		}
	}
	os << "}" << endl;
	while (!edges.empty()) {
		string edge = edges.top(); edges.pop();
		os << edge << endl;
	}
}

void
LemonCFG::findFollowers(ListDigraph::Node node, stack<ListDigraph::Node> &followers) {
	while (!followers.empty()) {
		followers.pop();
	}
#ifdef DBG_FINDFOLLOWERS
	cout << "findFollowers: " << getStartString(node) << " "
	     << countOutArcs(*this, node) << " outgoing arcs" << endl;
#endif /* DBG_FINDFOLLOWERS */
	for (ListDigraph::OutArcIt a(*this, node); a != INVALID; ++a) {
		ListDigraph::Node tgt = runningNode(a);
#ifdef DBG_FINDFOLLOWERS
		cout << "findFollowers adding: " << getStartString(node) << " -> "
		     << getStartString(tgt) << endl;
#endif /* DBG_FINDFOLLOWERS */
		followers.push(tgt);
	}
}

/**
 * Returns true if the nodes are in the same function
 */
bool
LemonCFG::sameFunc(ListDigraph::Node u, ListDigraph::Node v) {
	bool x = (0 == _function[u].compare(_function[v]));
	return x;
}

void
LemonCFG::start(Node n, unsigned long addr) {
	_saddr[n] = addr;
	stringstream ss;
	ss << "0x" << hex << addr;
	_haddr[n] = ss.str();

	if (getNode(addr) != INVALID) {
		throw runtime_error("Adding the same address twice!");
	}
	_addr2node[addr] = n;
}

string
LemonCFG::getStartString(Node n) {
	if (n == INVALID) {
		return "INVALID";
	}
	return _haddr[n];
}

unsigned long
LemonCFG::getStartLong(Node n) {
	return _saddr[n];
}

ListDigraph::Node
LemonCFG::getNode(unsigned long addr) {
	map<unsigned long, ListDigraph::Node>::iterator it;

	it = _addr2node.find(addr);
	if (it == _addr2node.end()) {
		return INVALID;
	}
	return it->second;
}
void
LemonCFG::setFunc(ListDigraph::Node node, string func) {
	_function[node] = func;
}

string
LemonCFG::getFunc(ListDigraph::Node node) {
	string fname = _function[node];
	return fname;
}

void
LemonCFG::setRoot(ListDigraph::Node node) {
	_root = node;
}

ListDigraph::Node
LemonCFG::getRoot() {
	return _root;
}

void
LemonCFG::cacheAssign(Cache *cache) {
	for (ListDigraph::NodeIt n(*this); n != INVALID; ++n) {
		ListDigraph::Node node = nodeFromId(id(n));
		uint32_t setIndex = cache->setIndex(getStartLong(node));
		_cache_set[node] = setIndex;
#ifdef DBG_CACHEASSIGN		
		cout << "cacheAssign " << getStartString(node)
		     << " maps to cache set " << setIndex << endl;
#endif /* DBG_CACHEASSIGN */
	}
}



unsigned int
LemonCFG::cacheSet(ListDigraph::Node node) {
	return _cache_set[node];
}


void
LemonCFG::setLoopHead(ListDigraph::Node n, ListDigraph::Node head) {
	_loop_head[n] = head;
}

ListDigraph::Node
LemonCFG::getLoopHead(ListDigraph::Node n) {
	return _loop_head[n];
}


unsigned int
LemonCFG::getLoopBound(ListDigraph::Node node) {
	return _loop_bound[node];
}

void
LemonCFG::setLoopBound(ListDigraph::Node node, unsigned int bound) {
	_loop_bound[node] = bound;
}

void
LemonCFG::markLoopHead(ListDigraph::Node node, bool yes) {
	_is_loop_head[node] = yes;
}

bool
LemonCFG::isLoopHead(ListDigraph::Node node) {
	return _is_loop_head[node];
}

bool
LemonCFG::conflicts(ListDigraph::Node node, Cache *cache) {
	unsigned long addr = _saddr[node];
	CacheSet *cs = cache->setOf(addr);

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

map<ListDigraph::Node, bool>
LemonCFG::getConflictsIn(ListDigraph::Node u, Cache *cache,
    ListDigraph::NodeMap<bool> &visited) {
	/* Caller must ensure getConflictsIn is called with unvisited nodes */
	visited[u] = true;

	map<ListDigraph::Node, bool> xflicts;
	unsigned long addr = _saddr[u];

	/*
	 * If this node would cause a conflict, stop add it to the list
	 * and abort
	 */
	if (conflicts(u, cache)) {
#ifdef DBG_GETCONFLICTSIN
		cout << "getConflictsIn: " << getStartString(u) << " conflicts" << endl;
		cout << "getConflictsIn: Cache Set Index[" << cache->setIndex(addr) << "] "
		     << "contents:" << endl;
		CacheSet *cs = cache->setOf(addr);
		for (uint32_t way=0; way <= cs->ways(); way++) {
			CacheLine *cl = cs->cacheLine(way);
			if (!cl) {
				break;
			}
			cout << "\tWay[" << way << "] Range 0x"
			     <<	hex << cl->getStartAddress() << " -> 0x" << cl->getEndAddress() << dec << endl;
		}
#endif /* DBG_GETCONFLICTSIN */		
		xflicts[u] = true;
		return xflicts;
	}

	/*
	 * If this node does not cause a conflict, process it and its
	 * children.
	 * Processing means:
	 *   Copy the cache as it is
	 *   Insert this node into the new cache copy
	 *   Visit children recursively
	 */
	Cache *cacheCopy = new Cache(*cache);
	cacheCopy->insert(addr);

	stack<ListDigraph::Node> kids;
	findFollowers(u, kids);
	while (!kids.empty()) {
		ListDigraph::Node v = kids.top(); kids.pop();
		if (visited[v]) {
			continue;
		}
		/* Recursive call */
		map<ListDigraph::Node, bool> newflicts =
			getConflictsIn(v, cacheCopy, visited);

		/* Merge the conflicts */
		for (map<ListDigraph::Node, bool>::iterator it = newflicts.begin();
		     it != newflicts.end(); it++) {
			if (it->second != true) {
				/* BIG PROBLEM! */
				throw runtime_error("Unexpected false value");
			}
			xflicts[it->first] = it->second;
		}
	}
	delete(cacheCopy);

	return xflicts;
}

/*
 * This needs to be a recursive DFS, or there will be problems with the cache
 * state at each node.
 */
map<ListDigraph::Node, bool>
LemonCFG::getConflicts(ListDigraph::Node root, Cache *cache) {
	ListDigraph::NodeMap<bool> visited(*this);
	map<ListDigraph::Node, bool> xflicts =
	    getConflictsIn(root, cache, visited);

	return xflicts;
}

map<ListDigraph::Node, bool>
LemonCFG::getCFREntry(Cache *cache) {
	ListDigraph::NodeMap<bool> procd(*this);
	stack<ListDigraph::Node> waiting;
	map<ListDigraph::Node, bool> result;

	ListDigraph::Node cur = getRoot();
	waiting.push(cur);
	result[cur] = true;
	while (!waiting.empty()) {
		cur = waiting.top(); waiting.pop();
		procd[cur] = true;

		/* Get the conflicts from *this* instruction */
		Cache *copy = new Cache(*cache);
		map<ListDigraph::Node, bool> xflicts =
			getConflicts(cur, copy);
		delete(copy);

		/* Add the conflicts to the waiting stack *only* if they have
		   not been processed */
#ifdef DBG_GETCFGENTRY
		cout << "getCFGEntry " << getStartString(cur) << " has conflicts: " << endl;
#endif /* DBG_GETCFGENTRY */
		for (map<ListDigraph::Node, bool>::iterator it = xflicts.begin();
		     it != xflicts.end(); it++) {
#ifdef DBG_GETCFGENTRY
			cout << "\t" << getStartString(it->first);
#endif /* DBG_GETCFGENTRY */
			if (procd[it->first]) {
#ifdef DBG_GETCFGENTRY
				cout << endl;
#endif /* DBG_GETCFGENTRY */

				continue;
			}
#ifdef DBG_GETCFGENTRY
			cout << " starts a new CFR" << endl;
#endif /* DBG_GETCFGENTRY */
			waiting.push(it->first);
			result[it->first] = true;
		}

	}
	return result;
}

void
LemonCFG::setColor(ListDigraph::Node node, string color) {
	_node_color[node] = color;
}


map<ListDigraph::Node, bool>
LemonCFG::getConflictorsIn(ListDigraph::Node cfrentry,
    ListDigraph::Node cur, Cache* cache,
    ListDigraph::NodeMap<ListDigraph::Node> &cfr,
    ListDigraph::NodeMap<bool> &visited) {
	/* CFR Entry points found from *this* node to continue the search */
	map<ListDigraph::Node, bool> entry;
	
	/* Caller must ensure this node (u) has not been visited */
	visited[cur] = true;
	unsigned long addr = _saddr[cur];

	/*
	 * If this node would cause a conflict, it belongs to
	 * a different conflict free region
	 *
	 * If it has not been assigned to a previous conflict
	 * free region, it will become the entry point for a
	 * new one. This is indicated by setting the cfr of
	 * the current node to itself.
	 */
	if (conflicts(cur, cache)) {
		cout << "getConflictorsIn: " << getStartString(cfrentry)
		     << " -x-> " << getStartString(cur) << endl;
		if (cfr[cur] == INVALID) {
			cout << "getConflictorsIn: " << getStartString(cur)
			     << " begins a new CFR " << endl;
			/* cur already has an existing CFR */
			cfr[cur] = cur;
			entry[cfr[cur]] = true;
		}
		/* A new CFR */
		return entry;
	}
	/* Does not conflict, assign cur to the CFR */
	cfr[cur] = cfrentry;
	cache->insert(addr);

	for (ListDigraph::OutArcIt a(*this, cur); a != INVALID; ++a) {
		ListDigraph::Node tgt = runningNode(a);
		if (visited[tgt]) {
			continue;
		}
		/* Recursive Call */
		map<ListDigraph::Node, bool> lower;
		lower = getConflictorsIn(cfrentry, tgt, cache, cfr, visited);

		/* Join result */
		map<ListDigraph::Node, bool>::iterator it;
		for (it = lower.begin(); it != lower.end(); it++) {
			entry[it->first] = true;
		}
	}
	return entry;
}

map<ListDigraph::Node, bool>
LemonCFG::getConflictors(ListDigraph::Node root, Cache *cache,
    ListDigraph::NodeMap<ListDigraph::Node> &membership) {
	ListDigraph::NodeMap<bool> visited (*this);
	map<ListDigraph::Node, bool> entry;

	Cache *copy = new Cache(*cache);
	entry = getConflictorsIn(root, root, copy, membership, visited);
	delete(copy);

	map<ListDigraph::Node, bool>::iterator it = entry.begin();
	cout << "getConflictors entry points for " << getStartString(root) << endl;
	for ( ; it != entry.end(); it++) {
		cout << "\t" << getStartString(it->first) << endl;
	}
	
	return entry;
}

ListDigraph::NodeMap<ListDigraph::Node>*
LemonCFG::getCFRMembership(Cache *cache) {
	ListDigraph::NodeMap<ListDigraph::Node> *membership =
		new ListDigraph::NodeMap<ListDigraph::Node>(*this);

	ListDigraph::NodeMap<bool> visited(*this);
	

	ListDigraph::Node cur = getRoot();
	map<ListDigraph::Node, bool> entries;
	entries = getConflictors(cur, cache, *membership);
	while (entries.size() != 0) {
		map<ListDigraph::Node, bool>::iterator it = entries.begin();
		cur = it->first;
		entries.erase(cur);
		map<ListDigraph::Node, bool> nexts = getConflictors(cur, cache, *membership);
		for (it = nexts.begin(); it != nexts.end(); it++) {
			entries[it->first] = it->second;
		}
		
	}

	/*
	 * Set the CFR membership while we're here, just because it's
	 * easy. Allow the caller to clear with LemonCFG::clearCFR
	 */
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		ListDigraph::Node node = nodeFromId(id(nit));
		ListDigraph::Node cfr = membership->operator[](node);
		_node_cfr[node] = cfr;
	}
	
	return membership;
}

void
LemonCFG::clearCFR() {
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		_node_cfr[nit] = INVALID;
	}
}

ListDigraph::Node
LemonCFG::getCFR(ListDigraph::Node node) {
	return _node_cfr[node];
}

