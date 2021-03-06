#ifndef CFRG_H
#define CFRG_H

#include <limits.h>
#include <queue>

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include "BundleTypes.h"
#include "CFG.h"
#include "CFR.h"
#include "PQueue.h"
using namespace lemon;

typedef list<CFR*> CFRList;
class CFRG : public ListDigraph {
public:
	CFRG(CFG &cfg) : _cfg(cfg), _gen(*this) {
		ord.open("order.log");
		ilo.open("ilo.log");
		sil.open("sil.log");
	}
	~CFRG() {
		ord.close();
		sil.close();
		ilo.close();
	}
	CFG * getCFG() {
		return &_cfg;
	}
	
	/* Override to ensure the mapping is enforced */
	ListDigraph::Node addNode() {
		throw runtime_error("Cannot add a node to a CFRG without a CFR");
	}
	ListDigraph::Node addNode(CFR *cfr);
	ListDigraph::Node findNode(CFR *cfr);
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
	void setInitialCFR(CFR *cfr);
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
	bool isHeadCFR(CFR* cfr);
	bool isTopHead(CFR* cfr);
	bool isTopHeadNode(ListDigraph::Node cfrg_node);
	/**
	 * Gets the CFRG node which contains the loop head of the given CFR
	 */
	CFR* getHead(CFR* cfr);
	ListDigraph::Node getHeadNode(ListDigraph::Node cfrg_node);

	/**
	 * Returns true if the CFR is preceded by a CFR that is part
	 * of a loop.
	 */
	bool isLoopExit(ListDigraph::Node cfrg_node);
	bool isLoopExitCFR(CFR *cfr);
	
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
	 *
	 * Also returns true if head == cfr
	 */
	bool inLoop(CFR* head, CFR* cfr);
	bool inLoopNode(ListDigraph::Node cfrg_head_node,
			ListDigraph::Node cfrg_node);

	/**
	 * Returns true if the cfr is contained within any loop within
	 * the loop of the head 
	 */	
	bool inDerivedLoop(CFR* head, CFR* cfr);
	bool inDerivedLoopNode(ListDigraph::Node cfrg_head_node,
			       ListDigraph::Node cfrg_node);
	
	/**
	 * Returns true if the CFR is in any loop
	 */
	bool isLoopPart(ListDigraph::Node cfrg_node);
	bool isLoopPartCFR(CFR* cfr);
	/**
	 * Returns the head of the given CFR that is top most, ie the
	 * head that is not a part of any other loop besides the one
	 * it starts.
	 */
	CFR* crown(CFR* cfr);

	/**
	 * Returns a list of CFRs that immediately precede the given
	 * CFR in the CFRG
	 *
	 * Caller must delete the returned list
	 */
	CFRList* preds(CFR* cfr);
	/**
	 * Returns a list of CFRS that immediately succeed the given
	 * CFR in the CFRG
	 *
	 * Caller must delete the returned list
	 */
	CFRList* succs(CFR* cfr);

	/**
	 * Returns a list of CFRs that are in the loop of the given
	 * CFR @param{cfr} (which is a loop head)
	 *
	 * If @param{cfr} contains an embedded loop, cfr_b, cfr_b will
	 * be included in the return list. It is the callers
	 * responsibility to call isHeadCFR(cfr_b) before handling
	 * cfr_b
	 *
	 * Caller must delete the returned list
	 */
	CFRList* inLoopOfCFR(CFR *cfr);
	
	/**
	 * Returns a list of CFRs that are one-hop outside of the
	 * loop of the loop head CFR @param{cfr}
	 *
	 * If @param{cfr} contains an embedded loop it is not
	 * considered an exit.
	 *
	 * Caller must delete the returned list
	 */
	CFRList* exitOfCFR(CFR *cfr);

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
		    ListDigraph::NodeMap<int> &distances,
		    node_map_t &prev);

	/* Debugging function */
	void dupeCheck();
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
	int topoMax(ListDigraph::Node cfrg_node, ListDigraph::NodeMap<int> &dist);
	void topoAdj(pqueue_t &pq, ListDigraph::Node cfrg_node,
		     ListDigraph::NodeMap<int> &dist);
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
	DBG dbg;
	ofstream ord, ilo, sil;
};


#endif /* CFRG_H */

