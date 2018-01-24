#include "WCETOFactory.h"

/**
 * List First Search filter function
 *
 * Returning true indicates that the CFR can be passed to the test function.
 */
#define dout fact->dbg.buf << fact->dbg.start
static bool
lfs_top_filter(CFRG &cfrg, CFR *cfr, void *userdata) {
	WCETOFactory *fact = (WCETOFactory *) userdata;
	bool rv = true;
	fact->dbg.inc("WCETOFactory::lfs_top_filter ");
	dout << *cfr << " does ";
	/**
	 * Since this is the top level LFS, only top level loop heads
	 * and independant nodes
	 */
	ListDigraph::Node cfri = cfr->getInitial();
	ListDigraph::Node cfgi = cfr->toCFG(cfri);
	CFG *cfg = cfr->getCFG();
	if (cfg->getHead(cfgi) != INVALID) {
		/*
		 * Only those nodes that are loop heads, or do not
		 * participate in a loop pass the filter
		 */
		fact->dbg.buf << "not ";
		rv = false;
	}
	fact->dbg.buf << "pass the filter." << endl;
	fact->dbg.flush(cout);
	fact->dbg.dec();
	return rv;
}
#undef dout

/**
 * List First Search Test Function
 *
 * Returning true indicates that the CFR can be passed to the working
 * function. 
 * Returning false indicates that the CFR should be placed at the end of the
 * list, to be processed again later.
 *
 * This test function guarantees that all predecessors of this CFR have had
 * their Demand values calculated, so it is safe for the work function to
 * proceed. 
 * 
 * @param[in] cfrg the CFRG which the CFR comes from
 * @param[in] cfr the CFR being worked on
 * @param[in|out] userdata, should be a pointer to the WCETOFactory
 *
 * @return true if the CFR may be processed by the work function
 */
static bool
lfs_top_test(CFRG &cfrg, CFR *cfr, void *userdata) {
	WCETOFactory *fact = (WCETOFactory *) userdata;
	#define dout fact->dbg.buf << fact->dbg.start

	ListDigraph::Node cfrgi = cfrg.findNode(cfr);
	CFRDemandMap &loopt = fact->loop_table();
	CFRDemandMap &cfrt = fact->cfr_table();
	bool rv = true;
	dout << "working on" << *cfr << endl;
	fact->dbg.inc("WCETOFactory::lfs_top_test ");
	for (ListDigraph::InArcIt iat(cfrg, cfrgi); iat != INVALID; ++iat) {
		ListDigraph::Node pred_node = cfrg.source(iat);
		CFR *pred_cfr = cfrg.findCFR(pred_node);
		if (cfrg.isLoopPart(pred_node)) {
			/* Preceding node is part of a loop */
			if (cfrg.inLoop(cfr, pred_cfr)) {
				dout << *pred_cfr << " is in the loop, ok."
				     << endl;
				/* Members of this loop (cfr) are not
				   expected to be calculated */
				continue;
			}
			/* 
			 * Use the loop table
			 */
			dout << *pred_cfr << " is part of a loop." << endl;
			CFR *outer = cfrg.crown(pred_cfr);
			if (!loopt.present(outer)) {
				dout << *outer << " has not been computed, "
				    << "skipping." << endl;
				rv = false; break;
			}
			continue;
		}
		/* Otherwise it's time to check the singular CFRs */
		if (!cfrt.present(pred_cfr)) {
			dout << *pred_cfr 
			     << " has not been computed, skipping." << endl;
			rv = false; break;
		}
		dout << *pred_cfr << " is an isolated CFR, passes" << endl;
	}
	fact->dbg.dec();

	if (rv) {
		dout << *cfr << " is ready." << endl;
	} else {
		dout << *cfr << " is not ready." << endl;
	}
	
	fact->dbg.flush(cout);
	/* change to return rv after the work portion has been
	   completed */
	return rv;
}
#undef dout

/**
 * List First Work Function
 *
 * Performs the Demand calculation for *this* CFR, if the CF
 *
 */
