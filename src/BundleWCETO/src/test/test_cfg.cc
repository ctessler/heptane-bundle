#include "test_cfg.h"

CFG::CFG() : ListDigraph(), _function(*this), _addr(*this), _loop_head(*this),
	     _is_loop_head(*this), _loop_iters(*this)
{
	_initial = INVALID;
}

CFG::CFG(CFG &other) : ListDigraph(), _function(*this), _addr(*this),
		       _loop_head(*this), _is_loop_head(*this),
		       _loop_iters(*this) {
	_initial = INVALID;
	
	DigraphCopy<ListDigraph, ListDigraph> dc(other, *this);
	ListDigraph::NodeMap<ListDigraph::Node> other_to_this(other);
	copyMaps(dc, other, *this);
	dc.nodeRef(other_to_this);
	dc.run();

	for (ListDigraph::NodeIt nit(other); nit != INVALID; ++nit) {
		ListDigraph::Node onode = nit;
		ListDigraph::Node tnode = other_to_this[onode];

		ListDigraph::Node ohead = other.getHead(onode);
		if (ohead != INVALID) {
			ListDigraph::Node thead = other_to_this[ohead];
			setHead(tnode, thead);
		}
	}
	ListDigraph::Node oinit = other.getInitial();
	if (oinit != INVALID) {
		ListDigraph::Node tinit = other_to_this[oinit];
		setInitial(tinit);
	}
}

std::ostream&
operator<< (std::ostream &stream, const CFG& cfg) {
	ListDigraph::Node initial = cfg.getInitial();
	stream << "("
	       << countNodes(cfg) << "v, "
	       << countArcs(cfg) << "e, "
	       << (initial == INVALID ?
		   "INVALID" : cfg.stringNode(initial))
	       << ")";

	return stream;
}


ListDigraph::Node
CFG::addNode() {
	ListDigraph::Node rv = ListDigraph::addNode();

	_addr[rv] = 0;
	_loop_head[rv] = INVALID;
	_is_loop_head[rv] = false;
	_loop_iters[rv] = 0;
	_function[rv] = FunctionCall("UNASSIGNED", 0x0);
	
	return rv;
}

string
CFG::stringNode(ListDigraph::Node node) const {
	if (node == INVALID) {
		return "INVALID";
	}
	stringstream ss;
	ss << stringAddr(node);
	if (isHead(node)) {
		ss << "[" << _loop_iters[node] << "]";
	}
	ListDigraph::Node head = getHead(node);
	ss << "("
	   << _function[node] << ", ";
	ss << "head:" << stringAddr(head)
	   << ")";
	return ss.str();
}

void
CFG::setInitial(ListDigraph::Node node) {
	_initial = node;
}
ListDigraph::Node
CFG::getInitial() const {
	return _initial;
}

ListDigraph::Node
CFG::getTerminal() const {
	return _terminal;
}

void
CFG::setTerminal(ListDigraph::Node node) {
	_terminal = node;
}

void
CFG::setFunction(ListDigraph::Node node, FunctionCall const &fcall) {
	_function[node] = fcall;
}

FunctionCall
CFG::getFunction(ListDigraph::Node node) const {
	return _function[node];
}

iaddr_t
CFG::getAddr(ListDigraph::Node node) const {
	if (!valid(node)) {
		throw runtime_error("Invalid node");
	}
	return _addr[node];
}
void
CFG::setAddr(ListDigraph::Node node, iaddr_t addr) {
	_addr[node] = addr;
}
string
CFG::stringAddr(ListDigraph::Node node) const {
	stringstream ss;
	if (node == INVALID) {
		return "INVALID";
	}
	ss << "0x" << hex << getAddr(node) << dec;
	return ss.str();
}

ListDigraph::Node
CFG::find(iaddr_t addr, FunctionCall const &fcall) {
	ListDigraph::NodeIt nit(*this);

	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		if (_addr[node] == addr &&
		    _function[node] == fcall) {
			return node;
		}
	}
	return INVALID;
}

list<ListDigraph::Node>
CFG::find(iaddr_t addr) {
	list<ListDigraph::Node> rlist;

	ListDigraph::NodeIt nit(*this);

	for ( ; nit != INVALID; ++nit) {
		ListDigraph::Node node = nit;
		if (_addr[node] == addr) {
			rlist.push_back(node);
		}
	}
	return rlist;
}


ListDigraph::Node
CFG::getHead(ListDigraph::Node node) const {
	return _loop_head[node];
}

void
CFG::setHead(ListDigraph::Node node, ListDigraph::Node head) {
	_loop_head[node] = head;
}

bool
CFG::isHead(ListDigraph::Node node) const {
	return _is_loop_head[node];
}

void
CFG::markHead(ListDigraph::Node node, bool yes) {
	_is_loop_head[node] = yes;
}

unsigned int
CFG::getIters(ListDigraph::Node head) const {
	return _loop_iters[head];
}

void
CFG::setIters(ListDigraph::Node head, unsigned int iters) {
	_loop_iters[head] = iters;
}

void
CFG::dump(string path) {
	ofstream dump(path.c_str());

	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		dump << stringNode(nit) << endl;
	}
	for (ListDigraph::ArcIt ait(*this); ait != INVALID; ++ait) {
		dump << stringNode(source(ait)) << " --> " << stringNode(target(ait)) << endl;
	}
	dump.close();
}
/**
 * CFG Private methods
 */
void
CFG::copyMaps(DigraphCopy<ListDigraph, ListDigraph> &dc, CFG &src, CFG &dst) {
	dc.nodeMap(src._function, dst._function);
	dc.nodeMap(src._addr, dst._addr);
	/* node -> node maps have to be handled specially */
	dc.nodeMap(src._is_loop_head, dst._is_loop_head);
	dc.nodeMap(src._loop_iters, dst._loop_iters);
}
