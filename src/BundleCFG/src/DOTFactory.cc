#include "DOTFactory.h"

void
DOTFactory::produce() {
	ofstream dot(_path.c_str());

	dot << "digraph G {" << endl;
	ListDigraph::Node root = _cfg.getInitial();
	ListDigraph::NodeMap<bool> visited(_cfg, false);
	stack<ListDigraph::Node> calls, subsq;
	stack<string> edges;

	string pre = "DOT::produce ";
	_debug << _indent << pre << _cfg.stringNode(root);

	calls.push(root);
	while (!calls.empty()) {
		/* New function call */
		ListDigraph::Node entry = calls.top(); calls.pop();
		FunctionCall call = _cfg.getFunction(entry);
		_debug << _indent << pre << "new call: " << call << endl;
		if (visited[entry]) {
			_debug << _indent << pre << "previously visited: " << call << endl;
			/* This function has been seen */
			continue;
		}
		stringstream callstream;
		callstream << call.getName() << "_0x" << hex << call.getCallSite() << dec;
		string callstr = callstream.str();
		dot << "subgraph cluster_" << callstr << " {" << endl
		    << "\tgraph [label = \"" << call << "\"];" << endl;

		subsq.push(entry);
		while (!subsq.empty()) {
			ListDigraph::Node bb_start = subsq.top(); subsq.pop();
			if (visited[bb_start]) {
				continue;
			}
			if (_cfg.isHead(bb_start)) {
				_debug << _indent << pre
				       << _cfg.stringNode(bb_start)
				       << " is a loop head " << endl;
				loopDOT(bb_start, dot, calls, subsq, visited);
				continue;
			}
			visited[bb_start] = true;

			/* Print this node, and all entries that are in the same BB */
			_debug << _indent << pre << "calling nodeDOT " << _cfg << endl;
			ListDigraph::Node bb_last = nodeDOT(dot, bb_start);

			/* Find the instructions immediately following the last one */
			stack<ListDigraph::Node> followers;
			succ(bb_last, followers);

			while (!followers.empty()) {
				ListDigraph::Node bb_next = followers.top(); followers.pop();
				_debug << _indent << pre << "pushing edge"
				       << _cfg.stringNode(bb_start) << " --> "
				       << _cfg.stringNode(bb_next) << endl;

				edges.push(edgeDOT(bb_start, bb_last, bb_next, bb_next));
				if (_cfg.getFunction(bb_start) != _cfg.getFunction(bb_next)) {
					/* next is a function entry point */
					calls.push(bb_next);
					continue;
				}
				subsq.push(bb_next);
			}
		}
		dot << "} // end function cluster_" << callstr << endl; // function cluster is done
	}

	while (!edges.empty()) {
		string edge = edges.top(); edges.pop();
		dot << edge << endl;
	}
	dot << "}" << endl;
	dot.close();
}

