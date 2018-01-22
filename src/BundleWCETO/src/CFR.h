#ifndef CFR_H
#define CFR_H

#include "CFRECBs.h"
#include "CFG.h"
#include "DBG.h"
#include "CFGTopSort.h"
#include <lemon/dijkstra.h>

class CFR : public CFG {
public:
	CFR(CFG &cfg) : CFG(), _cfg(cfg), _to_cfg(*this)
	{
		_switching = true;
		_exe = 0;
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
	CFR(CFR &cfr) : CFG(), _cfg(cfr._cfg), _to_cfg(*this)
	{
		_switching = true;
		_exe = 0;
	}
	/* Returns a pointer to the CFG which this CFR was extracted from */
	CFG* getCFG() const { return &_cfg; }

	/* Gets and sets the Cache */
	void setCache(Cache *cache) { _cache = cache; }
	Cache* getCache() { return _cache; }

	/* Gets and sets the switching property of *this* CFR */
	void setSwitching(bool on=true);
	bool getSwitching();

	/* WCETO calculation for *this* CFR */
	uint32_t wceto(uint32_t threads);
	/* Finds the maximum execution cost of one thread */
	uint32_t exeCost();
	/* Finds the cost for loading the cache with all instructions */
	uint32_t loadCost();

	/*
	 * ECB calculation and retrieval
	 *
	 * Caller of ECBs() must delete the list.
	 */
	uint32_t calcECBs();
	ECBs* getECBs();
	
	/*
	 * Override to prevent nodes from being added without having a node in
	 * the original CFG
	 */
	ListDigraph::Node addNode() {
		throw runtime_error("Cannot add a node to a CFR without a CFG node");
	}
	ListDigraph::Node addNode(ListDigraph::Node from_cfg);
	/* Gets the node in the original CFG from the CFR */
	ListDigraph::Node toCFG(ListDigraph::Node) const;
	/* Gets the node in the CFR given the CFG node */
	ListDigraph::Node fromCFG(ListDigraph::Node);

	/* 
	 * Overloaded methods
	 * 
	 * These ensure that the mapping to the control flow graph is preserved 
	 */
	/*
	 * Override to safely store the CFG head node
	 */
	
	void setInitial(ListDigraph::Node cfr_initial);
	/* getInitial will return the node *in* the CFR */

	/* Gets and sets the function associated with the instruction */
	FunctionCall getFunction(ListDigraph::Node node) const;
	void setFunction(ListDigraph::Node, FunctionCall const &fcall) {
		throw runtime_error("CFG::setFunction cannot be called on a CFR node");
	}

	/* Gets and sets the address of the instruction */
	iaddr_t getAddr(ListDigraph::Node node) const;
	string stringAddr(ListDigraph::Node node) const;
	void setAddr(ListDigraph::Node, iaddr_t addr) {
		throw runtime_error("CFG::setAddr cannot be called on a CFR node");
	}

	/* Finds the instruction with the address and function */
	ListDigraph::Node find(iaddr_t addr, FunctionCall const &fcall);

	/* Finds all instructions with the address */
	list<ListDigraph::Node> find(iaddr_t addr);

	/*
	 * getHead returns the node from the ** CFG ** because the head may not
	 * be contained within the CFR
	 */
	ListDigraph::Node getHead(ListDigraph::Node node) const;
	void setHead(ListDigraph::Node cfr_node, ListDigraph::Node cfg_node);
	/* Gets and sets if the instruction is a loop head */
	bool isHead(ListDigraph::Node node) const;
	void markHead(ListDigraph::Node node, bool yes=true) {
		throw runtime_error("CFG::markHead cannot be called on a CFR node");
	}
	/* Gets and sets the number of iterations in the loop started at the
	   instruction */
	unsigned int getIters(ListDigraph::Node head) const;
	void setIters(ListDigraph::Node head, unsigned int iters) {
		throw runtime_error("CFG::setIters cannot be called on a CFR node");
	}

	string stringNode(ListDigraph::Node node) const;
	friend std::ostream &operator<< (std::ostream &stream, const CFR& cfr);
	string str() const;

private:
	/*
	 * A note about loop heads of nodes, the CFR stores the CFG node as the
	 * loop head. This is because the loop head may not be contained within
	 * the CFR.
	 */
	CFG &_cfg;
	uint32_t _exe;
	ECBs _ecbs;
	/* A CFR cannot exist without a cache */
	Cache *_cache;
	/**
	 * CFRs are "switching" or "pass through" switching means it
	 * forces a thread level context switch, pass through means it
	 *  does not. This mapping records if it is switching or pass through
	 */
	bool _switching;
	DBG dbg;
	
	/* 
	 * This is the identifier of the CFR (in the CFG) which all nodes belong
	 * to, the CFR stores the node from the CFG ie 
	 *
	 *    (CFR::valid(_membership) == false)
	 */
	ListDigraph::Node _membership;
	/* CFR node -> CFG node */
	ListDigraph::NodeMap<ListDigraph::Node> _to_cfg;
	/* CFG node -> CFR node */
	map<ListDigraph::Node, ListDigraph::Node> _from_cfg;
	/* CFR node -> CFG node (that is the head) */
	map<ListDigraph::Node, ListDigraph::Node> _cfg_head;

	uint32_t maxLoads();
};

#endif /* CFR_H */
