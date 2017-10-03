#include "test_cfg.h"

CFG::CFG() : ListDigraph(), _function(*this), _addr(*this), _loop_head(*this)
{
	_initial = INVALID;
}

CFG::CFG(CFG &other) : ListDigraph(), _function(*this), _addr(*this),
		       _loop_head(*this)
{
		       
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
	ss << "("
	   << _function[node] << ", ";
	ss << "head:"
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




