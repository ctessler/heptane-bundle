#ifndef CFG_H
#define CFG_H

#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <string>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <fstream>
#include "BundleTypes.h"
#include "Cache.h"
#include "FunctionCall.h"
using namespace lemon;
using namespace std;

/**
 * @class CFG
 *
 * A class for representing Control Flow Graphs using the LEMON graph
 * library. Inherits from lemon::ListDigraph.
 *
 * Nodes are individual instructions
 */
class CFG : public ListDigraph {
public:
	/* Default Constructor */
	CFG();
	/* Copy constructor */
	CFG(CFG &other);
	friend std::ostream &operator<< (std::ostream &stream,
					 const CFG& cfg);
	/**
	 * Override to protect against incorrect node addition
	 */
	ListDigraph::Node addNode(void);
	string stringNode(ListDigraph::Node node) const;

	/* Gets and sets the initial (root) node of the CFG */
	ListDigraph::Node getInitial() const;
	void setInitial(ListDigraph::Node node);

	/* Gets and sets the terminal node of the CFG */
	ListDigraph::Node getTerminal() const;
	void setTerminal(ListDigraph::Node node);

	/* Gets and sets the function associated with the instruction */
	FunctionCall getFunction(ListDigraph::Node node) const;
	void setFunction(ListDigraph::Node, FunctionCall const &fcall);

	/* Gets and sets the address of the instruction */
	iaddr_t getAddr(ListDigraph::Node node) const;
	string stringAddr(ListDigraph::Node node) const;
	void setAddr(ListDigraph::Node, iaddr_t addr);

	/* Finds the instruction with the address and function */
	ListDigraph::Node find(iaddr_t addr, FunctionCall const &fcall);

	/* Finds all instructions with the address */
	list<ListDigraph::Node> find(iaddr_t addr);

private:
	ListDigraph::Node _initial, _terminal;
	
	/* Every instruction belongs to a function, functions are identified by
	   their calling address and their name */
	ListDigraph::NodeMap<FunctionCall> _function;
	/* Every instruction has a starting address */
	ListDigraph::NodeMap<iaddr_t> _addr;
	/* Some instructions have loop heads */
	ListDigraph::NodeMap<ListDigraph::Node> _loop_head;
};

#endif /* CFG_H */
