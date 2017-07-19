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
	dst._addr2node = src._addr2node;
	dc.nodeMap(src._saddr, dst._saddr);
	dc.nodeMap(src._haddr, dst._haddr);
	dc.nodeMap(src._iwidth, dst._iwidth);
	dc.nodeMap(src._asm, dst._asm);
	dc.nodeMap(src._coords, dst._coords);
	dc.nodeMap(src._nsizes, dst._nsizes);
	dc.nodeMap(src._nshapes, dst._nshapes);		
	dc.arcMap(src._awidths, dst._awidths);
}

/**
 * Creates a spanning tree from src in dst
 *
 * @param[in] src the graph a spanning tree is being made from
 * @param[in|out] dst the graph that is the result of the spanning
 * tree
 * @param[out] map maps the nodes from src to those in dst 
 */
void
LemonCFG::makeSpanningTree(LemonCFG &src, LemonCFG &dst,
	ListDigraph::NodeMap<ListDigraph::Node> &map) {
	/*
	 * Copy the graph first, this creatse the node map for reference
	 * later. An arcmap won't be made because the arcs will be changed. 
	 */
	DigraphCopy<ListDigraph, ListDigraph> dc(src, dst);
	copyMaps(dc, src, dst);
	dc.nodeRef(map);
	dc.run();

	ListDigraph::Node o_root = src.getRoot();
	dst.setRoot(map[o_root]);

	/*
	 * Remove all arcs from the dst, new ones will be added as part of the 
	 * depth first search from the original.
	 */
	for (ListDigraph::ArcIt a(dst); a != INVALID; ++a) {
		dst.erase(a);
	}

	/*
	 * Perform a DFS on the src graph to create a spanning in dst
	 */
	ListDigraph::NodeMap<bool> visited(src, false);
	stack<Node> nodeStack;
	Node s = src.getRoot();
	nodeStack.push(s);
	while (!nodeStack.empty()) {
		Node p = nodeStack.top();
		cout << "mst: Visiting node " << _haddr[p] << endl;
		nodeStack.pop();
		if (visited[p]) {
			continue;
		}
		visited[p] = true;
		for (ListDigraph::OutArcIt a(src, p); a != INVALID; ++a) {
			Node o_tgt = src.runningNode(a);
			Node o_src = src.baseNode(a);
			cout << "mst: Considering arc: " << src._haddr[o_src] << " -> " << src._haddr[o_tgt] << endl;
			if (!visited[o_tgt]) {
				nodeStack.push(o_tgt);
			}
			
			/* Add the edges */
			Node c_src = map[o_src];
			Node c_tgt = map[o_tgt];
			cout << "mst: Adding arc: " << dst._haddr[c_src] << " -> " << dst._haddr[c_tgt] << endl;
			dst.addArc(c_src, c_tgt);
		}
	}
}

LemonCFG::LemonCFG() : ListDigraph(), _saddr(*this), _haddr(*this),
		       _iwidth(*this), _asm(*this), _function(*this),
		       _loop_start(*this), _loop_bound(*this),
		       _coords(*this), _awidths(*this), _nsizes(*this),
		       _nshapes(*this) {
	_root = INVALID;
}

LemonCFG::LemonCFG(LemonCFG &src) : ListDigraph(), _saddr(*this), _haddr(*this),
				    _iwidth(*this), _asm(*this), _function(*this),
				    _loop_start(*this), _loop_bound(*this),				    
				    _coords(*this), _awidths(*this),
				    _nsizes(*this), _nshapes(*this) {
	DigraphCopy<ListDigraph, ListDigraph> dc(src, *this);
	ListDigraph::NodeMap<ListDigraph::Node> map(src);
	copyMaps(dc, src, *this);
	dc.nodeRef(map);
	dc.run();

	for (ListDigraph::NodeIt n(*this); n != INVALID; ++n) {
		if (map[n] == src._root) {
			_root = n;
		}
	}
}

ListDigraph::Node
LemonCFG::addNode(void) {
	ListDigraph::Node rv = ListDigraph::addNode();
	_loop_start[rv] = false;
	_loop_bound[rv] = 0;
	return rv;
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
		+ "[label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">";

	return rv;
}

string LemonCFG::nodeDOTend(ListDigraph::Node node) {
	string rv = "\t</TABLE>> ];";
	return rv;
}

