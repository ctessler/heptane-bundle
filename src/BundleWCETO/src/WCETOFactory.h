#ifndef WCETOFACTORY_H
#define WCETOFACTORY_H

#include "CFRG.h"
#include "DBG.h"
#include "CFRGLFS.h"
#include "CFRWCETOMap.h"
#include "CFRDemandMap.h"

typedef list<ThreadWCETOMap*> PredList;

class WCETOFactory {
public:
	WCETOFactory(CFRG &cfrg) : _cfrg(cfrg) {
	}
	WCETOFactory(CFRG &cfrg, uint32_t threads) : _cfrg(cfrg) {
		setThreads(threads);
	}
	/**
	 * Sets the number of threads the WCETO will be calculated for
	 *
	 * @param[in] threads the number of threads
	 */
	void setThreads(uint32_t threads) {
		_threads = threads;
	}
	uint32_t getThreads() {
		return _threads;
	}
	/**
	 * Induces the calculation of WCETO values
	 */
	void produce();
	/**
	 * Gets the WCETO value for a single CFR from the CFRG
	 *
	 * @param[in] cfr the CFR being inquired about (only valid after a call
	 * to produce) 
	 *
	 * @return the number of cycles the bounding the WECTO of the CFR.
	 */
	uint32_t value(CFR *cfr);
	/**
	 * Calculates the WCETO for a single CFR that is not a member of a loop
	 * (in the CFRG)
	 *
	 * @param[in] cfr the CFR having it's WCETO table calculated
	 */
	ThreadWCETOMap *isolatedWCETO(CFR *cfr);
	ThreadWCETOMap *loopWCETO(CFR *cfr);

	CFRG& cfrg() { return _cfrg; }
	CFRWCETOMap& cfr_table() { return _cfr_table; }
	CFRWCETOMap& loop_table() { return _loop_table; }
	CFRWCETOMap& scratch_table() { return _scratch_table; }

	DBG dbg;
	/**
	 * Incorporates the WCETO values from the preceding CFRs
	 *
	 * @param[in|out] twmap ThreadWCETO map for the CFR
	 * @param[in|out] preds list of WCETO maps for predecessors, ruined upon
	 * return.
	 */
	bool addPreds(ThreadWCETOMap *twmap, PredList& preds);
	/**
	 * Finds the immediate predecessors of *this* cfr 
	 *
	 * @param[in] cfr the CFR being asked about
	 * @param[in] cwmap the map to find the thread tables of the predecessors
	 * @param[out] preds the list of the immediate successors.
	 */
	uint32_t findPreds(CFR* cfr, CFRWCETOMap& cwmap, PredList& preds);
	CFR *outermostCFR(CFR *inloop);

	/**
	 * Returns the list of ECBs contained within the loop
	 */
	list<uint32_t>* loopECBs(CFR* cfr);
	uint32_t loopLoadCount(list<uint32_t> &ecbs);
private:
	uint32_t _threads;
	CFRG &_cfrg;
	CFRWCETOMap _cfr_table, _loop_table, _scratch_table;

	/** Helper for addPreds */
	uint32_t APmaxWCETO(ThreadWCETOMap &twmap, uint32_t &max_idx);
};

#endif /* WCETOFACTORY_H */
