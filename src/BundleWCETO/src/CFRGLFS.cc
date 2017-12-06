#include "CFRGLFS.h"
void
CFRGLFS::search(CFR *start, CFR *end) {
	ListDigraph::NodeMap<bool> visited(_cfrg);

	list<CFR*> q;
	q.push_front(start);

	do {
		step(q, visited);
		CFR *next = q.front();
		if (next == end) {
			return;
		}
	} while (!q.empty());

}

/**
 * Advances the search by one node
 */
void
CFRGLFS::step(list<CFR*> &q, ListDigraph::NodeMap<bool> &visited) {
	CFR *current = q.front();
	q.pop_front();

	ListDigraph::Node cur_node = _cfrg.findNode(current);

	if (_filt_fn && ! _filt_fn(_cfrg, current, _ud)) {
		/* Didn't pass the filter, all done, do not requeue */
		return;
	}
	
	if (_test_fn && ! _test_fn(_cfrg, current, _ud)) {
		/* Did not pass the test */
		q.push_back(current);
		return;
	}

	if (_work_fn) {
		_work_fn(_cfrg, current, _ud);
	}
	visited[cur_node] = true;

	ListDigraph::OutArcIt ait(_cfrg, cur_node);
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node succ = _cfrg.target(ait);
		if (visited[succ]) {
			continue;
		}
		CFR *succ_cfr = _cfrg.findCFR(succ);
		q.push_back(succ_cfr);
	}
}
