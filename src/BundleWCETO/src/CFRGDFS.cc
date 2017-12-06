#include "CFRGDFS.h"

bool
CFRGDFS::search(CFR* start) {
	string prefix = "CFRGDFS::search ";
	_visited.clear();

	stack<CFR *> stk;
	stk.push(start);

	while (!stk.empty()) {
		CFR *cur = stk.top(); stk.pop();
		cout << prefix << *cur << endl;

		if ((cur != start) && _sel_fn) {
			if (_sel_fn(_cfrg, cur, _userdata)) {
				cout << prefix << " has reached the search node"
				     << endl;
				return true;
			}
		}
		if (_work_fn) {
			cout << prefix << "calling work" << endl;
			_work_fn(_cfrg, cur, _userdata);
		}
		_visited[cur] = true;
		
		ListDigraph::Node cfrg_node = _cfrg.findNode(cur);
		if (cfrg_node == INVALID) {
			throw runtime_error("Unable to find CFRG node for CFR");
		}
		ListDigraph::OutArcIt ait(_cfrg, cfrg_node);
		for ( ; ait != INVALID; ++ait) {
			ListDigraph::Node node_succ = _cfrg.target(ait);
			CFR *cfr_succ = _cfrg.findCFR(node_succ);
			cout << prefix << *cur << " has successor "
			     << *cfr_succ << endl;
			if (_mask_fn) {
				if (!_mask_fn(_cfrg, cfr_succ, _userdata)) {
					continue;
				}
			}
			if (!_visited[cfr_succ]) {
				stk.push(cfr_succ);
			}
		}
	}
	return false;
}
