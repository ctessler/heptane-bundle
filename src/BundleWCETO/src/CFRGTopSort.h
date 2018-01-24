#ifndef CFRGTopSort_H
#define CFRGTopSort_H
#include "CFRG.h"
#include "CFRGDFS.h"
#include "PQueue.h"

typedef bool (*CFRGTS_mask_t)
    (CFRG &cfrg, ListDigraph::Node node, void *userdata); 
typedef bool (*CFRGTS_work_t)(CFRG &cfrg, ListDigraph::Node node, void *userdata);
typedef bool (*CFRGTS_sel_t)(CFRG &cfrg, ListDigraph::Node node, void *userdata);

class CFRGTopSort {
public:
	CFRGTopSort(CFRG &cfrg) : _cfrg(cfrg), _distances(_cfrg), _resd(_cfrg),
			       result((NodeCompLT(_resd))) {
	}
	CFRGTopSort(CFRG &cfrg, void *userdata,
	    CFRGTS_mask_t mn=NULL, CFRGTS_work_t wn=NULL, CFRGTS_sel_t sn=NULL)	:
		_userdata(userdata), _cfrg(cfrg), _distances(_cfrg),
		_resd(_cfrg), result((NodeCompLT(_resd))) {
		setMask(mn);
		setWork(wn);
		setSel(sn);
	}
	/*
	 * Performs the topological sort, returns true if the
	 * selector function found the node.
	 */
	bool sort(ListDigraph::Node start);
	/* 
	 * Sets the search mask, when the mask returns false the search
	 * will not include the node
	 */
	void setMask(CFRGTS_mask_t mask) { _mask_fn = mask; }
	CFRGTS_mask_t getMask() { return _mask_fn; }
	/*
	 * Sets the working function, each node that passes the mask
	 * will have this work function called upon it before
	 * processing its children. 
	 */
	void setWork(CFRGTS_work_t work) { _work_fn = work; }
	CFRGTS_work_t getWork() { return _work_fn; }
	/*
	 * Sets the select function, the search will stop *before*
	 * performing any work on the node that matches the selector.
	 */
	void setSel(CFRGTS_sel_t sel) { _sel_fn = sel; }
	CFRGTS_sel_t getSel() { return _sel_fn; }
	/*
	 * Sets the user data
	 */
	void setUserData(void *data) { _userdata = data; }
	void *getUserData() { return _userdata; }

	pqueue_t result;
private:
	CFRG &_cfrg;
	ListDigraph::NodeMap<int> _distances;
	ListDigraph::NodeMap<int> _resd;
	CFRGTS_mask_t _mask_fn = NULL;
	CFRGTS_work_t _work_fn = NULL;
	CFRGTS_sel_t _sel_fn = NULL;
	void *_userdata = NULL;
};

#endif /* CFRDFS_H */
