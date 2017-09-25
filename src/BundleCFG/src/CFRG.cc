#include "CFRG.h"

void
CFRG::order() {
	/*
	 *
	 */
	ListDigraph::NodeMap<int> generation(*this);
	ListDigraph::NodeMap<int> loopOffset(*this);

	/* Handle the loop offsets first */
	
}

void
CFRG::doLoopOffsets(ListDigraph::NodeMap<int> &loopOffset) {
	for (ListDigraph::NodeIt nit(*this); nit != INVALID; ++nit) {
		
	}
}