static void
lfs_top_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	WCETOFactory *fact = (WCETOFactory *) userdata;
	#define dout fact->dbg.buf << fact->dbg.start
	fact->dbg.inc("WCETOFactory::lfs_top_work ");

	ListDigraph::Node cfrgi = cfrg.findNode(cfr);
	if (cfrg.isHead(cfrgi)) {
		dout << *cfr << " is a loop head" << endl;
		fact->loopDemand(cfr);
	} else {
		dout << *cfr << " is an independent CFR" << endl;
		fact->inDemand(cfr);
	}
	fact->dbg.flush(cout);
	fact->dbg.dec();
	#undef dout
}

#define dout dbg.buf << dbg.start
void
WCETOFactory::produce() {
	dbg.inc("WCETOFactory::produce ");

	switchPass();
	
	CFRGLFS lfs(_cfrg, lfs_top_filter, lfs_top_test, lfs_top_work,
		    (void*) this);
	CFR *initial = _cfrg.getInitialCFR();
	lfs.search(initial);

	dbg.dec();
	dbg.flush(cout);
}
#undef dout

uint32_t
WCETOFactory::value(CFR *cfr) {
	uint32_t wceto=0;
	CFRDemand *dmnd = getDemand(cfr);
	if (!dmnd) {
		return 0;
	}
	wceto = dmnd->getWCETOMap().wceto(_threads);
	return wceto;
}

#define dout dbg.buf << dbg.start
CFRDemand *
WCETOFactory::inDemand(CFR* cfr) {
	dbg.inc("inDemand: ");
	CFRDemand *cfrd = _cfrt.present(cfr);
	if (cfrd) {
		dout << *cfr << " is present" << endl;
		dbg.flush(cout); dbg.dec();
		return cfrd;
	}
	cfrd = _cfrt.request(cfr);

	ThreadWCETOMap &twmap = cfrd->getWCETOMap();
	twmap.fillExe(cfr, _threads);
	twmap[1] += cfr->loadCost();

	dout << *cfr << " base demandmap: " << endl
	     << cfrd->str(dbg.start) << endl;

	/* Add the context switch cost (only if it's a switching CFR) */
	if (cfr->getSwitching()) {
		for (uint32_t i = 1; i <= _threads; i++) {
			twmap[i] += _ctx_cost;
		}
		dout << *cfr << " demand with switching: " << endl
		     << cfrd->str(dbg.start) << endl;
	}

	/* Consider predecessors */
	CFRList *preds = _cfrg.preds(cfr);
	CFRDemandMap scratch;
	CFRList::iterator it;
	dbg.inc("inDemand-preds: ");
	for (it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred_cfr = *it;
		dout << *cfr << " preceded by " << endl
		     << dbg.start << "    " << *pred_cfr << endl;
		CFRDemand *pred_dmnd = NULL;
		if (_cfrg.isLoopPartCFR(pred_cfr)) {
			/* Must exist in the loop table */
			CFR *head = _cfrg.crown(pred_cfr);
			pred_dmnd = _loopt.present(head);
		} else {
			/* Must exist in the cfr table */
			pred_dmnd = _cfrt.present(pred_cfr);
		}
		scratch.insert(
		    pair<CFR*, CFRDemand*>(pred_cfr, new CFRDemand(*pred_dmnd)));
	}
	dbg.dec();
	maxMerge(*cfrd, scratch, true);
	dout << *cfr << " demand after merging: " << endl
	     << cfrd->str(dbg.start) << endl;
	delete preds;

	dbg.flush(cout);	
	dbg.dec(); 
	return cfrd;
}
#undef dout

class LoopData {
public:
	LoopData(WCETOFactory& f, CFR* h, CFRDemandMap& s) :
		fact(f), scratch(s), head(h) {
	}
	WCETOFactory& fact;
	CFRDemandMap& scratch;
	CFR* head;
};

/**
 * List First Search filter function
 *
 * Returning true indicates that the CFR can be passed to the test function.
 */
static bool
lfs_loop_filter(CFRG &cfrg, CFR *cfr, void *userdata) {
	LoopData *data = (LoopData *)userdata;
	WCETOFactory &fact = data->fact;
	CFR *head = data->head;

	#define dout fact.dbg.buf << fact.dbg.start
	fact.dbg.inc("lfs_loop_filter: ");

	if (head == cfr) {
		dout << *cfr << " head is not filtered" << endl;
		fact.dbg.flush(cout);
		fact.dbg.dec();
		return true;
	}
	/**
	 * Prevent processing of CFRs outside of the loop
	 */
	bool rv = false;
	if (cfrg.inLoop(head, cfr)) {
		dout << "accepted CFR " << *cfr << fact.dbg.cont
		     << "head is " << *head << endl;
		rv = true;
	} else {
		dout << "rejected CFR " << *cfr << fact.dbg.cont
		     << "head is not " << *head << endl;
	}
	fact.dbg.flush(cout);
	fact.dbg.dec();
	return rv;
}
#undef dout

