#ifndef CFRGWCETO_H
#define CFRGWCETO_H

#include <limits.h>
#include <queue>

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include "CFRG.h"
#include "CFRGDFS.h"
#include "CFRGLFS.h"
#include "DBG.h"
using namespace lemon;

class WCETOMap : public std::map<uint32_t, uint32_t> {
	/*
	 * Records the delta of adding one more thread at each level,
	 * not the total WCETO at that level.
	 *
	 * thread 0 = 0 
	 * thread 1 = 1 thread - 0 threads
	 * thread 2 = 2 threads - 1 thread
	 * ...
	 *
	 * [0] = 0
	 * [1] = 110
	 * [2] = 120 - 110 = 10
	 * [3] = 130 - 120 = 10
	 * ...
	 */
public:
	friend std::ostream &operator<<(std::ostream&, const WCETOMap&);
	uint32_t wceto();
};

class CFRTable : public std::map<CFR*, WCETOMap*> {
public:
	friend std::ostream &operator<<(std::ostream&, const CFRTable&);
};

class CFRGWCETOFactory;

typedef struct {
	CFRGWCETOFactory *lud_this;
	CFR *lud_head_cfr;
	CFRTable *lud_scratch_tbl;
	int lud_threads;
} loop_user_data_t;

class CFRGWCETOFactory {
public:
	CFRGWCETOFactory(CFRG &cfrg) : _cfrg(cfrg) {
		
	}
	~CFRGWCETOFactory();
	void setThreads(uint32_t threads) {
		_nthreads = threads;
	}
	uint32_t getThreads() {
		return _nthreads;
	}
	void produce();
	uint32_t value(CFR *cfr);
	uint32_t CFRWCETO(CFR *cfr);
	void LoopWCETO(CFR *cfr);
	CFRTable &cfrtable() { return _cfrtable; }
	CFRTable &looptable() { return _looptable; }
	bool useLoopTable(CFR *cfr, CFR *pred);
	/**
	 * Collects the WCETO's of the predecessors of this CFR
	 * placing them in the result.
	 *
	 * @param[in] cfr requesting WCETOs for CFRs preceding this
	 * one
	 * @param[in] singles CFRTable for CFRs outside of loops
	 * @param[in] lpps CFRTable for CFRs within loops
	 * @param[out] result where the predecessors CFR -> WCETO
	 *    mappings are placed 
	 *
	 * @return true if the operation could be completed, false if
	 *     predecessors values were unavailable. 
	 */
	bool justPreds(CFR* cfr, CFRTable &singles, CFRTable &loops,
		       CFRTable &result);

	/**
	 * Adds the maximum values from the predecessor table to the
	 * WCETO times for a single CFR
	 *
	 * @param[in|out] wmap incremented by the maximum value for
	 * each thread
	 * @param[in|out] preds table of WCETO values, when
	 * selected the WCETO value for a number of threads is set to
	 * 0.
	 *
	 * @return true upon success, false if an error occurred.
	 */
	bool addMaxPreds(WCETOMap &wmap, CFRTable &preds);
	DBG dbg;
private:
	CFRG &_cfrg;
	uint32_t _nthreads;
	/*
	 * Individual CFRs have a WCETO in this table
	 * Members of a loop do not.
	 */
	CFRTable _cfrtable;
	/*
	 * Loops are treated as single nodes in this calculation,
	 * a total WCET is needed for each. In this table, any node
	 * that is a member of a loop has the total WCETO of the
	 * entire loop as it's WCETO. 
	 */ 
	CFRTable _looptable;
};


#endif /* CFRGWCETO_H */