string
LemonCFG::nodeDOTrow(ListDigraph::Node node) {
	string label = nodeLabel(node);
	string text = "\t\t<TR><TD PORT=\"" + label + "\">"
		+ _haddr[node] + "</TD></TR>";

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
LemonCFG::toEPS(string path) {
	unsigned int ncount = countNodes(*this);
	if (ncount <= 0) {
		return;
	}

	/* Make a copy of this graph for a reduced presentation */
	LemonCFG copy(*this);
	cout << "Number of nodes before reduction: " << countNodes(copy) << endl;
	copy.reduceGraph();
	cout << "Number of nodes after reduction: " << countNodes(copy) << endl;

	/* Get a spanning tree of the current graph */
	LemonCFG spanTree;
	ListDigraph::NodeMap<ListDigraph::Node> nodeMap(copy);
	makeSpanningTree(copy, spanTree, nodeMap);
	cout << "Spanning Tree Root: " << spanTree._haddr[spanTree.getRoot()] << endl;
	ListDigraph::Node spanRoot = spanTree.getRoot();

	ListDigraph::NodeMap<bool> lvisit(spanTree, false);
	ListDigraph::NodeMap<int> colVal(spanTree, 0);
	
	stack<Node> nodeStack;
	nodeStack.push(spanRoot);
	int col = 0;
	while (!nodeStack.empty()) {
		Node p = nodeStack.top();
		nodeStack.pop();
		if (lvisit[p]) {
			continue;
		}
		lvisit[p] = true;
		colVal[p] = col;
		cout << "Span tree: " << hex << spanTree._haddr[p] << dec << " col: " << col << endl;
		col += L_SPACING;
		for (ListDigraph::OutArcIt a(spanTree, p); a != INVALID; ++a) {
			Node o_tgt = spanTree.runningNode(a);
			if (!lvisit[o_tgt]) {
				nodeStack.push(o_tgt);
			}
		}
	}

	ListDigraph::NodeMap<bool> rvisit(spanTree, false);
	ListDigraph::NodeMap<int> rowVal(spanTree, 0);
	nodeStack.push(spanRoot);
	int row = 0;
	while (!nodeStack.empty()) {
		Node p = nodeStack.top();
		nodeStack.pop();
		if (rvisit[p]) {
			continue;
		}
		rvisit[p] = true;
		rowVal[p] = row;
		cout << "Span tree: " << hex << spanTree._haddr[p] << " row: " << dec << row << endl;		
		row += L_SPACING;
		
		stack<Node> reverser;
		for (ListDigraph::OutArcIt a(spanTree, p); a != INVALID; ++a) {
			cout << "Adding to reverser" << endl;
			reverser.push(spanTree.runningNode(a));
		}
		while (!reverser.empty()) {
			Node t = reverser.top();
			reverser.pop();
			cout << "Reversed with: " << spanTree._haddr[p] << endl;
			if (!rvisit[t]) {
				nodeStack.push(t);
			}
		}
	}

	for (ListDigraph::NodeIt n(copy); n != INVALID; ++n) {
		Node sNode = nodeMap[n];
		int c = colVal[sNode];
		int r = rowVal[sNode];
		cout << "Node: " << _haddr[n] << " c: " << c << " r: " << r << endl;
		copy._coords[n] = Point(c, r);
		copy._nsizes[n] = L_NODE_SIZE;
		copy._nshapes[n] = L_NODE_SHAPE;
	}

	graphToEps(copy, path.c_str()).
		coords(copy._coords).
		nodeTexts(copy._haddr).
		nodeShapes(copy._nshapes).
		scale(.6).
		nodeScale(1).
		nodeTextSize(.6).
		drawArrows().
		absoluteNodeSizes().
		absoluteArcWidths().
		title("Sample .eps figure (Palette demo)").
		copyright("(C) 2003-2009 LEMON Project").
		run();
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
LemonCFG::markLoop(ListDigraph::Node node, bool yes, unsigned int bound) {
	_loop_start[node] = yes;
	_loop_bound[node] = bound;
}
bool
LemonCFG::isLoopStart(ListDigraph::Node node) {
	return _loop_start[node];
}

unsigned int
LemonCFG::loopBound(ListDigraph::Node node) {
	return _loop_bound[node];
}

