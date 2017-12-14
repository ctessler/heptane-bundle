#include "WCETOFactory.h"

/**
 * List First Search filter function
 *
 * Returning true indicates that the CFR can be passed to the test function.
 */
static bool
lfs_top_filter(CFRG &cfrg, CFR *cfr, void *userdata) {
	/**
	 * Since this is the top level LFS, all nodes pass. 
	 */
	return true;
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
 * their WCETO values calculated, so it is safe for the work function to
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
	fact->dbg.inc("lfs_top_test: ");
	fact->dbg.buf << fact->dbg.start << "testing CFR " << *cfr << endl;
	fact->dbg.inc("lfs_top_test: ");	
	#define dout fact->dbg.buf << fact->dbg.start

	CFRWCETOMap &cfrtbl = fact->cfr_table();
	CFRWCETOMap &looptbl = fact->loop_table();

	
	bool rv=true;
	ListDigraph::Node cfrgi = cfrg.findNode(cfr);
	if (cfrg.isLoopPart(cfrgi)) {
		dout << *cfr << " is part of a loop, pre-check is different"
		     << endl;
	}
	for (ListDigraph::InArcIt iat(cfrg, cfrgi); iat != INVALID; ++iat) {
		ListDigraph::Node pred_node = cfrg.source(iat);
		CFR *pred_cfr = cfrg.findCFR(pred_node);
		dout << "preceding CFR " << *pred_cfr << endl;
		if (cfrg.isLoopPart(pred_node)) {
			if (cfrg.isLoopPart(cfrgi)) {
				continue;
			}
			/* Use the loop table */
			dout << *pred_cfr << " is part of a loop." << endl;
			CFR *outer = fact->outermostCFR(pred_cfr);
			if (!looptbl.present(outer)) {
				dout << *outer << " has not been computed, "
				    << "skipping." << endl;
				rv = false; break;
			}
			continue;
		}
		/* Otherwise it's time to check the singular CFRs */
		dout << *pred_cfr << " is an isolated CFR" << endl;
		if (!cfrtbl.present(pred_cfr)) {
			dout << *pred_cfr 
			     << " has not been computed, skipping." << endl;
			rv = false; break;
		}
	}
	
	dout << *cfr << " checked, precedessors are ";
	if (!rv) {
		fact->dbg.buf << "not ";
	}
	fact->dbg.buf << "completed" << endl;
	fact->dbg.flush(cout);
	fact->dbg.dec();
	fact->dbg.dec();

	/* change to rv when loop wceto has finished*/
	return rv;
}

/**
 * List First Work Function
 *
 * Performs the WCETO calculation for *this* CFR, if the CF
 *
 */
static void
lfs_top_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	WCETOFactory *fact = (WCETOFactory *) userdata;
	#define dout fact->dbg.buf << fact->dbg.start
	fact->dbg.inc("lfs_top_work: ");
	dout << "working CFR " << *cfr << endl;

	ListDigraph::Node cfrg_node = cfrg.findNode(cfr);
	if (cfrg.isHead(cfrg_node)) {
		dout << *cfr << " is a loop starter" << endl;
		fact->loopWCETO(cfr);
	} else {
		dout << *cfr << " is an isolated CFR" << endl;
		fact->isolatedWCETO(cfr);
	}
	
	dout << *cfr << " work completed" << endl;
	fact->dbg.flush(cout);
	fact->dbg.dec();	
	#undef dout
}

void
WCETOFactory::produce() {
	#define dout dbg.buf << dbg.start
	dbg.inc("WCETOFactory::produce ");
	dout << "BEGIN" << endl;
	
	CFRGLFS lfs(_cfrg, lfs_top_filter, lfs_top_test, lfs_top_work,
		    (void*) this);
	lfs.search(_cfrg.getInitialCFR());
	dout << "END" << endl;
	dbg.dec();
	#undef dout
}

