#include "CFGDFS.h"

bool
CFGDFS::search(ListDigraph::Node start) {
	string prefix = "CFGDFS::search ";
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		ListDigraph::Node n = nit;
		_visited[n] = false;
	}

	stack<ListDigraph::Node> stk;
	stk.push(start);

	while (!stk.empty()) {
		ListDigraph::Node cur = stk.top(); stk.pop();

		if ((cur != start) && _sel_fn) {
			if (_sel_fn(_cfg, cur, _userdata)) {
				/* Reached the target */
				return true;
			}
		}
		if (_work_fn) {
			_work_fn(_cfg, cur, _userdata);
		}
		_visited[cur] = true;
		ListDigraph::OutArcIt ait(_cfg, cur);
		for ( ; ait != INVALID; ++ait) {
			ListDigraph::Node succ = _cfg.target(ait);
			if (_mask_fn) {
				if (!_mask_fn(_cfg, succ, _userdata)) {
					continue;
				}
			}
			if (!_visited[succ]) {
				stk.push(succ);
			}
		}
	}
	return false;
}

bool
CFGDFS::recSearch(ListDigraph::Node start) {
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		ListDigraph::Node n = nit;
		_visited[n] = false;
	}
	return _recSearch(start);
}

bool
CFGDFS::_recSearch(ListDigraph::Node start) {
	if (_sel_fn) {
		if (_sel_fn(_cfg, start, _userdata)) {
			/* Reached the target */
			return true;
		}
	}

	if (_work_fn) {
		_work_fn(_cfg, start, _userdata);
	}
	_visited[start] = true;
	ListDigraph::OutArcIt ait(_cfg, start);
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node succ = _cfg.target(ait);
		if (_mask_fn) {
			if (!_mask_fn(_cfg, succ, _userdata)) {
				continue;
			}
		}
		if (!_visited[succ]) {
			_recSearch(succ);
		}
	}
	if (_fin_fn) {
		_fin_fn(_cfg, start, _userdata);
	}
}
