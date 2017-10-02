#include "DOTfromCFR.h"

void
DOTfromCFR::produce() {
	ofstream dot(getPath().c_str());

	dot << "digraph G {" << endl;
	ListDigraph::Node root = _cfr.getInitial();
	stack<ListDigraph::Node> calls, subsq;
	stack<string> edges;

	string pre = "DOTfromCFR::produce ";
	_debug << _indent << pre << _cfr.stringNode(root);

	calls.push(root);
	while (!calls.empty()) {
		/* New function call */
		ListDigraph::Node entry = calls.top(); calls.pop();
		FunctionCall call = _cfr.getFunction(entry);
		_debug << _indent << pre << "new call: " << call << endl;
		if (visited(entry)) {
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
			if (visited(bb_start)) {
				continue;
			}
			if (_cfr.isHead(bb_start)) {
				_debug << _indent << pre
				       << _cfr.stringNode(bb_start)
				       << " is a loop head " << endl;
				loopDOT(bb_start, dot, calls, subsq);
				continue;
			}
			visit(bb_start, true);

			/* Print this node, and all entries that are in the same BB */
			_debug << _indent << pre << "calling nodeDOT " << _cfr << endl;
			ListDigraph::Node bb_last = nodeDOT(dot, bb_start);

			/* Find the instructions immediately following the last one */
			stack<ListDigraph::Node> followers;
			succ(bb_last, followers);

			while (!followers.empty()) {
				ListDigraph::Node bb_next = followers.top(); followers.pop();
				_debug << _indent << pre << "pushing edge"
				       << _cfr.stringNode(bb_start) << " --> "
				       << _cfr.stringNode(bb_next) << endl;

				edges.push(edgeDOT(bb_start, bb_last, bb_next, bb_next));
				if (_cfr.getFunction(bb_start) != _cfr.getFunction(bb_next)) {
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
DOTfromCFR::loopDOT(ListDigraph::Node head, ofstream &os,
		    stack<ListDigraph::Node> &calls,
		    stack<ListDigraph::Node> &subsq) {
	stack<ListDigraph::Node> kids;
	stack<string> edges;

	string pre = "loopDOT ";
	if (visited(head)) {
		return;
	}

	os << "subgraph cluster_loop_" << _cfr.stringAddr(head) << "{" << endl
	   << "graph [label =\"loop [" << _cfr.getIters(head) << "]\"];" << endl;

	kids.push(head);
	while (!kids.empty()) {
		ListDigraph::Node u = kids.top(); kids.pop();
		if (visited(u)) {
			continue;
		}
		_debug << _indent << pre << "loop " << _cfr.stringAddr(head)
		       << " node " << _cfr.stringNode(u) << endl;

		if (u != head && _cfr.isHead(u)) {
			/*
			 * Case: An embedded loop starting with u
			 * If u is a loop header embedded in this
			 * loop, it's innermost loop header will be
			 * head
			 */
			loopDOT(u, os, calls, subsq);
			continue;
		}
		#if 0
		ListDigraph::Node inner_head = _cfr.toCFG(_cfr.getHead(u));
		#endif
		ListDigraph::Node inner_head = _cfr.getHead(u);
		if (u != head && inner_head != head) {
			if (inner_head != INVALID && !visited(inner_head)) {
				/*
				 * Case: A node with an innermost header that
				 * is not head. Call recursively with its head.
				 */
				loopDOT(inner_head, os, calls, subsq);
			}
			if (!visited(u)) {
				/*
				 * Case: A node reachable but outside
				 * of this loop starting with head
				 */
				subsq.push(u);
			}
			continue;
		}
		visit(u, true);

		/* Display this node */
		ListDigraph::Node bb_last = nodeDOT(os, u);

		/* Find the instructions immediately following the last one */
		stack<ListDigraph::Node> followers;
		succ(bb_last, followers);

		while (!followers.empty()) {
			ListDigraph::Node bb_next = followers.top(); followers.pop();
			_debug << _indent << pre << "loop " << _cfr.stringAddr(head)
			       << " pushing edge " << _cfr.stringNode(u) << " --> "
			       << _cfr.stringNode(bb_next) << endl;
			edges.push(edgeDOT(u, bb_last, bb_next, bb_next));
			if (_cfr.getFunction(u) != _cfr.getFunction(bb_next)) {
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
DOTfromCFR::nodeDOT(ofstream &os, ListDigraph::Node node) {
	os << nodeDOTstart(node) << endl;
	os << nodeDOTrow(node) << endl;
	ListDigraph::Node last = node;
	string pre = "nodeDOT ";

	int count = countOutArcs(_cfr, last);
	_debug << _indent << pre << _cfr.stringNode(node) << " out arcs " << count << endl;
	while (countOutArcs(_cfr, last) == 1) {
		ListDigraph::OutArcIt a(_cfr, last);
		ListDigraph::Node next = _cfr.runningNode(a);

		_debug << _indent << pre << _cfr.stringNode(last) << " --> "
		       << _cfr.stringNode(next) << " ";
		if (_cfr.getFunction(last) != _cfr.getFunction(next)) {
			/* Function call */
			_debug << "preserved function call" << endl;
			break;
		}
		if (_cfr.getAddr(next) - _cfr.getAddr(last) != 4) {
			/* Not consecutive */
			_debug << "preserved non-consecutive" << endl;
			break;
		}
		if (countInArcs(_cfr, next) > 1) {
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

string
DOTfromCFR::nodeLabel(ListDigraph::Node node) {
	stringstream label;
	label << _cfr.getFunction(node).getName() << "_0x"
	      << _cfr.getFunction(node).getCallSite()
	      << "_" << _cfr.stringAddr(node);
	return label.str();
}

string
DOTfromCFR::nodeDOTstart(ListDigraph::Node node) {
	string nlbl = nodeLabel(node);
	string rv;

	rv = "\t" + nlbl + "[shape=plaintext]\n"
		+ "\t" + nlbl
		+ "[label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n"
		+ "<TR><TD>Address</TD><TD>Cache Set</TD><TD>CFR</TD></TR>";

	return rv;
}

string
DOTfromCFR::nodeDOTend(ListDigraph::Node node) {
	string rv = "\t</TABLE>> ];";
	return rv;
}

string
DOTfromCFR::nodeDOTrow(ListDigraph::Node node) {
	string label = nodeLabel(node);
	uint32_t set = 0;
	if (getCache()) {
		set = getCache()->setIndex(_cfr.getAddr(node));
	}
	stringstream ss ;
	ss << set;

	string color = getColor(node);
	if (color.compare("") == 0) {
		color = "#FFFFFF";
	}

	string cfrs =
		_cfr.stringAddr(_cfr.membership(node));

	string text = "\t\t<TR><TD BGCOLOR=\""
		+ color + "\" PORT=\"" + label + "\">"
		+ _cfr.stringAddr(node) + "</TD>"
		+ "<TD>" + ss.str() + "</TD>"
		+ "<TD>" + cfrs + "</TD>"
		+ "</TR>";

	return text;
}

string
DOTfromCFR::edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		    ListDigraph::Node v, ListDigraph::Node port_v ) {
	string ulabel, vlabel, uplabel, vplabel;
	ulabel = nodeLabel(u);
	uplabel = nodeLabel(port_u);
	vlabel = nodeLabel(v);
	vplabel = nodeLabel(port_v);

	return ulabel + ":" + uplabel + " -> " +
		vlabel + ":" + vplabel + ";";
}