uint32_t
WCETOFactory::value(CFR *cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("value: ");

	uint32_t wceto=0;
	ThreadWCETOMap *twmap = NULL;
	if (_cfrg.isLoopPart(_cfrg.findNode(cfr))) {
		dout << *cfr << " using the loop table" << endl;
		twmap = _loop_table.present(cfr);
		if (!twmap) {
			/* have to use the head instead */
			ListDigraph::Node cfg_node =
			    cfr->getHead(cfr->getInitial());
			cfr = _cfrg.findCFRbyCFGNode(cfg_node);
			twmap = _loop_table.present(cfr);
		}
	} else {
		twmap = _cfr_table.present(cfr);
		dout << *cfr << " using the cfr table " << endl
		     << twmap->str(dbg.start) << endl;		
	}
	if (!twmap) {
		dbg.flush(cout);
		throw runtime_error("value: Unable to find thread map");
	}
	wceto = twmap->wceto(_threads);
	dout << *cfr << " has wceto: " << wceto << endl;
	dbg.dec();
	dbg.flush(cout);
	#undef dout
	return wceto;
}

/**
 * Necessary pre-condition, all predecessors have had their WCETO values
 * calculated. 
 */
ThreadWCETOMap*
WCETOFactory::isolatedWCETO(CFR *cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("isolatedWCETO: ");
	dout << *cfr << " begin." << endl;

	ListDigraph::Node cfrg_node;
	ThreadWCETOMap *rval = _cfr_table.present(cfr);
	PredList pmaps; pmaps.clear();

	if (rval) {
		dout << *cfr << " already calculated." << endl;
		goto done;
	}
	rval = _cfr_table.request(cfr);
	rval->fill(cfr, _threads);
	dout << *cfr << " independent WCETO values" << endl;
	dbg.buf << rval->str(dbg.start) << endl;
	
	/* Now the predecessors */
	cfrg_node = _cfrg.findNode(cfr);
	for (ListDigraph::InArcIt iat(_cfrg, cfrg_node); iat != INVALID; ++iat) {
		ListDigraph::Node prev_node = _cfrg.source(iat);
		CFR *pred_cfr = _cfrg.findCFR(prev_node);
		dout << *cfr << dbg.cont << "preceded by " << *pred_cfr << endl;
		ThreadWCETOMap *twmap = NULL;
		if (_cfrg.isLoopPart(prev_node)) {
			/* use the loop table */
			CFR *outer = outermostCFR(pred_cfr);
			dout << "using loop table of " << *outer << endl;
			twmap = new ThreadWCETOMap(_loop_table.request(outer));
		} else {
			dout << "using cfr table" << endl;			
			twmap = new ThreadWCETOMap(_cfr_table.request(pred_cfr));
		}
		if (!twmap) {
			throw runtime_error("No predecessor calculation.");
		}
		pmaps.push_back(twmap);
	}
	addPreds(rval, pmaps);

done:
	for (PredList::iterator pit = pmaps.begin(); pit != pmaps.end(); ++pit) {
		delete (*pit);
	}
	pmaps.clear();
	
	dbg.buf << rval->str(dbg.start) << endl;
	dout << *cfr << " end." << endl;
	dbg.dec();
	#undef dout

	return rval;
}


class LoopData {
public:
	LoopData(WCETOFactory *f, CFR *c, CFRWCETOMap *m) {
		ld_fact = f; ld_cfr_head = c; ld_scratch = m;
	}
	WCETOFactory *ld_fact;
	CFR *ld_cfr_head;
	CFRWCETOMap *ld_scratch;
};

/**
 * List First Search filter function
 *
 * Returning true indicates that the CFR can be passed to the test function.
 */