/**
 * List First Search Test Function
 *
 * Returning true indicates that the CFR can be passed to the working
 * function. 
 * Returning false indicates that the CFR should be placed at the end of the
 * list, to be processed again later.
 *
 * This test function guarantees that all predecessors of this CFR within the
 * loop have had their WCETO values calculated, so it is safe for the work
 * function to proceed. 
 * 
 * @param[in] cfrg the CFRG which the CFR comes from
 * @param[in] cfr the CFR being worked on
 * @param[in|out] userdata, an instance of LoopData
 *
 * @return true if the CFR may be processed by the work function
 */
static bool
lfs_loop_test(CFRG &cfrg, CFR *cfr, void *userdata) {
	LoopData *data = (LoopData *)userdata;
	WCETOFactory &fact = data->fact;
	CFR *head = data->head;
	CFRDemandMap &scratch = data->scratch;

	#define dout fact.dbg.buf << fact.dbg.start
	fact.dbg.inc("lfs_loop_test: ");
	dout << "loop " << *head << endl;
	dout << "CFR " << *cfr << endl;

	bool rv = true;
	if (head == cfr) {
		dout << "head element, all done" << endl;
		fact.dbg.flush(cout);
		fact.dbg.dec();
		return rv;
	}
	CFRList *preds = cfrg.preds(cfr);
	CFRList::iterator it;
	for (it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred = *it;
		dout << *cfr << " preceded by" << endl;
		dout << "    "  << *pred << endl;
		/* Check if the predecessor is in the loop */
		if (!cfrg.inLoop(head, pred)) {
			dout << *pred << " not in the same loop as " << endl;
			dout << "    " << *cfr << endl;
			/* It's not, don't consider it */
			continue;
		}
		dout << *pred << " in the same loop as " << endl;
		dout << "    " << *cfr << endl;
		if (scratch.present(pred)) {
			dout << *pred << " is present in the scratch table"
			     << endl;
			continue;
		}
		if (pred == head) {
			/* loop head will be handled by loopDemand */
			continue;
		}
		if (cfrg.isHead(cfrg.findNode(pred))) {
			dout << *pred << " is a loop head" << endl;
			CFRDemand *pred_dmnd = fact.loopDemand(pred);
			CFRDemand *copy = new CFRDemand(*pred_dmnd);
			dout << "inserting into scratch" << endl
			     << copy->str(fact.dbg.start) << endl;
			scratch.insert(pair<CFR*, CFRDemand*>(pred, copy));
			continue;
		}
		dout << *pred << " not in the scratch table, and not a loop head"
		     << endl;
		dout << *cfr << " is not ready." << endl;
		rv = false;
		break;
			
	}

	delete preds;
	fact.dbg.flush(cout);	
	fact.dbg.dec();
	return rv;
}
#undef dout

