#ifndef LEMONCFG_H
#define LEMONCFG_H

#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/graph_to_eps.h>
#include <string>
#include <sstream>
#include <stack>
#include <stdexcept>
using namespace lemon;
using namespace std;

/*
 * Typedefs required for maps
 */
typedef dim2::Point<int> Point;

class foo {
public:
};	

/**
 * @class LemonCFG
 *
 * A class for representing Control Flow Graphs using the LEMON graph
 * library. Inherits from lemon::ListDigraph.
 *
 * Nodes are individual instructions
 *
 * Example usage:
 *
 * LemonCFG cfg;
 *
 */
class LemonCFG : public ListDigraph {
public:
	/**
	 * Default constructior
	 */
	LemonCFG();
	/**
	 * Copy Constructor
	 */
	LemonCFG(LemonCFG &src);
	/**
	 * Sets the root of the CFG
	 * 
	 * @param[in] node the node to be the root, must be in this CFG
	 */
	void setRoot(ListDigraph::Node node);
	/**
	 * Gets the root of the CFG
	 * 
	 * @return the root node of the CFG
	 */
	ListDigraph::Node getRoot();
	/**
	 * Converts the CFG to an EPS file.
	 *
	 * @param[in] path the full path to the EPS file being
	 * written.
	 */
	void toEPS(string path);
	/**
	 * Converts a CFG to a graphvis dot file
	 *
	 * @param[in] path the full path to the dot file being written.
	 */
	void toDOT(string path);
	/**
	 * Sets the address of the instruction 
	 */
	void start(Node n, unsigned long addr);
	/**
	 * Gets the string of the starting address of the instruction
	 *
	 * @param[in] node the node that is the instruction
	 *
	 * @return the string of the starting address
	 */
	string getStartString(Node n);
	/**
	 * Gets the string of the starting address of the instruction
	 *
	 * @param[in] node the node that is the instruction
	 *
	 * @return the starting address
	 */
	unsigned long getStartLong(Node n);
	/**
	 * Finds the node with the instruction at the given starting address.
	 *
	 * @param[in] addr the starting address of the instruction
	 *
	 * @return INVALID if the node is not present, the node otherwise.
	 */
	ListDigraph::Node getNode(unsigned long addr);
	/**
	 * Sets the function containing the instruction
	 *
	 * @param[in] node the instruction
	 * @param[in] func the name of the function the instruction is in.
	 */
	void setFunc(ListDigraph::Node node, string func);
	/**
	 * Gets the function the node is contained within
	 *
	 * @param[in] node the instruction
	 *
	 * @return the function name of the node
	 */
	string getFunc(ListDigraph::Node node);
	/**
	 * Reduces a graph to consolidate the number of nodes
	 */
	void reduceGraph();
private:
	/*
	 * Private Members
	 */

	/* Address of the first byte in main memory of instruction */
	ListDigraph::NodeMap<unsigned long> _saddr;
	/* Hex text for starting addresses */
	ListDigraph::NodeMap<string> _haddr;
	/*
	 * Size of access in bytes, the total memory size of the
	 * instruction and arguments for Node n is _saddr[n] +
	 * _iwidth[n]
	 */ 
	ListDigraph::NodeMap<unsigned int> _iwidth;
	/* Assembly string of instruction */
	ListDigraph::NodeMap<string> _asm;
	/* Map from Node to containing function */
	ListDigraph::NodeMap<string> _function;
	
	/* Map from instruction address to Lemon node */
	map<unsigned long, ListDigraph::Node> _addr2node;

	/* Graphing Maps */
	/* Coordinates for plotting */
	ListDigraph::NodeMap<Point> _coords;
	/* Widths for archs */
	ListDigraph::ArcMap<double> _awidths;
	/* Node sizes */
	ListDigraph::NodeMap<double> _nsizes;
	/* Node shapes */
	ListDigraph::NodeMap<int> _nshapes;

	/* Root node */
	ListDigraph::Node _root;

	/*
	 * Private Methods
	 */
	void copyMaps(DigraphCopy<ListDigraph, ListDigraph> &dc,
	    LemonCFG &src, LemonCFG &dst);
	void makeSpanningTree(LemonCFG &src, LemonCFG &dst,
	    ListDigraph::NodeMap<ListDigraph::Node> &map);
	string nodeLabel(ListDigraph::Node node);
	string nodeDOTstart(ListDigraph::Node node);
	string nodeDOTend(ListDigraph::Node node);	
	ListDigraph::Node nodeDOT(ofstream &os, ListDigraph::Node node);
	string nodeDOTrow(ListDigraph::Node node);
	string edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		       ListDigraph::Node v, ListDigraph::Node port_v);
	bool sameFunc(ListDigraph::Node u, ListDigraph::Node v);

	void findFollowers(ListDigraph::Node entry, stack<ListDigraph::Node> &followers);
};


#endif /* LEMONCFG_H */