static bool
lfs_loop_filter(CFRG &cfrg, CFR *cfr, void *userdata) {
	LoopData *ldata = (LoopData *)userdata;
	WCETOFactory *fact = ldata->ld_fact;
	CFR *head_cfr = ldata->ld_cfr_head;

	#define dout fact->dbg.buf << fact->dbg.start
	fact->dbg.inc("lfs_loop_filter: ");

	if (head_cfr == cfr) {
		dout << *cfr << " skip the head in test, not filter" << endl;
		fact->dbg.flush(cout); fact->dbg.dec();
		return true;
	}
	/**
	 * Prevent processing of CFRs outside of the loop
	 */
	bool rv = false;
	if (cfrg.inLoop(head_cfr, cfr)) {
		dout << "accepted CFR " << *cfr << fact->dbg.cont
		     << "head is " << *head_cfr << endl;
		rv = true;
	} else {
		dout << "rejected CFR " << *cfr << fact->dbg.cont
		     << "head is not " << *head_cfr << endl;
	}

	fact->dbg.dec();
	#undef dout
	return rv;
}

/**
 * Assistant function to lfs_loop_test and lfs_loop_work
 *
 * For a given cfr, and currently processing loop (head_cfr) if the
 * cfr is part of a loop embedded within the head_cfr's loop, find the
 * closest head to the head_cfr that contains the given cfr. 
 *
 * @param[in] cfrg the CFRG containing the CFRs
 * @param[in] head_cfr the bounding loop CFR
 * @param[in] cfr a cfr within a loop which is contained within the
 *   loop of the head_cfr
 *
 * @return the head of the loop closest to the head_cfr.
 */