static void
lfs_loop_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	LoopData *data = (LoopData *)userdata;
	WCETOFactory &fact = data->fact;
	CFR *head = data->head;
	CFRDemandMap &scratch = data->scratch;

	#define dout fact.dbg.buf << fact.dbg.start
	fact.dbg.inc("lfs_loop_work: ");
	dout << "loop " << *head << endl;
	dout << "CFR " << *cfr << endl;

	if (head == cfr) {
		dout << "head element, all done" << endl;
		fact.dbg.flush(cout);
		fact.dbg.dec();
		return;
	}
	CFRDemand *dmnd = scratch.request(cfr);
	/* The demand for each CFR will be the WCET used to reach and
	 * execute that CFR stored in dmnd->getEXE(). The WCETOMap()
	 * is not used, at all. 
	 *
	 * Additionally, the Demand stores the ECBs for *this* CFR
	 * only.
	 */
	if (cfrg.isHead(cfrg.findNode(cfr))) {
		dout << *cfr << " is an embedded loop, recursing" << endl;
		fact.dbg.inc("lfs_loop(recursive)_work: ");
		CFRDemand *ldmnd = fact.loopDemand(cfr);
		int dcount = fact.dupeCount(ldmnd->getECBs());
		int iters = cfr->getIters(cfr->getInitial());
		int mlatency = cfr->getCache()->memLatency();
		dout << *cfr << " Total EXE for embedded loop: "
		     << ldmnd->getEXE() << endl;
		dout << *cfr << " ECBs: " << ldmnd->getECBs()
		     << " dupes: " << dcount << endl;
		dmnd->getEXE() = ldmnd->getEXE() + iters * dcount * mlatency;
		dout << *cfr << " total execution per iteration: "
		     << dmnd->getEXE() << endl;
		fact.dbg.dec();
	}

	dout << "(Work) Initial demand: " << endl
	     << dmnd->str(fact.dbg.start) << endl;
	
	uint32_t wceto=0;
	/* Perform the worst case execution time merger */
	CFRList *preds = cfrg.preds(cfr);
	dout << *cfr << " has " << preds->size() << " predecessors" << endl;
	for (CFRList::iterator it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred = *it;
		if (pred == head) {
			continue;
		}
		/* Check if the predecessor is in the loop */
		if (!cfrg.inLoop(head, pred)) {
			dout << *pred << " not in the same loop as " << endl;
			dout << "    " << *cfr << endl;
			/* It's not, don't consider it */
			continue;
		}
		CFRDemand *pdmnd = scratch.present(pred);
		if (pdmnd->getEXE() > wceto) {
			wceto = pdmnd->getEXE();
		}
	}
	delete preds;
	dmnd->getEXE() += wceto;

	dout << "(work) Demand after merging predecessors: " << endl
	     << dmnd->str(fact.dbg.start) << endl;
	fact.dbg.flush(cout);
	fact.dbg.dec();
	#undef dout
}


