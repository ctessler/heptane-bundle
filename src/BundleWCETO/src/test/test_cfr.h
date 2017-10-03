#ifndef CFR_H
#define CFR_H

#include "test_cfg.h"
#include <lemon/dijkstra.h>

class CFR : public CFG {
public:
	CFR(CFG &cfg) : CFG(), _cfg(cfg), _to_cfg(*this), _from_cfg(*this) {
	}
	/* Returns the CFR membership of this node */
	ListDigraph::Node membership(ListDigraph::Node node) const {
		if (!valid(node)) {
			return INVALID;
		}
		if (!_cfg.valid(_membership)) {
			throw runtime_error("Node mapping invalidated");
		}
		return _membership;
	}
	CFR(CFR &cfr) : CFG(), _cfg(cfr._cfg), _to_cfg(*this), _from_cfg(*this) {
		cout << "CFR Copy Constructor invoked!" << endl;
	}
	/* Returns a pointer to the CFG which this CFR was extracted from */
	CFG* getCFG() { return &_cfg; }

	/* Gets and sets the Cache */
	void setCache(Cache *cache) { _cache = cache; }
	Cache* getCache() { return _cache; }
	
	/*
	 * Override to prevent nodes from being added without having a node in
	 * the original CFG
	 */
	ListDigraph::Node addNode() {
		throw runtime_error("Cannot add a node to a CFR without a CFG node");
	}
	ListDigraph::Node addNode(ListDigraph::Node from_cfg);
	/* Gets the node in the original CFG from the CFR */
	ListDigraph::Node toCFG(ListDigraph::Node);
	/* Gets the node in the CFR given the CFG node */
	ListDigraph::Node fromCFG(ListDigraph::Node);

	/* Override to prevent nodes being added without a node from the CFG */
	void setInitial() {
		throw runtime_error("Cannot set the initial node without a CFG node");
	}
	void setInitial(ListDigraph::Node cfr_initial, ListDigraph::Node cfg_initial);

	string stringNode(ListDigraph::Node node) const;
	friend std::ostream &operator<< (std::ostream &stream,
					 const CFR& cfr);

	/* WCETO calculation for *this* CFR */
	unsigned long int wcet(unsigned int threads);

	/* Unit test */
	static bool test();
private:
	CFG &_cfg;

	/*
	 * All nodes of a CFR have the same initial instruction, which was
	 * derived from the original CFG. _membership is the node of the initial
	 * instruction in the CFG this CFR was derived from.
	 *
	 * _to_cfg maps from *this* CFR to the CFG it was derived from
	 */
	ListDigraph::Node _membership;
	ListDigraph::NodeMap<ListDigraph::Node> _to_cfg;
	ListDigraph::NodeMap<ListDigraph::Node> _from_cfg;
	
	/*
	 * A note about loop heads of nodes, the CFR stores the CFG node as the
	 * loop head. This is because the loop head may not be contained within
	 * the CFR.
	 */

	/* A CFR cannot exist without a cache */
	Cache *_cache;
};

#endif /* CFR_H */