static CFR*
leading_cfr(CFRG &cfrg, CFR* head_cfr, CFR* cfr) {
	
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
 * @param[in|out] userdata, should be a pointer to the WCETOFactory
 *
 * @return true if the CFR may be processed by the work function
 */
static bool
lfs_loop_test(CFRG &cfrg, CFR *cfr, void *userdata) {
	LoopData *ldata = (LoopData *)userdata;
	WCETOFactory *fact = ldata->ld_fact;
	CFRWCETOMap *scratch = ldata->ld_scratch;
	CFR *head_cfr = ldata->ld_cfr_head;
	
	#define dout fact->dbg.buf << fact->dbg.start
	fact->dbg.inc("lfs_loop_test: ");
	dout << "loop " << *head_cfr << endl;
	dout << "CFR " << *cfr << endl;

	if (head_cfr == cfr) {
		dout << "head element, all done" << endl;
		fact->dbg.flush(cout); fact->dbg.dec();
		return true;
	}
		
	PredList pmaps;
	/* Now the predecessors */
	ListDigraph::Node cfrg_node = cfrg.findNode(cfr);
	bool rv = true;
	for (ListDigraph::InArcIt iat(cfrg, cfrg_node); iat != INVALID; ++iat) {
		ListDigraph::Node prev_node = cfrg.source(iat);
		CFR *pred_cfr = cfrg.findCFR(prev_node);
		dout << *cfr << fact->dbg.cont << "preceded by " << *pred_cfr
		     << endl;

		/* Check if the predecessor is in the loop */
		if (!cfrg.inLoop(head_cfr, pred_cfr)) {
			dout << *pred_cfr << " not in the same loop as "
			     << fact->dbg.cont << *cfr << endl;
			/* It's not, don't consider it */
			continue;
		}
		dout << *pred_cfr << " in the same loop as " << endl
		     << fact->dbg.cont << *cfr << endl;
		if (scratch->present(pred_cfr)) {
			dout << *pred_cfr << " is present in the scratch table"
			     << endl;
			continue;
		}
		if (cfrg.isHead(cfrg.findNode(pred_cfr))) {
			dout << *pred_cfr << " is a loop head" << endl;
			ThreadWCETOMap *pred_map = fact->loopWCETO(pred_cfr);
			ThreadWCETOMap *copy = new ThreadWCETOMap(pred_map);
			dout << "inserting " << *copy << " into scratch" << endl;
			scratch->insert(
			    pair<CFR*,ThreadWCETOMap*>(pred_cfr, copy));
			continue;
		}
		dout << *pred_cfr << " not in the scratch table, and not a "
		     << "loop head." << fact->dbg.cont
		     << *cfr << " is not ready." << endl;
		rv = false;
		break;
	}

	dout << *cfr << " accepted, passing to work" << endl;
	fact->dbg.flush(cout);
	fact->dbg.dec();
	#undef dout
	return true;
}

static void
lfs_loop_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	/* 
	 * Things to remember.
	 *  All predecessors in the scratch table
	 *  All entries are in the loop, may be the head_cfr of another CFR.
	 */
	LoopData *ldata = (LoopData *)userdata;
	WCETOFactory *fact = ldata->ld_fact;
	CFRWCETOMap *scratch = ldata->ld_scratch;
	CFR *head_cfr = ldata->ld_cfr_head;
	
	#define dout fact->dbg.buf << fact->dbg.start
	fact->dbg.inc("lfs_loop_work: ");
	dout << "loop " << *head_cfr << endl;
	dout << "CFR " << *cfr << endl;

	if (head_cfr == cfr) {
		dout << "head element, all done" << endl;
		fact->dbg.flush(cout); fact->dbg.dec();
		return;
	}

	PredList pmaps;
	ListDigraph::Node cfr_node = cfrg.findNode(cfr);
	for (ListDigraph::InArcIt iat(cfrg, cfr_node); iat != INVALID; ++iat) {
		ListDigraph::Node prev_node = cfrg.source(iat);
		CFR *prev_cfr = cfrg.findCFR(prev_node);

		dout << *cfr << " is preceded by " << *prev_cfr << endl;
		ThreadWCETOMap *prev_map = scratch->present(prev_cfr);
		if (!prev_map) {
			throw runtime_error("Missing TWMap");
		}
		ThreadWCETOMap *copy = new ThreadWCETOMap(prev_map);
		dout << *prev_cfr << " has a predecessor map: " << endl
		     << copy->str(fact->dbg.start) << endl;
		pmaps.push_back(copy);
	}

	ThreadWCETOMap *cur_map = scratch->request(cfr);
	if (cfrg.isHead(cfr_node)) {
		dout << *cfr << " is an embedded loop, recursing" << endl;
		ThreadWCETOMap *loop_map = fact->loopWCETO(cfr);
		cur_map->fill(*loop_map);
	} else {
		dout << *cfr << " is a stand alone, filling" << endl;
		cur_map->fill(cfr, fact->getThreads());
		cfr->calcECBs();
	}
	
	dout << *cfr << " adding predecessors: " << endl
	     << cur_map->str(fact->dbg.start) << endl;
	fact->addPreds(cur_map, pmaps);

	for (PredList::iterator pit = pmaps.begin(); pit != pmaps.end(); ++pit) {
		delete (*pit);
	}
	pmaps.clear();

	dout << *cfr << " finished, result: " << cur_map << endl
	     << cur_map->str(fact->dbg.start) << endl;
		
	fact->dbg.flush(cout);
	fact->dbg.dec();
	#undef dout
	return;
}

/**
 * Necessary pre-condition, the individual CFRs preceding this loop head must be
 * computed. 
 */
