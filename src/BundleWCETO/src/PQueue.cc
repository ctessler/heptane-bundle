#include "PQueue.h"

bool NodeCompLT::operator() (const ListDigraph::Node &lhs,
			       const ListDigraph::Node &rhs) const {
	if (_dist[lhs] < _dist[rhs]) {
		/* Least first */
		return true;
	}
	return false;
}

bool NodeCompGR::operator() (const ListDigraph::Node &lhs,
			       const ListDigraph::Node &rhs) const {
	if (_dist[lhs] > _dist[rhs]) {
		/* Greatest */
		return true;
	}
	return false;
}
