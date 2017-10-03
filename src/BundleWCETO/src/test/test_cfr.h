#ifndef TEST_CFR_H
#define TEST_CFR_H

#include "test_cfg.h"

class CFR : public CFG {
public:
	CFR (CFG &cfg) : _cfg(cfg) { }
	ListDigraph::Node addNode() {

	}
	ListDigraph::Node addNode(ListDigraph::Node from_cfg) {
		return CFG::addNode();
	}

	/* Override to prevent nodes being added without a node from the CFG */
	void setInitial(ListDigraph::Node) {
		throw runtime_error("Cannot set the initial node without a CFG node");
	}
	void setInitial(ListDigraph::Node cfr_initial, ListDigraph::Node cfg_initial);
private:
	CFG &_cfg;

	ListDigraph::Node _membership;	
};

#endif