ThreadWCETOMap *
WCETOFactory::loopWCETO(CFR *cfr) {
	#define dout dbg.buf << dbg.start
	dbg.inc("loopWCETO: ");

	ListDigraph::Node cfrg_node;
	ThreadWCETOMap *rval = _loop_table.present(cfr);
	if (rval) {
		cout << "loopWCETO: address of rval: " << rval << endl;
		dout << *cfr << " already calculated." << endl
		     << rval->str(dbg.start) << endl;
		dbg.flush(cout);
		dbg.dec();
		cout << "loopWCETO: returning safely" << endl;
		return rval;
	}

	rval = _loop_table.request(cfr);
	dout << *cfr << " new loop table " << rval << endl
	     << _loop_table.present(cfr)->str(dbg.start) << endl;
	rval->fill(cfr, _threads);
	cfr->calcECBs();
	dout << *cfr << " independent WCETO (without iterations)" << endl
	     << rval->str(dbg.start) << endl;

	CFRWCETOMap *scratch = new CFRWCETOMap();
	ThreadWCETOMap *copy = new ThreadWCETOMap(rval);
	scratch->insert(pair<CFR*, ThreadWCETOMap*>(cfr, copy));

	LoopData *ldata = new LoopData(this, cfr, scratch);
	
	CFRGLFS lfs(_cfrg, lfs_loop_filter, lfs_loop_test, lfs_loop_work,
		    ldata);
	lfs.search(cfr);

	/* Scratch is the data for *this* loop */
	dbg.inc("loopWCETO: ");
	dout << *cfr << " search complete, result is " << endl
	     << scratch->str(dbg.start) << endl;
	dbg.dec();

	/* Now bring this data into the loop table entry */
	PredList preds; /* Do not free the entries in here */
	findPreds(cfr, *scratch, preds);

	dout << "In loop predecessors to consider: " << preds.size() << endl;
	for (PredList::iterator pit=preds.begin(); pit != preds.end(); pit++) {
		ThreadWCETOMap *twmap = (*pit);
		dout << twmap->str(dbg.start) << endl;
	}
	addPreds(rval, preds);

	uint32_t iters = cfr->getIters(cfr->getInitial());
	if (preds.size() == 0) {
		/* the entire loop is contained in the CFR, this will
		 * only execute once, therefore the BRT is only paid
		 * only once
		 */
		dout << *cfr << " is a fully contained loop" << endl;
		rval->at(1) = cfr->loadCost() + cfr->exeCost() * iters;
		for (uint32_t i=2; i <= _threads; i++) {
			rval->at(i) = cfr->exeCost() * iters;
		}
	} else {
		ThreadWCETOMap::iterator twit;
		for (twit = rval->begin(); twit != rval->end(); ++twit) {
			twit->second = twit->second * iters;
		}
	}
	preds.clear();
	dout << "Map updated with iterations" << endl
	     << rval->str(dbg.start) << endl;
	

	/* Consider the predecessors that are not within the loop */
	findPreds(cfr, _cfr_table, preds);
	dout << "Out of loop individual CFRs to consider: "
	     << preds.size() << endl;
	findPreds(cfr, _loop_table, preds);
	dout << "Adding out of loop loop head CFRS: "
	     << preds.size() << endl;

	PredList destroyme;
	PredList::iterator pit;
	for (pit = preds.begin(); pit != preds.end(); ++pit) {
		destroyme.push_back(new ThreadWCETOMap(*pit));
	}
	preds.clear();
	
	addPreds(rval, destroyme);
	dout << *cfr << " Final table: " << rval << endl
	     << rval->str(dbg.start) << endl;
	for (pit = destroyme.begin(); pit != destroyme.end(); ++pit) {
		delete (*pit);
	}
	destroyme.clear();

	delete scratch;
	delete ldata;
	
	dbg.flush(cout);
	dbg.dec();
	#undef dout
	return rval;
}

bool
WCETOFactory::addPreds(ThreadWCETOMap *twmap, PredList& preds) {
	#define dout dbg.buf << dbg.start
	dbg.inc("addPreds: ");
	dout << "# of predecessors: " << preds.size() << endl;
	if (preds.size() == 0) {
		dbg.dec();
		return true;
	}
	for (uint32_t thread=1; thread <= _threads; thread++) {
		uint32_t max_wceto=0, max_idx=0;
		ThreadWCETOMap *max_map = NULL;
		PredList::iterator pit;
		for (pit = preds.begin(); pit != preds.end(); ++pit) {
			ThreadWCETOMap *cmap = (*pit);
			dbg.inc("addPreds: ");
			dout << "considering " << endl << cmap->str(dbg.start)
			     << endl;
			uint32_t cidx;
			uint32_t wcet = APmaxWCETO(*cmap, cidx);
			dout << "wcet and cidx: " << wcet << " " << cidx << endl;
			if (wcet > max_wceto) {
				max_wceto = wcet;
				max_map = cmap;
				max_idx = cidx;
			}
			dbg.dec();
		}
		(*twmap)[thread] += max_wceto;
		if (!max_map) {
			dbg.flush(cout);
			throw runtime_error("WCETOFactory::addPreds "
			    "unable to find a maximum entry");
		}
		(*max_map)[max_idx] = 0;
	}

	dbg.dec();
	#undef dout
	return true;
}

