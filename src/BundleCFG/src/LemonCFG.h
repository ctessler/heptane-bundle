#ifndef LEMONCFG_H
#define LEMONCFG_H

#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/graph_to_eps.h>
#include <lemon/dijkstra.h>
#include <string>
#include <sstream>
#include <stack>
#include <stdexcept>
#include "Cache.h"
using namespace lemon;
using namespace std;

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
	 * Override the addNode() method to allow initialization
	 */
	ListDigraph::Node addNode(void);
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
	 * Assigns cache sets to individual instructions
	 *
	 * @param[in] cache the cache being used to assign sets
	 */
	void cacheAssign(Cache *cache);
	/**
	 * Gets the cache set associated with an individual instruction.
	 *
	 * @param[in] node the node of the instruction
	 */
	unsigned int cacheSet(ListDigraph::Node node);
	/**
	 * Converts a CFG to a graphvis dot file
	 *
	 * @param[in] path the full path to the dot file being written.
	 */
	void toDOT(string path);
	/**
	 * Converts a CFG to a jpeg file
	 * 
	 * @param[in] path the full path to the jpg file being written
	 */
	void toJPG(string path);
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
	 * Sets the loop head of node n to the node head, if n ==
	 * head, then bound must be > 0 
	 *
	 * @param[in] n the node being marked as the start of a loop.
	 * @param[in] exit the node immediately following n that is outside the loop.
	 */
	void setLoopHead(ListDigraph::Node n, ListDigraph::Node head);
	/**
	 * Marks a node as a loop header
	 *
	 * @param[in] n the node
	 * @param[in] yes true if it is a header, false if it is not
	 */
	void markLoopHead(ListDigraph::Node n, bool yes=true);
	/**
	 * Returns true if the node is the start of a loop
	 *
	 * @param[in] node the node being tested
	 */
	bool isLoopHead(ListDigraph::Node node);
	/**
	 * Gets the innermost loop head of the node n
	 *
	 * @param[in] n the node
	 *
	 * @return the loop head node if one exists, INVALID otherwise
	 */
	ListDigraph::Node getLoopHead(ListDigraph::Node n);
	/**
	 * Returns the bound on the number of iterations of the loop
	 *
	 * @param[in] node the loop head being tested for iterations
	 *
	 * @return >0 if the node is a loop head
	 */
	unsigned int getLoopBound(ListDigraph::Node node);
	/**
	 * Sets the loop bound on a loop head
	 *
	 * @param[in] node that is a loop head
	 * @param[in] bound the maximum iterations
	 */
	void setLoopBound(ListDigraph::Node node, unsigned int bound);


	/**
	 * Returns a list of nodes that are loop heads in "order"
	 *
	 * To be in order, no node (that is a loop head) that is contained
	 * within another loop may occur later in the list. For example:
	 * if a is a loop head that is contained within a loop starting with b,
	 *    { a, ..., b } will be true
	 *
	 * Using less than to mean that a is included in b. For the loop heads: 
	 *   a < b, c < d
	 * There is no ordering between a & c, a & d, b & c, or b & d, the
	 * resulting ordered list may look like this
	 *   { c, a, d, b }
	 */
	std::list<ListDigraph::Node> orderLoopHeads();
	/**
	 * Returns the set of conflicts starting from a specific node
	 *
	 * @param[in] root the node to start from
	 * @param[in] cache the cache to use to evaluate what
	 * conflicts are
	 *
	 * @return a map from Node to boolean values, though the
	 * boolean is superfluous. All entries (that are keys) will
	 * have a value of true in the map indicating they are indeed
	 * conflicts. A map is chosen for quick lookups on the result
	 *
	 * Cache cache;
	 * map<ListDigraph::Node, bool> xflicts;
	 * xflicts = lemonCFG.getConflicts(lemonCFG.getRoot(), &cache);
	 */
	map<ListDigraph::Node, bool>
	    getConflicts(ListDigraph::Node root, Cache *cache);

	/**
	 * Returns the set of conflict free region entry points
	 *
	 * @param[in] cache the cache used to evaluate what conflicts
	 *     exist 
	 *
	 * @return a map from Node to boolean values, though the
	 * boolean is superfluous. All entries (that are keys) will have
	 * a value of true in the map indicating they are entry points
	 * for conflict free regions.
	 */
	map<ListDigraph::Node, bool> getCFREntry(Cache *cache);


	/**
	 * Gets the CFR from a specific node
	 *
	 * @param[in] root the beginning of a CFR
	 * @param[in] cache cache used to determine the boundaries of the CFR
	 * @param[in|out] membership contains the current mapping of instruction to CFR, 
	 *    membership[a] = b means that a is in the CFR with entry point b. 
	 *    membership[a] = a means that a is the entry point of a CFR with entry point a.
	 *
	 * @return a mapping of entry points for subsequent CFRs
	 */
	map<ListDigraph::Node, bool> getConflictors(ListDigraph::Node root,
	    Cache *cache);

	/**
	 * Finds the membership of instructions to their CFR for the entire CFG
	 *
	 * @param[in] cache the cache used to determine CFR boundaries
	 *
	 * @return membership for each instruction, must be deleted by the
	 *    caller:
	 *    membership[a] = b means that a is in the CFR with entry point b.
	 *    membership[a] = a means that a is the entry point of a CFR with
	 *        entry point a. 
	 *
	 */
	void getCFRMembership(Cache *cache);

	/**
	 * Clears the CFR entries added by getCFRMembership
	 */
	void clearCFR();

	/**
	 * Gets the CFR assigned by getCFRMembership
	 */
	ListDigraph::Node getCFR(ListDigraph::Node node);

	/**
	 * Sets the CFR the node belongs to
	 *
	 * CFR memebership is typically set by getCFRMembership() but this
	 * function can be used to modify a CFR. Take special care when doing
	 * so, incorrect CFR membership may break expectations
	 */
	void setCFR(ListDigraph::Node node, ListDigraph::Node cfr);
	
	/**
	 * Returns true if the node is a CFR entry point
	 *
	 * A node may be an entry point to a CFR, but it may not
	 * identify the CFR. Meaning, a CFR has an initial instruction
	 * A, while node B may be reachable from some other path thate
	 * does not go through A. B is classified as an entry point to
	 * A, but not the initial instruction. To find the initial
	 * instruction of B call getCFR(B) to get A. The result of
	 * calling getCFR(A) will be A as well.
	 */
	bool isCFREntry(ListDigraph::Node node);
	
	/**
	 * Gives a node a specific color in the CFG when printed to
	 * DOT
	 * 
	 * @param[in] node the node being colored
	 * @param[in] color the text string of the color
	 */
	void setColor(ListDigraph::Node node, string color);

	/**
	 * Dumps the LemonCFG to a file.
	 */
	void toFile(string path);
	/**
	 * Dumps the CFR entry points to a file.
	 */
	void toCFR(string path);

	unsigned int CFRWCET(ListDigraph::Node node, unsigned int threads, Cache &cache);
