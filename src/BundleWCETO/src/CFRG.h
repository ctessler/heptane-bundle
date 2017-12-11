#ifndef CFRG_H
#define CFRG_H

#include <limits.h>
#include <queue>

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include "BundleTypes.h"
#include "CFG.h"
#include "CFR.h"
using namespace lemon;

class CFRGNodeComp {
public:
	CFRGNodeComp(ListDigraph::NodeMap<int> &distances) : _dist(distances) {
	}
	bool operator() (const ListDigraph::Node &lhs,
			 const ListDigraph::Node &rhs) const {
		if (_dist[lhs] < _dist[rhs]) {
			/* Least first */
			return true;
		}
		return false;
	}
private:
	ListDigraph::NodeMap<int> &_dist;
};

typedef multiset<ListDigraph::Node, CFRGNodeComp> pqueue_t;

class CFRG : public ListDigraph {
public:
	CFRG(CFG &cfg) : _cfg(cfg), _gen(*this) {
		
	}
	CFG * getCFG() {
		return &_cfg;
	}
	
	/* Override to ensure the mapping is enforced */
	ListDigraph::Node addNode() {
		throw runtime_error("Cannot add a node to a CFRG without a CFR");
	}
	ListDigraph::Node addNode(CFR *cfr);
	ListDigraph::Node findNode(CFR *cfr) {
		return _from_cfr_to_node[cfr];
	}
	int getGeneration(ListDigraph::Node node) {
		return _gen[node];
	}
	CFR* findCFR(ListDigraph::Node node) {
		if (node == INVALID) {
			return NULL;
		}
		map<ListDigraph::Node, CFR*>::iterator mit =
			_from_node_to_cfr.find(node);
		if (mit != _from_node_to_cfr.end()) {
			return mit->second;
		}
		return NULL;
	}
	/* Gets and sets the inital CFR of the CFRG */
	void setInitialCFR(CFR *cfr) {
		_initial = cfr;
	}
	CFR *getInitialCFR() {
		return _initial;
	}
	ListDigraph::Node getInitial() {
		CFR *cfr = getInitialCFR();
		if (cfr == NULL) {
			throw runtime_error("No initial CFR");
		}
		return findNode(cfr);
	}
	ListDigraph::Node getTerminal() {
		for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
			ListDigraph::Node node = nit;
			if (countOutArcs(*this, node) == 0) {
				return node;
			}
		}
		return INVALID;
	}
	bool isHead(ListDigraph::Node cfrg_node);
	/**
	 * Returns true if node a and node b share the same innermost
	 * loop head. If a and b have no innermost loop head, the
	 * result is true.
	 *
	 * @return true if a and b share the same innermost loop head.
	 */
	bool sameLoopNode(ListDigraph::Node a, ListDigraph::Node b);
	bool sameLoop(CFR *a, CFR *b);
	/**
	 * Returns true if the cfr is in the loop started by the heading cfr
	 */
	bool inLoop(CFR* head, CFR* cfr);
	/**
	 * Returns true if the CFR is in any loop
	 */
	bool isLoopPart(ListDigraph::Node cfrg_node);

	/**
	 * An expensive operation, this finds the CFR based on the
	 * node from the *CFG*
	 *
	 * @param[in] node the node from the CFG that identifies a CFR
	 *
	 * @return the CFR if one exists, NULL otherwise
	 */
	CFR *findCFRbyCFGNode(ListDigraph::Node);

	/* Below here could be factored, it's about ordering and generations */
	void order();
	void dijk(ListDigraph::Node source,
		  ListDigraph::Node target,
		  ListDigraph::NodeMap<int> &distances,
		  node_map_t &prev);
	void _order(ListDigraph::Node source,
		    ListDigraph::Node target,
		    ListDigraph::NodeMap<int> &distances,
		    node_map_t &prev);
	void interiorLoopOrder(ListDigraph::Node, pqueue_t&,
			       ListDigraph::NodeMap<int>&,
			       node_map_t &);
	ListDigraph::Node smallestInLoop(ListDigraph::Node cfrg_node,
					 pqueue_t &pqueue);
	void iloSuccessorUpdate(int new_distance,
				pqueue_t &pqueue,
				ListDigraph::NodeMap<int> &distances,
				node_map_t &prev,
				set<ListDigraph::Node> &succ);
private:
	CFG &_cfg;
	CFR *_initial = NULL;
	CFR *_terminal = NULL;
	/* Stores the CFR pointer and the CFRG node with it */
	map<CFR*, ListDigraph::Node> _from_cfr_to_node;
	/* Stores the CFRG node to the CFR pointer */
	map<ListDigraph::Node, CFR*> _from_node_to_cfr;
	ListDigraph::NodeMap<int> _gen;

	void doLoopOffsets(ListDigraph::NodeMap<int> &loopOffset);

	string _indent = "";
	stack<string> _indent_stack;
	void indentInc() {
		_indent_stack.push(_indent);
		_indent = _indent + ". ";
	}
	void indentDec() {
		_indent = _indent_stack.top();
		_indent_stack.pop();
	}
		
};


#endif /* CFRG_H */