uint32_t
WCETOFactory::APmaxWCETO(ThreadWCETOMap &twmap, uint32_t &max_idx) {
	uint32_t max_wceto = 0;
	for (uint32_t idx=1; idx < twmap.size(); idx++) {
		if (twmap[idx - 1] != 0) {
			/*
			 * Zero value indicates that the thread before was
			 * selected
			 */
			return max_wceto;
		}
		if (twmap[idx] > max_wceto) {
			max_idx = idx;
			max_wceto = twmap[idx];
		}
	}
	return max_wceto;
}

uint32_t
WCETOFactory::findPreds(CFR* cfr, CFRWCETOMap& cwmap, PredList& preds) {
	#define dout dbg.buf << dbg.start
	dbg.inc("findPreds: ");
	
	uint32_t count=0;
	ListDigraph::Node cfrg_node =_cfrg.findNode(cfr);
	for (ListDigraph::InArcIt iat(_cfrg, cfrg_node); iat != INVALID; ++iat) {
		ListDigraph::Node prev_node = _cfrg.source(iat);
		CFR *prev_cfr = _cfrg.findCFR(prev_node);

		ThreadWCETOMap *twmap = cwmap.present(prev_cfr);
		if (twmap) {
			dout << *cfr << " found predecessor " << *prev_cfr << endl;
			dout << "adding map " << twmap->str(dbg.start) << endl;
			preds.push_back(twmap);
			count++;
		}
	}
	dbg.dec();
	#undef dout
	return count;
}

CFR*
WCETOFactory::outermostCFR(CFR *inloop) {
	#define dout dbg.buf << dbg.start
	dbg.inc("outermostCFR: ");

	dout << *inloop << " starting CFR" << endl;
	dbg.inc("outermostCFR: ");
	CFR *top = inloop;
	ListDigraph::Node cfrg_node = _cfrg.findNode(inloop);
	while (_cfrg.isLoopPart(cfrg_node)) {
		CFR *cfr = _cfrg.findCFR(cfrg_node);
		dout << *cfr << " next head" << endl;
		ListDigraph::Node cfg_node = cfr->getHead(cfr->getInitial());
		if (cfg_node != INVALID) {
			CFR *head = _cfrg.findCFRbyCFGNode(cfg_node);
			top = head;
			cfrg_node = _cfrg.findNode(head);
		} else {
			break;
		}
	}

	dbg.dec();
	dout << *top << " top most CFR" << endl;
	dbg.dec();
	#undef dout
	return top;
}

class ECBLoopData {
public:
	ECBLoopData (WCETOFactory *f, CFR *head, list<uint32_t>& ECBs)
		: fact(f), cfr_head(head), ecbs(ECBs) {
	}
	WCETOFactory *fact;
	CFR *cfr_head;
	list<uint32_t> &ecbs;
};
 
static bool
lfs_ecb_filter(CFRG &, CFR *cfr, void *userdata) {
	ECBLoopData *data = (ECBLoopData *) userdata;
	
	return true;
}
static bool
lfs_ecb_test(CFRG &, CFR *cfr, void *userdata) {
	ECBLoopData *data = (ECBLoopData *) userdata;
	
	return true;
}
static void
lfs_ecb_work(CFRG &, CFR *cfr, void *userdata) {
	ECBLoopData *data = (ECBLoopData *) userdata;	

}
 
list<uint32_t> *
WCETOFactory::loopECBs(CFR *loop_head) {
	list<uint32_t> *rval = new list<uint32_t>();

	ECBLoopData *data = new ECBLoopData(this, loop_head, *rval);

	delete data;
	return rval;
}
