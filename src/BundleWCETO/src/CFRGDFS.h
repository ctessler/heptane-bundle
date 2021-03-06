#ifndef CFRGDFS_H
#define CFRGDFS_H
#include "CFRG.h"

typedef bool (*DFS_mask_t)(CFRG &, CFR *cfr, void *userdata); 
typedef bool (*DFS_work_t)(CFRG &cfrg, CFR *cfr, void *userdata);
typedef bool (*DFS_sel_t)(CFRG &cfrg, CFR *cfr, void *userdata);
typedef bool (*DFS_fin_t)(CFRG &cfrg, CFR *cfr, void *userdata);

class CFRGDFS {
public:
	CFRGDFS(CFRG &cfrg) : _cfrg(cfrg) {
	}
	/*
	 * Performs the depth first search, returns true if the
	 * selector function found the node.
	 *
	 * search - uses a stack, no finish function available
	 * recSearch - uses recursion, finish function available
	 */
	bool search(CFR *start);
	bool recSearch(CFR *start);
	/* 
	 * Sets the search mask, when the mask returns false the search
	 * will not include the CFR
	 */
	void setMask(DFS_mask_t mask) { _mask_fn = mask; }
	DFS_mask_t getMask() { return _mask_fn; }
	/*
	 * Sets the working function, each node that passes the mask
	 * will have this work function called upon it before
	 * processing its children. 
	 */
	void setWork(DFS_work_t work) { _work_fn = work; }
	DFS_work_t getWork() { return _work_fn; }
	/*
	 * Sets the select function, the search will stop *before*
	 * performing any work on the CFR that matches the selector.
	 */
	void setSel(DFS_sel_t sel) { _sel_fn = sel; }
	DFS_sel_t getSel() { return _sel_fn; }
	/*
	 * Sets the finish function, called on each CFR after all of
	 * it's children have been visited -- only called if recSearch
	 * is used. 
	 */
	void setFin(DFS_fin_t fin) { _fin_fn = fin; }
	DFS_sel_t setFin() { return _fin_fn; }
	/*
	 * Sets the user data
	 */
	void setUserData(void *data) { _userdata = data; }
	void *getUserData() { return _userdata; }
private:
	CFRG &_cfrg;
	map<CFR*, bool> _visited;
	DFS_mask_t _mask_fn = NULL;
	DFS_work_t _work_fn = NULL;
	DFS_sel_t _sel_fn = NULL;
	DFS_fin_t _fin_fn = NULL;
	void *_userdata = NULL;

	bool _recSearch(CFR *start);	
};

#endif /* CFRDFS_H */
