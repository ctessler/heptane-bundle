#ifndef CFRGLFS_H
#define CFRGLFS_H

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include "CFRG.h"

/**
 * For lack of a better name, I am calling this a "List First Search"
 * where the element currently being searched is tested to see if it
 * is ready to be worked before the operation is performed.
 *
 * Visiting a node is divided into two parts, the test and the
 * search. To be searched a node must pass its test. If a node does
 * not pass its test, it is placed at the end of the list.
 */
typedef bool (*lfs_filt_t)(CFRG &, CFR *cfr, void *userdata);
typedef bool (*lfs_test_t)(CFRG &, CFR *cfr, void *userdata);
typedef void (*lfs_work_t)(CFRG &, CFR *cfr, void *userdata);

class CFRGLFS {
public:
	CFRGLFS(CFRG &cfrg) : _cfrg(cfrg) {
		_test_fn = NULL;
		_work_fn = NULL;
	}
	CFRGLFS(CFRG &cfrg, lfs_filt_t lfn=NULL, lfs_test_t tfn=NULL,
		lfs_work_t wfn=NULL, void *userdata=NULL) : _cfrg(cfrg) {
		setFilter(lfn);
		setTest(tfn);
		setWork(wfn);
		setData(userdata);
	}
	void setFilter(lfs_filt_t fn) {
		_filt_fn = fn;
	}
	void setTest(lfs_test_t fn) {
		_test_fn = fn;
	}
	void setWork(lfs_work_t fn) {
		_work_fn = fn;
	}
	void setData(void *userdata) {
		_ud = userdata;
	}
	void search(CFR *start, CFR *end=NULL);
private:
	CFRG &_cfrg;
	lfs_filt_t _filt_fn;
	lfs_test_t _test_fn;
	lfs_work_t _work_fn;
	void *_ud;

	void step(list<CFR*> &q, ListDigraph::NodeMap<bool> &visited);
};
#endif
