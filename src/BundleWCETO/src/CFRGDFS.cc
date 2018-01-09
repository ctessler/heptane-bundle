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
				/* Reached the target */
				return true;
			}
		}
		if (_work_fn) {
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

bool
CFRGDFS::recSearch(CFR *start) {
	_visited.clear();
	return _recSearch(start);
}

bool
CFRGDFS::_recSearch(CFR *start) {
	if (_sel_fn) {
		if (_sel_fn(_cfrg, start, _userdata)) {
			/* Reached the target */
			return true;
		}
	}

	if (_work_fn) {
		_work_fn(_cfrg, start, _userdata);
	}
	_visited[start] = true;
	ListDigraph::OutArcIt ait(_cfrg, _cfrg.findNode(start));
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node nsucc = _cfrg.target(ait);
		CFR *succ = _cfrg.findCFR(nsucc);
		if (_mask_fn) {
			if (!_mask_fn(_cfrg, succ, _userdata)) {
				continue;
			}
		}
		if (!_visited[succ]) {
			_recSearch(succ);
		}
	}
	if (_fin_fn) {
		_fin_fn(_cfrg, start, _userdata);
	}
}
