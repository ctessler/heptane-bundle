#ifndef PQUEUE_H
#define PQUEUE_H

#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <set>
using namespace std;
using namespace lemon;

class NodeCompLT {
public:
	NodeCompLT(ListDigraph::NodeMap<int> &distances) : _dist(distances) {
	}
	bool operator() (const ListDigraph::Node &lhs,
			 const ListDigraph::Node &rhs) const;
private:
	ListDigraph::NodeMap<int> &_dist;
};

class NodeCompGR {
public:
	NodeCompGR(ListDigraph::NodeMap<int> &distances) : _dist(distances) {
	}
	bool operator() (const ListDigraph::Node &lhs,
			 const ListDigraph::Node &rhs) const;
private:
	ListDigraph::NodeMap<int> &_dist;
};

typedef multiset<ListDigraph::Node, NodeCompLT> pqueue_t;
typedef multiset<ListDigraph::Node, NodeCompGR> pqueue_gr_t;

#endif
