#include "test_cfg.h"

CFG::CFG() : ListDigraph(), _function(*this)
{
	_initial = INVALID;
}

CFG::CFG(CFG &other) : ListDigraph(), _function(*this)
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

	_function[rv] = FunctionCall("UNASSIGNED", 0x0);
	
	return rv;
}

string
CFG::stringNode(ListDigraph::Node node) const {
	if (node == INVALID) {
		return "INVALID";
	}
	stringstream ss;
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

