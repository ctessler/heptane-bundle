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
private:
	CFG &_cfg;
};

#endif