CFRDemand*
WCETOFactory::loopDemand(CFR *cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("loopDemand: ");
	dout << "begin " << *cfr << endl;
	CFRDemand *cfrd = _loopt.present(cfr);
	if (cfrd) {
		dout << *cfr << " is present, returning" << endl;
		dbg.flush(cout);
		dbg.dec();
		return cfrd;
	}
	cfrd = _loopt.request(cfr);
	cfrd->getWCETOMap().fillExe(cfr, _threads);
	dout << *cfr << " Initial demand" << endl
	     << cfrd->str(dbg.start) << endl;
	dout << "beginning search" << endl;

	CFRDemandMap scratch;
	LoopData *data = new LoopData(*this, cfr, scratch);
	CFRGLFS lfs(_cfrg, lfs_loop_filter, lfs_loop_test, lfs_loop_work,
		    data);
	lfs.search(cfr);
	dout << *cfr << " search complete" << endl;

	/*
	 * At the end of processing the CFRDemand for a loop head will
	 * contain values with different meanings than those of
	 * independent CFRs.
	 *
	 * loopt[loop_head].getExe() - the WCET of a single thread
	 * loopt[loop_head].getECBs() - the ECBs of all CRFs within
	 * the loop
	 * loopt[loop_head].getWCETOMap() - the cumulative map for the
	 * entire loop, used when the loop is treated as a single CFR
	 */
	for (CFRDemandMap::iterator it = scratch.begin(); it != scratch.end();
	     ++it) {
		cfrd->getECBs().merge(it->second->getECBs());
	}
	dout << *cfr << " total ECBs: " << endl;
	dout << cfrd->getECBs() << endl;
	
	uint32_t wceto=0;
	CFRList *preds = _cfrg.preds(cfr);
	for (CFRList::iterator it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred = *it;
		if (!_cfrg.inLoop(cfr, pred)) {
			continue;
		}
		CFRDemand *pdmnd = scratch.present(pred);
		dbg.inc("loopDemand pred: ");
		dbg.buf << pdmnd->str(dbg.start) << endl;
		/* Find the max WCET value */
		if (pdmnd->getEXE() > wceto) {
			wceto = pdmnd->getEXE();
		}
		dbg.dec();
	}
	delete preds;
	uint32_t iters = cfr->getIters(cfr->getInitial());
	cfrd->getEXE() += wceto;
	dout << *cfr << " total single thread WCETO: " << cfrd->getEXE() << endl;
	cfrd->getEXE() *= iters;

	/* Worst case execution time for each thread */
	ThreadWCETOMap &map = cfrd->getWCETOMap();
	for (uint32_t i = 1; i <= _threads; i++) {
		map[i] = cfrd->getEXE();
	}
	dout << *cfr << " per thread execution cost" << endl
	     << cfrd->str(dbg.start) << endl;

	/* Handle the cache loads */
	uint32_t mlatency = cfr->getCache()->memLatency();
	uint32_t dcount = dupeCount(cfrd->getECBs());
	uint32_t piterload = dcount * mlatency;
	/* First thread pays the cost of loading all the ECBs */
	map[1] += cfrd->getECBs().size() * mlatency;
	/* 2nd -> nth thread only load the duplicates */
	dout << *cfr << " per iteration load cost: " << piterload << endl;
	for (uint32_t i = 1 ; i <= _threads; i++) {
		map[i] += piterload * iters;
	}
	/* The first thread includes the per iteration load cost in
	   the initial load, remove the double counting */
	map[1] -= piterload; 
	dout << cfrd->getECBs()
	     << " ECB dupe count: " << dcount << endl;
	dout << "Demand after incorporating ECB loads initial/periter "
	     << cfrd->getECBs().size() * cfr->getCache()->memLatency() << "/"
	     << dcount * cfr->getCache()->memLatency() << endl
	     << cfrd->str(dbg.start) << endl;

	/* Values preserved in the loop table at this point
	 *   EXE worst case execution time for a single thread
	 *   ECBs the total ECBs accessed in the loop */

	for (CFRDemandMap::iterator it = scratch.begin(); it != scratch.end();
	     ++it) {
		delete it->second;
	}
	scratch.clear();

	/* Now it's time to use the predecessors to update our costs */
	preds = _cfrg.preds(cfr);
	for (CFRList::iterator it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred = *it;
		if (_cfrg.inLoop(cfr, pred)) {
			continue;
		}
		CFRDemand* pdmnd = NULL;
		if (_cfrg.isLoopPartCFR(pred)) {
			/* Must exist in the loop table */
			CFR *head = _cfrg.crown(pred);
			pdmnd = _loopt.present(head);
		} else {
			/* Must exist in the cfr table */
			pdmnd = _cfrt.present(pred);
		}
		scratch.insert(
		    pair<CFR*, CFRDemand*>(pred, new CFRDemand(*pdmnd)));
	}
	delete preds;

	maxMerge(*cfrd, scratch, true);
	dout << "Demand after adding predecessors" << endl
	     << cfrd->str(dbg.start) << endl;
	
	delete data;
	dout << "end " << *cfr << endl;
	dbg.flush(cout);
	dbg.dec();
	#undef dout
	return cfrd;
}

CFRDemand *
WCETOFactory::getDemand(CFR *cfr) {
	CFRDemand *dmnd;
	if (_cfrg.isLoopPartCFR(cfr)) {
		CFR *head = _cfrg.crown(cfr);
		dmnd = _loopt.present(head);
	} else {
		dmnd = _cfrt.present(cfr);
	}
	return dmnd;
}

uint32_t
WCETOFactory::dupeCount(ECBs &ecbs) {
	uint32_t rval = 0;
	ECBs u(ecbs);

	ecbs.sort();
	u.unique();
	u.sort();
	ECBs::iterator eit = ecbs.begin();
	for (ECBs::iterator uit = u.begin(); uit != u.end(); ++uit) {
		uint32_t value = *uit;
		uint32_t count = 0;
		while (eit != ecbs.end() && value == *eit) {
			count++;
			eit++;
		}
		if (count > 1) {
			rval += count;
		}
	}
	return rval;
}

void
WCETOFactory::maxMerge(CFRDemand &dem, CFRDemandMap &map, bool includeLoad) {
	#define dout dbg.buf << dbg.start
	string pfx = "maxMerge (";
	if (includeLoad) {
		pfx += "exe+load): ";
	} else {
		pfx += "exe): ";
	}
	dbg.inc(pfx);
	dout << "Merging " << map.size() << " sets of demands" << endl;
	if (map.size() == 0) {
		dbg.flush(cout);
		dbg.dec();
		return;
	}

	ThreadWCETOMap &twmap = dem.getWCETOMap();
	for (uint32_t i = 1; i <= _threads; i++) {
		CFRDemandMap::iterator it;
		uint32_t max_idx=0, max_wceto=0;
		ThreadWCETOMap *max_twmap=NULL;
		for (it = map.begin(); it != map.end(); ++it) {
			uint32_t t_idx=0, t_wceto=0;
			ThreadWCETOMap &twmap = it->second->getWCETOMap();
			t_wceto = maxEle(twmap, t_idx);
			if (t_wceto > max_wceto) {
				max_wceto = t_wceto;
				max_idx = t_idx;
				max_twmap = &twmap;
			}
		}
		(*max_twmap)[max_idx] = 0;
		dem.getWCETOMap()[i] += max_wceto;
	}
	dbg.flush(cout);	
	dbg.dec(); 
	#undef dout
}

