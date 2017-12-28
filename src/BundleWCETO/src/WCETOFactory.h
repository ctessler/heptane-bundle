#ifndef WCETOFACTORY_H
#define WCETOFACTORY_H

#include "CFRG.h"
#include "DBG.h"
#include "CFRGLFS.h"
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
	 * References to internal structures, used by the searching
	 * mechanisms.
	 */
	CFRG& cfrg() { return _cfrg; }
	CFRDemandMap& cfr_table() { return _cfrt; }
	CFRDemandMap& loop_table() { return _loopt; }
	CFRDemandMap& scratch_table() { return _scratcht; }

	/**
	 * Calculates the demand for a single CFR not contained within
	 * a loop.
	 */
	CFRDemand* inDemand(CFR* cfr);
	/**
	 * Calculates the demand for a loop begining with the given
	 * CFR
	 */
	CFRDemand* loopDemand(CFR* cfr);
	CFRDemand* old_loopDemand(CFR* cfr);	

	/**
	 * Gets the demand for a given CFR, requires produce() has
	 * been called beforehand.
	 *
	 * Do not modify or free the returned Demand.
	 */
	CFRDemand* getDemand(CFR *cfr);
	
	/**
	 * Returns the number of times duplicate entries are in the
	 * list. 
	 *
	 * Examples:
	 * (1, 2) -- dupeCount returns 0
	 * (1, 1) -- returns 2
	 * (1, 1, 2, 2, 2) -- returns 5
	 */
	uint32_t dupeCount(ECBs &ecbs);
	/**
	 * Debug object
	 */
	DBG dbg;
	void dumpCFRs();
private:
	uint32_t _threads;
	CFRG &_cfrg;
	CFRDemandMap _cfrt, _loopt, _scratcht;


	/**
	 * Merges the execution (and maybe cache loads) into the given
	 * demand map.
	 *
	 * @param[in|out] dem the demand being updated
	 * @param[in|out] map of demands being merged into dem,
	 *   ruined after calling
	 * @param[in] includeLoad when false only execution values are
	 *     considered. When true, cache loads are also considered.
	 */
	void maxMerge(CFRDemand &dem, CFRDemandMap &map, bool includeLoad);
	uint32_t maxEle(ThreadWCETOMap &dem, uint32_t &idx);
};

#endif /* WCETOFACTORY_H */
