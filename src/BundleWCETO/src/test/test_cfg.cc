#include "test_cfg.h"

/**
 * CFG Class
 */
CFG::CFG() : ListDigraph(), _function(*this)
{
	_initial = INVALID;
}

void
CFG::setInitial(ListDigraph::Node node) {
	_initial = node;
}


ListDigraph::Node
CFG::addNode() {
	ListDigraph::Node rv = ListDigraph::addNode();

	#if 0
	_addr[rv] = 0;
	_loop_head[rv] = INVALID;
	_is_loop_head[rv] = false;
	_loop_iters[rv] = 0;
	#endif
	_function[rv] = FunctionCall("UNASSIGNED", 0x0);
	
	return rv;
}