uint32_t
WCETOFactory::maxEle(ThreadWCETOMap &twmap, uint32_t &idx) {
	#define dout dbg.buf << dbg.start
	dbg.inc("maxEle: ");
	
	uint32_t wceto=0;
	for (idx = 1; idx <= _threads; idx++) {
		wceto=twmap[idx];
		if (twmap[idx] != 0) {
			break;
		}
	}

	dbg.flush(cout);
	dbg.dec(); 
	#undef dout
	return wceto;
}

void
WCETOFactory::dumpCFRs() {
	#define dout dbg.buf << dbg.start
	dbg.inc("dumpCFRs: ");

	for (ListDigraph::NodeIt nit(_cfrg); nit != INVALID; ++nit) {
		CFR *cfr = _cfrg.findCFR(nit);

		dout << *cfr << endl;
		dbg.inc("dumpCFRS --: ");
		CFRDemand *d = NULL;
		if (_cfrg.isLoopPartCFR(cfr)) {
			CFR *head = _cfrg.crown(cfr);
			d = _loopt.present(head);
		} else {
			d = _cfrt.present(cfr);
		}
		dbg.buf << d->str(dbg.start) << endl;
		dbg.dec();
	}

	dbg.flush(cout);
	dbg.dec(); 
	#undef dout
}


/**
 * A function that returns if a CFR should remain a switching CFR or not.
 *
 * This function may only be called during the WCETO calculation. If
 * called outside of the WCETO calculation process the result is
 * unreliable. 
 *
 * @param[in] cfr the CFR in question
 *
 * @return true if the CFR should be as switching CFR, false if it
 * should be pass through
 */
#define dout dbg.buf << dbg.start
bool
WCETOFactory::shouldSwitch(CFR *cfr) {
	dbg.inc("shouldSwitch: " + cfr->str() + " " );
	bool rv = false;

	uint32_t B = cfr->getCache()->memLatency();
	ECBs *ecbs = cfr->getECBs();
	ecbs->sort();
	
	CFRList *preds = _cfrg.preds(cfr);
	if (preds->size() == 0) {
		dout << "initial CFR, switching." << endl;
		rv = true;
	}
	
	CFRList::iterator it;
	for (it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred_cfr = *it;
		if (_cfrg.isLoopPartCFR(pred_cfr)) {
			/* Loop exits are always switched */
			dout << "is a loop exit, switching." << endl;
			rv = true;
			break;
		}
		/* Merge the ECBs */
		ECBs u(*ecbs);
		ECBs *others = pred_cfr->getECBs();
		others->sort();
		u.merge(*others);
		delete others;

		/* To count only the 2+ occurences */
		uint32_t dupes = dupeCount(u);
		uint32_t pcost = dupes * B * _threads;
		if (pcost > _ctx_cost) {
			/* Found one that doesn't exceed the CTX cost */
			dout << *pred_cfr << " has too many conflicts, switching."
			     << endl;
			rv = true;
			break;
		}
	}
	delete preds;
	delete ecbs;

	if (rv == false) {
		dout << "pass through CFR" << endl;
	}
	
	dbg.dec();
	return rv;
}

class DFSData {
public:
	DFSData(WCETOFactory &f, CFRG &c) : fact(f), assigned(c) {
	}
	ListDigraph::NodeMap<bool> assigned;
	WCETOFactory &fact;
	ECBs eu;
	CFRList nexts;
};

/**
 * The masking function will decide if this CFR can be added to the
 * pass through region
 */
