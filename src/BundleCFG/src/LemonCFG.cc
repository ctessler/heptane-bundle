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
	dst._addr2node = src._addr2node;
}

LemonCFG::LemonCFG() : ListDigraph(), _saddr(*this), _haddr(*this),
		       _iwidth(*this), _asm(*this), _function(*this),
		       _loop_head(*this), _loop_bound(*this), _is_loop_head(*this),
		       _cache_set(*this) {
	_root = INVALID;
}

LemonCFG::LemonCFG(LemonCFG &src) : ListDigraph(), _saddr(*this), _haddr(*this),
				    _iwidth(*this), _asm(*this), _function(*this),
				    _loop_head(*this), _loop_bound(*this), _is_loop_head(*this),
				    _cache_set(*this) {
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

	cout << "toDOT begins with: " << getStartString(root) << endl;

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
				cout << "toDOT " << getStartString(bb_start) << " is a loop head" << endl;
				loopDOT(bb_start, os, calls, subsq, visited);
				continue;
			}
			visited[bb_start] = true;

			/* Print this node, and all entries that are in the same BB */
			cout << "toDOT Calling nodeDOT with " << getStartString(bb_start) << endl;
			ListDigraph::Node bb_last = nodeDOT(os, bb_start);

			/* Find the instructions immediately following the last one */
			stack<ListDigraph::Node> followers;
			findFollowers(bb_last, followers);

			while (!followers.empty()) {
				ListDigraph::Node bb_next = followers.top(); followers.pop();
				cout << "Pushing edge " << edgeDOT(bb_start, bb_last, bb_next, bb_next) << endl;
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

	int count = countOutArcs(*this, last);
	cout << "nodeDOT " << getStartString(node) << " out arcs: " << count << endl;

	while (countOutArcs(*this, last) == 1) {
		ListDigraph::OutArcIt a(*this, last);
		ListDigraph::Node next = runningNode(a);
		cout << "node DOT " << getStartString(last) << " -> " << getStartString(next) << " ";
		if (!sameFunc(last, next)) {
			/* Function call */
			cout << "preserved function call" << endl;
			break;
		}
		if ( _saddr[next] - _saddr[last] != 4) {
			/* Not consecutive */
			cout << "preserved non-consecutive" << endl;
			break;
		}
		if (countInArcs(*this, next) > 1) {
			/* Multiple ingressing egdes */
			cout << "preserved next node has an incoming edge" << endl;
			break;
		}
		cout << "contracted" << endl;
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
		+ "<TR><TD>Address</TD><TD>Cache Set</TD></TR>";

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
	string text = "\t\t<TR><TD PORT=\"" + label + "\">"
		+ _haddr[node] + "</TD>"
		+ "<TD>" + ss.str() + "</TD>"
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
		cout << "loopDOT working loop " << getStartString(head)
		     <<	" node: " << getStartString(u) << endl;

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
			cout << "loopDOT Pushing edge " << edgeDOT(u, bb_last, bb_next, bb_next) << endl;
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
	cout << "findFollowers: " << getStartString(node) << " "
	     << countOutArcs(*this, node) << " outgoing arcs" << endl;
	for (ListDigraph::OutArcIt a(*this, node); a != INVALID; ++a) {
		ListDigraph::Node tgt = runningNode(a);
		cout << "findFollowers adding: " << getStartString(node) << " -> "
		     << getStartString(tgt) << endl;
			
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



static bool
contractOne(LemonCFG &cfg, ListDigraph::NodeMap<unsigned long> &coverage,
	    ListDigraph::Node node) {
	if (countOutArcs(cfg, node) != 1) {
		/*
		 * If this node has zero or >1 edges, it cannot be
		 * reduced with it's subsequent nodes
		 */
		cout << "Skipping (out arc != 1) : " << cfg.getStartString(node) << endl;
		return false;
	}
	/* There can be only one */
	ListDigraph::OutArcIt a(cfg, node);
	ListDigraph::Node src = cfg.baseNode(a);
	ListDigraph::Node tgt = cfg.runningNode(a);
	string src_s = cfg.getStartString(src);
	string tgt_s = cfg.getStartString(tgt);
	string edge = src_s + " -> " + tgt_s;
	unsigned long tgt_i;
	tgt_i = cfg.getStartLong(tgt);

	cout << "Considering Contracting: " << edge << endl;

	if (tgt_i - coverage[src] != 4) {
		cout << "Not contracting (distance) : " << edge << endl;
		return false;
	}

	if (countInArcs(cfg, tgt) != 1) {
		cout << "Not contracting (in arc) : " << edge << endl;
		return false;
	}

	cout << "Contracting : " << edge;
	cfg.contract(src, tgt);
	coverage[src] = tgt_i;

	return true;
}

void
LemonCFG::reduceGraph() {
	ListDigraph::Node root = getRoot();
	cout << "My root: " << _haddr[root] << endl;

	/* Node coverage */
	ListDigraph::NodeMap<unsigned long> coverage(*this, 0);

	for (ListDigraph::NodeIt n(*this); n != INVALID; ++n) {
		bool changes;
		do {
			cout << "Checking node: " << getStartString(n) << endl;
			if (coverage[n] == 0) {
				coverage[n] = _saddr[n];
			}
			changes = contractOne(*this, coverage, n);
			if (changes) {
				cout << "  contracted" << endl;
			}
		} while (changes);
	}
	return;
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
		cout << "cacheAssign " << getStartString(node)
		     << " maps to cache set " << setIndex << endl;
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

