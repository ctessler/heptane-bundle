#include "test_cfr.h"

void
CFR::setInitial(ListDigraph::Node cfr_initial, ListDigraph::Node cfg_initial) {
	_membership = cfg_initial;
	CFG::setInitial(cfr_initial);
}