void
DOTFactory::loopDOT(ListDigraph::Node head, ofstream &os,
    stack<ListDigraph::Node> &calls, stack<ListDigraph::Node> &subsq,
    ListDigraph::NodeMap<bool> &visited) {
	stack<ListDigraph::Node> kids;
	stack<string> edges;

	string pre = "loopDOT ";
	if (visited[head]) {
		return;
	}

	os << "subgraph cluster_loop_" << _cfg.stringAddr(head) << "{" << endl
	   << "graph [label =\"loop [" << _cfg.getIters(head) << "]\"];" << endl;

	kids.push(head);
	while (!kids.empty()) {
		ListDigraph::Node u = kids.top(); kids.pop();
		if (visited[u]) {
			continue;
		}
		_debug << _indent << pre << "loop " << _cfg.stringAddr(head)
		       << " node " << _cfg.stringNode(u) << endl;

		if (u != head && _cfg.isHead(u)) {
			/*
			 * Case: An embedded loop starting with u
			 * If u is a loop header embedded in this
			 * loop, it's innermost loop header will be
			 * head
			 */
			loopDOT(u, os, calls, subsq, visited);
			continue;
		}
		ListDigraph::Node inner_head = _cfg.getHead(u);
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
		succ(bb_last, followers);

		while (!followers.empty()) {
			ListDigraph::Node bb_next = followers.top(); followers.pop();
			_debug << _indent << pre << "loop " << _cfg.stringAddr(head)
			       << " pushing edge " << _cfg.stringNode(u) << " --> "
			       << _cfg.stringNode(bb_next) << endl;
			edges.push(edgeDOT(u, bb_last, bb_next, bb_next));
			if (_cfg.getFunction(u) != _cfg.getFunction(bb_next)) {
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

ListDigraph::Node
DOTFactory::nodeDOT(ofstream &os, ListDigraph::Node node) {
	os << nodeDOTstart(node) << endl;
	os << nodeDOTrow(node) << endl;
	ListDigraph::Node last = node;
	string pre = "nodeDOT ";

	int count = countOutArcs(_cfg, last);
	_debug << _indent << pre << _cfg.stringNode(node) << " out arcs " << count << endl;
	while (countOutArcs(_cfg, last) == 1) {
		ListDigraph::OutArcIt a(_cfg, last);
		ListDigraph::Node next = _cfg.runningNode(a);

		_debug << _indent << pre << _cfg.stringNode(last) << " --> "
		       << _cfg.stringNode(next) << " ";
		if (_cfg.getFunction(last) != _cfg.getFunction(next)) {
			/* Function call */
			_debug << "preserved function call" << endl;
			break;
		}
		if (_cfg.getAddr(next) - _cfg.getAddr(last) != 4) {
			/* Not consecutive */
			_debug << "preserved non-consecutive" << endl;
			break;
		}
		if (countInArcs(_cfg, next) > 1) {
			/* Multiple ingressing egdes */
			_debug << "preserved next node has an incoming edge" << endl;
			break;
		}
		_debug << "contracted" << endl;
		os << nodeDOTrow(next) << endl;
		last = next;
	}

	os << nodeDOTend(node) << endl;
	return last;
}

void
DOTFactory::succ(ListDigraph::Node node, stack<ListDigraph::Node> &followers) {
	while (!followers.empty()) {
		followers.pop();
	}
	string pre = "succ ";
	_debug << _indent << pre << _cfg.stringNode(node) << " "
	       << countOutArcs(_cfg, node) << " outgoing arcs" << endl;
	for (ListDigraph::OutArcIt a(_cfg, node); a != INVALID; ++a) {
		ListDigraph::Node tgt = _cfg.runningNode(a);
		_debug << _indent << pre << "adding: "
		       << _cfg.stringNode(node) << " -> "
		       << _cfg.stringNode(tgt) << endl;
		followers.push(tgt);
	}
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
DOTFactory::edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		    ListDigraph::Node v, ListDigraph::Node port_v ) {
	string ulabel, vlabel, uplabel, vplabel;
	ulabel = nodeLabel(u);
	uplabel = nodeLabel(port_u);
	vlabel = nodeLabel(v);
	vplabel = nodeLabel(port_v);

	return ulabel + ":" + uplabel + " -> " +
		vlabel + ":" + vplabel + ";";
}

string
DOTFactory::nodeDOTstart(ListDigraph::Node node) {
	string nlbl = nodeLabel(node);
	string rv;

	rv = "\t" + nlbl + "[shape=plaintext]\n"
		+ "\t" + nlbl
		+ "[label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n"
		+ "<TR><TD>Address</TD><TD>Cache Set</TD><TD>CFR</TD></TR>";

	return rv;
}

string
DOTFactory::nodeDOTend(ListDigraph::Node node) {
	string rv = "\t</TABLE>> ];";
	return rv;
}

string
DOTFactory::nodeDOTrow(ListDigraph::Node node) {
	string label = nodeLabel(node);
	uint32_t set = 0;
	if (_cache) {
		set = _cache->setIndex(_cfg.getAddr(node));
	}
	stringstream ss ;
	ss << set;

	string color = getColor(node);
	if (color.compare("") == 0) {
		color = "#FFFFFF";
	}

	string cfrs = "nil";
	#if 0
	CFR const* cfr = dynamic_cast<CFR const*>(&_cfg);
	if (cfr != NULL) {
		cfrs = cfr->stringAddr(cfr->membership(node));
	}
	#endif

	string text = "\t\t<TR><TD BGCOLOR=\""
		+ color + "\" PORT=\"" + label + "\">"
		+ _cfg.stringAddr(node) + "</TD>"
		+ "<TD>" + ss.str() + "</TD>"
		+ "<TD>" + cfrs + "</TD>"
		+ "</TR>";

	return text;
}

string
DOTFactory::nodeLabel(ListDigraph::Node node) {
	stringstream label;
	label << _cfg.getFunction(node).getName() << "_0x"
	      << _cfg.getFunction(node).getCallSite()
	      << "_" << _cfg.stringAddr(node);
	return label.str();
}


#if 0
static string
CFRDOTid(CFR &cfr) {
	CFG *cfg = cfr.getCFG();
	ListDigraph::Node cfr_initial = cfr.getInitial();
	ListDigraph::Node cfg_node = cfr.toCFG(cfr_initial);


	stringstream id;
	FunctionCall call = cfg->getFunction(cfg_node);
	id << call.getName() << "_" << call.getCallSite() << "_" << cfg->stringAddr(cfg_node);

	return id.str();
}

static string
CFRDOT(CFR &cfr, unsigned int threads) {
	CFG *cfg = cfr.getCFG();
	ListDigraph::Node cfr_initial = cfr.getInitial();
	ListDigraph::Node cfg_node = cfr.toCFG(cfr_initial);

	stringstream label, node;
	label << "<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">" << endl << "\t"
	      << "<TR><TD COLSPAN=\"2\">CFR</TD></TR>" << endl
	      << "<TR><TD>" << cfg->stringAddr(cfg_node) << "</TD>"
	      << "<TD>" << cfg->getFunction(cfg_node) << "</TD></TR>"
	      << "<TR><TD> Threads:" << threads << "</TD>"
	      << "<TD>WCET+O: " << cfr.wcet(threads) << "</TD></TR>"
	      << "<TR><TD>isHead: " << cfr.isHead(cfr_initial) << "</TD>"
	      << "<TD>Head: " << cfg->stringNode(cfr.getHead(cfr_initial)) << "</TD></TR>"
	      << "</TABLE>>";
				    
	node << CFRDOTid(cfr) << "[shape=plaintext]" << endl
	     << CFRDOTid(cfr) << "[ label=" << label.str() << " ];" << endl;

	return node.str();
}

#endif