#undef dout
#define dout fact.dbg.buf << fact.dbg.start
static bool
dfs_mask(CFRG &cfrg, CFR *cfr, void *userdata) {
	DFSData *data = (DFSData *) userdata;
	WCETOFactory &fact = data->fact;
	ECBs u(data->eu);
	fact.dbg.inc("dfs_mask: ");
	dout << *cfr << " BEGIN" << endl;

	bool rv = true;
	if (cfrg.isHeadCFR(cfr)) {
		rv = false;
	}
	
	ListDigraph::Node cfr_node = cfrg.findNode(cfr);
	ListDigraph::InArcIt ait(cfrg, cfr_node);
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node pred_node = cfrg.source(ait);
		CFR *pred_cfr = cfrg.findCFR(pred_node);
		if (pred_cfr->getSwitching() == false) {
			/* One predecessor is a pass through, but
			   decided this CFR would not work. Cannot
			   change this CFR to a pass through node */
			rv = false;
			break;
		}
	}
	dout << *cfr << " END (" << rv << ")" << endl;
	fact.dbg.dec(); fact.dbg.flush(cout);
	return rv;
}

static bool
dfs_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	DFSData *data = (DFSData *) userdata;
	WCETOFactory &fact = data->fact;
	ListDigraph::Node cfrn = cfrg.findNode(cfr);
	uint32_t B = cfr->getCache()->memLatency();
	ECBs u(data->eu);

	fact.dbg.inc("dfs_work: ");
	dout << *cfr << " BEGIN" << endl;
	
	/*
	 * Check if the successors can be added to this pass through
	 * region
	 */
	ListDigraph::OutArcIt ait(cfrg, cfrn);
	fact.dbg.inc("succs: ");
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node succ_node = cfrg.target(ait);
		CFR *succ_cfr = cfrg.findCFR(succ_node);
		if (cfrg.isHead(succ_node) && cfrg.isTopHeadNode(succ_node)) {
			dout << *succ_cfr << " is a loop crown, stopping" << endl;
			continue;
		}
		if (data->assigned[succ_node]) {
			dout << *succ_cfr << " skipped already assigned" << endl;
			/* Already assigned switching */
			continue;
		}
		if (cfrg.isLoopExitCFR(succ_cfr)) {
			dout << *succ_cfr << " loop exit, assigned switching"
			     << endl;
			succ_cfr->setSwitching(true);
			data->assigned[succ_node] = true;
			continue;
		}
		
		ECBs *ecbs = cfr->getECBs();
		u.merge(*ecbs);
		delete ecbs;

		uint32_t dupes = fact.dupeCount(u);
		uint32_t pcost = dupes * B;
		dout << *succ_cfr << " pass through cost: " << pcost << endl;
		if (pcost < fact.getCTXCost()) {
			dout << *succ_cfr << " assigned pass through" << endl;
			succ_cfr->setSwitching(false);
		} else {
			dout << *succ_cfr << " assigned switching" << endl;
			succ_cfr->setSwitching(true);
			data->assigned[succ_node] = true;
		}
	}
	fact.dbg.dec();
	dout << *cfr << " END" << endl;
	fact.dbg.dec();
}

static bool
dfs_sel(CFRG &cfrg, CFR *cfr, void *userdata) {
	return false;
}
static bool
dfs_fin(CFRG &cfrg, CFR *cfr, void *userdata) {
	return false;
}
#undef dout


#define dout dbg.buf << dbg.start
void
WCETOFactory::switchPass() {
	dbg.inc("switchPass: ");

	CFRGTopSort tops(_cfrg);
	tops.sort(_cfrg.getInitial());
	pqueue_t::iterator pit;
	for (pit = tops.result.begin(); pit != tops.result.end(); ++pit) {
		ListDigraph::Node node = *pit;
		CFR *cfr = _cfrg.findCFR(node);
		if (_cfrg.isLoopPart(node)) {
			/* Node is part of a loop */
			if (!_cfrg.isTopHeadNode(node)) {
				/* Not the loop head */
				cfr->setSwitching(false);
				continue;
			}
			/* 
			 * Is a loop head, handle switching update for
			 * the entire loop
			 */
			continue;
		}
		DFSData data(*this, _cfrg);
		CFRGDFS dfs(_cfrg);
		dfs.setUserData(&data);
		dfs.setMask(dfs_mask);
		dfs.setWork(dfs_work);
		dfs.search(cfr);
	}
	
	
	dbg.dec(); dbg.flush(cout);
}

#undef dout
