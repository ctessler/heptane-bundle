#ifndef DOTFROMCFR_H
#define DOTFROMCFR_H
#include "CFR.h"
#include "DOTFactory.h"

class DOTfromCFR : public DOTFactory {
public:
	DOTfromCFR(CFR &cfr) : DOTFactory(cfr), _cfr(cfr), _visited(_cfr) { }
	void produce();
private:
	CFR &_cfr;
	ListDigraph::NodeMap<bool> _visited;
	string _indent;
	stringstream _debug;

	void clearVisited() {
		for (ListDigraph::NodeIt nit(_cfr); nit != INVALID; ++nit) {
			_visited[nit] = false;
		}
	}
	bool visited(ListDigraph::Node node) {
		return _visited[node];
	}
	void visit(ListDigraph::Node node, bool yes=true) {
		_visited[node] = yes;
	}

	void loopDOT(ListDigraph::Node head, ofstream &os,
		     stack<ListDigraph::Node> &calls,
		     stack<ListDigraph::Node> &subsq);
	ListDigraph::Node nodeDOT(ofstream &os, ListDigraph::Node node);
	string edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		       ListDigraph::Node v, ListDigraph::Node port_v);

	string nodeLabel(ListDigraph::Node node);
	string nodeDOTstart(ListDigraph::Node node);
	string nodeDOTend(ListDigraph::Node node);
	string nodeDOTrow(ListDigraph::Node node);
};

#endif
