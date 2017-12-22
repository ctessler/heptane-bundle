#include "WCETOFactory.h"

/**
 * List First Search filter function
 *
 * Returning true indicates that the CFR can be passed to the test function.
 */
static bool
lfs_top_filter(CFRG &cfrg, CFR *cfr, void *userdata) {
	WCETOFactory *fact = (WCETOFactory *) userdata;
	bool rv = true;
	#define dout fact->dbg.buf << fact->dbg.start
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
	#undef dout
	return rv;
}

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
	fact->dbg.inc("WCETOFactory::lfs_top_test ");

	ListDigraph::Node cfrgi = cfrg.findNode(cfr);
	CFRDemandMap &loopt = fact->loop_table();
	CFRDemandMap &cfrt = fact->cfr_table();
	bool rv = true;

	for (ListDigraph::InArcIt iat(cfrg, cfrgi); iat != INVALID; ++iat) {
		ListDigraph::Node pred_node = cfrg.source(iat);
		CFR *pred_cfr = cfrg.findCFR(pred_node);
		dout << "preceding CFR " << *pred_cfr << endl;
		if (cfrg.isLoopPart(pred_node)) {
			/* Preceding node is part of a loop */
			if (cfrg.inLoop(cfr, pred_cfr)) {
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
		dout << *pred_cfr << " is an isolated CFR" << endl;
		if (!cfrt.present(pred_cfr)) {
			dout << *pred_cfr 
			     << " has not been computed, skipping." << endl;
			rv = false; break;
		}
	}
	
	fact->dbg.flush(cout);
	fact->dbg.dec();
	#undef dout
	/* change to return rv after the work portion has been
	   completed */
	return rv;
}

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

void
WCETOFactory::produce() {
	#define dout dbg.buf << dbg.start
	dbg.inc("WCETOFactory::produce ");
	
	CFRGLFS lfs(_cfrg, lfs_top_filter, lfs_top_test, lfs_top_work,
		    (void*) this);
	lfs.search(_cfrg.getInitialCFR());
	dbg.dec();
	#undef dout
}

uint32_t
WCETOFactory::value(CFR *cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("WCETOFactory::value ");
	uint32_t wceto=0;

	CFRDemand *dmnd;
	if (_cfrg.isLoopPartCFR(cfr)) {
		CFR *head = _cfrg.crown(cfr);
		dmnd = _loopt.present(head);
	} else {
		dmnd = _cfrt.present(cfr);
	}
	wceto = dmnd->getWCETOMap().wceto(_threads);

	dbg.flush(cout);
	dbg.dec(); 
	#undef dout
	return wceto;
}

CFRDemand *
WCETOFactory::inDemand(CFR* cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("inDemand: ");
	CFRDemand *cfrd = _cfrt.present(cfr);
	if (cfrd) {
		dout << *cfr << " is present" << endl;
		return cfrd;
	}
	cfrd = _cfrt.request(cfr);

	ThreadWCETOMap &twmap = cfrd->getWCETOMap();
	twmap.fillExe(cfr, _threads);
	twmap[1] += cfr->loadCost();

	dout << *cfr << " base demandmap: " << endl
	     << cfrd->str(dbg.start) << endl;

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
	#undef dout
	return cfrd;
}

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
		dout << *cfr << " skip the head in test, not filter" << endl;
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
	#undef dout
	return rv;
}

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
	#undef dout
	return rv;
}

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
	dmnd->getWCETOMap().fillExe(cfr, fact.getThreads());
	if (cfrg.isHead(cfrg.findNode(cfr))) {
		dout << *cfr << " is an embedded loop, recursing" << endl;
		CFRDemand *ldmnd = fact.loopDemand(cfr);
		dmnd->getEXE() =
		    ldmnd->getEXE() * cfr->getIters(cfr->getInitial());
	}

	dout << "(Work) Initial demand: " << endl
	     << dmnd->str(fact.dbg.start) << endl;
	/* ECBs (for this CFR) are stored in the scratch table */

	uint32_t wceto=0;
	/* Perform the worst case execution time merger */
	CFRList *preds = cfrg.preds(cfr);
	for (CFRList::iterator it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred = *it;
		if (pred == head) {
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
	
	uint32_t wceto=0;
	CFRList *preds = _cfrg.preds(cfr);
	for (CFRList::iterator it = preds->begin(); it != preds->end(); ++it) {
		CFR *pred = *it;
		if (!_cfrg.inLoop(cfr, pred)) {
			continue;
		}
		CFRDemand *pdmnd = scratch.present(pred);
		dbg.inc("loopDemand pred: ");
		dout << pdmnd->str(dbg.start) << endl;
		/* Find the max WCET value */
		if (pdmnd->getEXE() > wceto) {
			wceto = pdmnd->getEXE();
		}
		dbg.dec();
	}
	delete preds;				    
	cfrd->getEXE() += wceto;

	/* Worst case execution time for each thread */
	ThreadWCETOMap &map = cfrd->getWCETOMap();
	for (uint32_t i = 1; i <= _threads; i++) {
		map[i] = cfrd->getEXE() * cfr->getIters(cfr->getInitial());
	}

	/* Handle the cache loads */
	/* All ECBs are loaded at least once, in the first thread */
	map[1] += cfrd->getECBs().size() * cfr->getCache()->memLatency();
	/* 2nd -> nth thread only load the duplicates */
	uint32_t dcount = dupeCount(cfrd->getECBs());
	for (uint32_t i = 2 ; i <= _threads; i++) {
		map[i] += dcount * cfr->getCache()->memLatency();
	}
	dout << cfrd->getECBs()
	     << " ECB dupe count: " << dcount << endl;
	dout << "Demand after incorporating ECB loads initial/periter "
	     << cfrd->getECBs().size() * cfr->getCache()->memLatency() << "/"
	     << dcount * cfr->getCache()->memLatency() << endl
	     << cfrd->str(dbg.start) << endl;

	/* Values preserved in the loop table at this point
	 *   EXE worst case execution time for a single thread
	 *   ECBs the total ECBs accessed in the loop */

	/* Now it's time to use the predecessors to update our costs */
	scratch.empty();
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