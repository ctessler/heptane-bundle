#ifndef CFRG_H
#define CFRG_H

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include "CFG.h"
#include "CFR.h"
using namespace lemon;

class CFRG : public ListDigraph {
public:
	CFRG(CFG &cfg) : _cfg(cfg) {
		
	}
	/* Override to ensure the mapping is enforced */
	ListDigraph::Node addNode() {
		throw runtime_error("Cannot add a node to a CFRG without a CFR");
	}
	ListDigraph::Node addNode(CFR *cfr) {
		map<CFR*, ListDigraph::Node>::iterator mit = _from_cfr_to_node.find(cfr);
		if (mit != _from_cfr_to_node.end()) {
			return mit->second;
		}
		ListDigraph::Node rv = ListDigraph::addNode();
		_from_cfr_to_node[cfr] = rv;
		_from_node_to_cfr[rv] = cfr;
		return rv;
	}
	ListDigraph::Node findNode(CFR *cfr) {
		return _from_cfr_to_node[cfr];
	}
	CFR* findCFR(ListDigraph::Node node) {
		map<ListDigraph::Node, CFR*>::iterator mit = _from_node_to_cfr.find(node);
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
	void order();
private:
	CFG &_cfg;
	CFR *_initial = NULL;
	map<CFR*, ListDigraph::Node> _from_cfr_to_node;
	map<ListDigraph::Node, CFR*> _from_node_to_cfr;

	void doLoopOffsets(ListDigraph::NodeMap<int> &loopOffset);
};

#endif /* CFRG_H */

