#ifndef CFGTopSort_H
#define CFGTopSort_H
#include "CFG.h"
#include "CFGDFS.h"
#include "PQueue.h"

typedef bool (*TS_mask_t)(CFG &cfg, ListDigraph::Node node, void *userdata); 
typedef bool (*TS_work_t)(CFG &cfg, ListDigraph::Node node, void *userdata);
typedef bool (*TS_sel_t)(CFG &cfg, ListDigraph::Node node, void *userdata);

class CFGTopSort {
public:
	CFGTopSort(CFG &cfg) : _cfg(cfg), _distances(_cfg), _resd(_cfg),
			       result((NodeCompLT(_resd))) {
	}
	CFGTopSort(CFG &cfg, void *userdata,
	    TS_mask_t mn=NULL, TS_work_t wn=NULL, TS_sel_t sn=NULL)
		: _userdata(userdata), _cfg(cfg), _distances(_cfg), _resd(_cfg),
		  result((NodeCompLT(_resd))) {
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
	void setMask(TS_mask_t mask) { _mask_fn = mask; }
	TS_mask_t getMask() { return _mask_fn; }
	/*
	 * Sets the working function, each node that passes the mask
	 * will have this work function called upon it before
	 * processing its children. 
	 */
	void setWork(TS_work_t work) { _work_fn = work; }
	TS_work_t getWork() { return _work_fn; }
	/*
	 * Sets the select function, the search will stop *before*
	 * performing any work on the node that matches the selector.
	 */
	void setSel(TS_sel_t sel) { _sel_fn = sel; }
	TS_sel_t getSel() { return _sel_fn; }
	/*
	 * Sets the user data
	 */
	void setUserData(void *data) { _userdata = data; }
	void *getUserData() { return _userdata; }

	pqueue_t result;
private:
	CFG &_cfg;
	ListDigraph::NodeMap<int> _distances;
	ListDigraph::NodeMap<int> _resd;
	TS_mask_t _mask_fn = NULL;
	TS_work_t _work_fn = NULL;
	TS_sel_t _sel_fn = NULL;
	void *_userdata = NULL;
};

#endif /* CFRDFS_H */