private:
	/*
	 * Private Members
	 *
	 * When adding a new private member, make sure update the
	 * constructors as well as the copyMaps method if the new
	 * member is a map.
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
	/* Map from Node to Node of innermost loop head */
	ListDigraph::NodeMap<ListDigraph::Node> _loop_head;
	/* Map from Node to loop bound (only for loop_head); */
	ListDigraph::NodeMap<unsigned int> _loop_bound;
	/* True for nodes that are loop heads */
	ListDigraph::NodeMap<bool> _is_loop_head;
	/* Instruction -> Cache Set mapping */
	ListDigraph::NodeMap<unsigned int> _cache_set;
	/* Instruction -> Color */
	ListDigraph::NodeMap<string> _node_color;
	/* Instruction -> CFR entry point */
	ListDigraph::NodeMap<ListDigraph::Node> _node_cfr;
	/* Instruction -> Bool (is entry point) */
	ListDigraph::NodeMap<bool> _node_is_entry;
	/* Instruction -> Bool is the first instruction exiting a loop */
	ListDigraph::NodeMap<bool> _node_is_exit;
	/* Visited mapping for searching */
	ListDigraph::NodeMap<bool> _visited;
	
	
	/* Map from instruction address to Lemon node */
	map<unsigned long, ListDigraph::Node> _addr2node;

	/* Root node */
	ListDigraph::Node _root;

	/* Private Methods */
	void copyMaps(DigraphCopy<ListDigraph, ListDigraph> &dc,
	    LemonCFG &src, LemonCFG &dst);
	bool sameFunc(ListDigraph::Node u, ListDigraph::Node v);
	void findFollowers(ListDigraph::Node entry, stack<ListDigraph::Node> &followers);
	bool conflicts(ListDigraph::Node node, Cache *cache);
	void clearVisited();
	void markVisited(ListDigraph::Node);
	bool isVisited(ListDigraph::Node);
	map<ListDigraph::Node, bool> getConflictsIn(ListDigraph::Node u,
	    Cache *cache, ListDigraph::NodeMap<bool> &visited);
	map<ListDigraph::Node, bool> getConflictorsIn(ListDigraph::Node cfrentry,
	    ListDigraph::Node cur, Cache* cache,
	    ListDigraph::NodeMap<bool> &visited);
	void markLoopExits(ListDigraph::Node node);

	/* Display methods */
	string nodeLabel(ListDigraph::Node node);
	string nodeDOTstart(ListDigraph::Node node);
	string nodeDOTend(ListDigraph::Node node);	
	ListDigraph::Node nodeDOT(ofstream &os, ListDigraph::Node node);
	string nodeDOTrow(ListDigraph::Node node);
	string edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		       ListDigraph::Node v, ListDigraph::Node port_v);
	void loopDOT(ListDigraph::Node head, ofstream &os,
		     stack<ListDigraph::Node> &calls,
		     stack<ListDigraph::Node> &subsq,
		     ListDigraph::NodeMap<bool> &visited);

};


#endif /* LEMONCFG_H */
