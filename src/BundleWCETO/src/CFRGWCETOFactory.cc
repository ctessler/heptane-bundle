#include"CFRGWCETOFactory.h"

std::ostream &operator<<(std::ostream& stream, const WCETOMap& wmap) {
	WCETOMap::const_iterator mit;
	uint32_t nthreads = wmap.size() - 1;
	stream << "Thread\tWCETO";
	for (mit = wmap.begin(); mit != wmap.end(); ++mit) {
		uint32_t threads = mit->first;
		uint32_t wceto = mit->second;
		stream << endl << threads << "/"<< nthreads << "\t" << wceto;
	}
	return stream;
}

uint32_t WCETOMap::wceto() {
	WCETOMap::iterator wit;
	uint32_t wceto = 0;
	for(wit = begin(); wit != end(); ++wit) {
		wceto += wit->second;
	}
	return wceto;
}

std::ostream &operator<<(std::ostream& stream, const CFRTable& cfrtable) {
	CFRTable::const_iterator cit;
	for (cit = cfrtable.begin(); cit != cfrtable.end(); ++cit) {
		CFR *cfr = cit->first;
		WCETOMap *wmap = cit->second;
		stream << *cfr << endl << "wmap : " << wmap << " " << *wmap;
	}
	return stream;
}

static WCETOMap *
inWMap(CFR *cfr, CFRTable &table) {
	CFRTable::iterator needle;
	needle = table.find(cfr);
	if (needle != table.end()) {
		return needle->second;
	}
	return NULL;
}


static WCETOMap *
findWMap(CFR *cfr, CFRTable &table) {
	WCETOMap *wmap = inWMap(cfr, table);
	if (wmap) {
		return wmap;
	}

	wmap = new WCETOMap;
	table[cfr] = wmap;
	return wmap;
}



CFRGWCETOFactory::~CFRGWCETOFactory() {
	CFRTable::iterator cursor;
	for (cursor = _cfrtable.begin(); cursor != _cfrtable.end(); cursor++) {
		delete cursor->second;
	}
	for (cursor = _looptable.begin(); cursor != _looptable.end(); cursor++) {
		delete cursor->second;
	}
}

static bool
produce_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	string prefix = "produce_work: ";
	CFRGWCETOFactory *wcetof = (CFRGWCETOFactory*) userdata;

	cout << "produce_work: " << *cfr << endl;
	ListDigraph::Node cfri = cfr->getInitial();
	if (cfr->isHead(cfri)) {
		cout << prefix << *cfr << " is a head" << endl;
		wcetof->LoopWCETO(cfr);
	} else {
		cout << prefix << *cfr << " is an isolated CFR" << endl;
		wcetof->CFRWCETO(cfr);
	}
}


void
CFRGWCETOFactory::produce() {
	CFRGDFS dfs(_cfrg);
	dfs.setUserData(this);
	dfs.setWork(produce_work);
	dfs.search(_cfrg.getInitialCFR());
}

/**
 * Makes sure that this CFR can be used as a predecessor for determining the
 * WCETO of a successor CFR
 *
 * @param[in] cfr the CFR under inspection
 *
 * side effects: updates the _cfrtable with entries for this CFR
 */
void
CFRGWCETOFactory::wceto_precheck(CFR *cfr) {
	WCETOMap *wmap = inWMap(cfr, _cfrtable);
	if (!wmap) {
		CFRWCETO(cfr);
	}
}

uint32_t
CFRGWCETOFactory::CFRWCETO(CFR *cfr) {
	cout << "CFRWCETO: begin " << *cfr << endl;
	/* Precondition check */
	WCETOMap *wmap = inWMap(cfr, _cfrtable);
	if (wmap) return wmap->wceto();

	/* No previous value, begin calculation */
	wmap = findWMap(cfr, _cfrtable);

	uint32_t threads = _nthreads;
	wmap->insert(make_pair(0, 0));
	for (uint32_t thread=1; thread <= threads; thread++) {
		uint32_t wcet = cfr->wcet(thread) - cfr->wcet(thread - 1);
		wmap->insert(make_pair(thread, wcet));
	}
	cout << "CRFWCETO: " << *cfr << " individual contribution" << endl
	     << *wmap << endl;

	CFRTable scratch_table;
	if (!justPreds(cfr, _cfrtable, _looptable, scratch_table)) {
		throw runtime_error("CFRWCETO: predecessor problem");
	}
	cout << "CFRWCETO: " << *cfr << " predecessors" << endl
	     << scratch_table << endl;
	if (!addMaxPreds(*wmap, scratch_table)) {
		throw runtime_error("CFRWCETO: max selection problem");
	}
	/* 
	 * wmap now has the WCETO for each thread of this CFR
	 * including the path to reach it, a pointer to it was stored
	 * in the _cfrtable earlier
	 */
	CFRTable::iterator cleanup;
	for (cleanup = scratch_table.begin(); cleanup != scratch_table.end();
	     cleanup++) {
		delete cleanup->second;
	}
	
no_pred:
	return wmap->wceto();
}

static bool
loop_sel(CFRG &cfrg, CFR *cfr, void *userdata) {
	loop_user_data_t *data = (loop_user_data_t *) userdata;
	CFR *head_cfr = data->lud_head_cfr;

	if (cfr == head_cfr) {
		return true;
	}
	
	return false;
}

static bool
loop_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	string prefix = "CFRGWCETFactory::loop_work ";
	cout << prefix << *cfr << endl;
	
	loop_user_data_t *data = (loop_user_data_t *) userdata;
	CFRTable *table = data->lud_scratch_tbl;
	uint32_t threads = data->lud_threads;

	WCETOMap *wmap = findWMap(cfr, *table);
	wmap->insert(make_pair(0, 0));
	for (uint32_t thread=1; thread <= threads; thread++) {
		uint32_t wcet = cfr->wcet(thread) - cfr->wcet(thread - 1);
		wmap->insert(make_pair(thread, wcet));
	}
	table->insert(make_pair(cfr, wmap));

	return true;
}

static bool
loop_mask(CFRG &cfrg, CFR *cfr, void *userdata) {
	loop_user_data_t *data = (loop_user_data_t *) userdata;
	CFR *head_cfr = data->lud_head_cfr;
	if (cfr == head_cfr) {
		return true;
	}

	if (cfr->isHead(cfr->getInitial())) {
		/* Embedded loop */
		return false;
	}

	/*
	 * Are the CFRs in the same loop (ie, do they have the same
	 * innermost loop head)
	 */
	if (cfrg.sameLoop(head_cfr, cfr)) {
		return true;
	}

	/* Head nodes are different */
	return false;
}

static bool
lfs_filt(CFRG &cfrg, CFR *cfr, void *userdata) {
	string prefix = "lfs_filt: ";
	cout << prefix << *cfr << endl;
	loop_user_data_t *data = (loop_user_data_t *) userdata;
	CFR *head_cfr = data->lud_head_cfr;

	if (!cfrg.inLoop(head_cfr, cfr)) {
		/*
		 * The CFR that begins the loop is not the loop this
		 * CFR belongs to
		 */
		cout << prefix << *cfr << " does not belong in loop "
		     << *head_cfr << endl;		
		return false;
	}
	return true;
}
static bool
lfs_test(CFRG &cfrg, CFR *cfr, void *userdata) {
	string prefix = "lfs_test: ";
	cout << prefix << *cfr << endl;	
	loop_user_data_t *data = (loop_user_data_t *) userdata;
	CFRGWCETOFactory *fact = data->lud_this;
	CFR *head_cfr = data->lud_head_cfr;

	/*
	 * Check that all predecessors have had their WCETO's
	 * calculated before "working" on this CFR
	 */
	ListDigraph::Node cfr_node = cfrg.findNode(cfr);
	ListDigraph::InArcIt ait(cfrg, cfr_node);
	CFRTable *scratch = data->lud_scratch_tbl;
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node pred_node = cfrg.target(ait);
		CFR *pred_cfr = cfrg.findCFR(pred_node);

		WCETOMap *pred_map;		
		if (fact->useLoopTable(cfr, pred_cfr)) {
			/* The predecessor begins a loop */
			pred_map = findWMap(pred_cfr, fact->looptable());
		} else {
			/* Another CFR in this loop */
			pred_map = findWMap(pred_cfr, *scratch);
		}
		if (!pred_map) {
			return false;
		}
	}
	return true;
}

static void
lfs_work(CFRG &cfrg, CFR *cfr, void *userdata) {
	string prefix = "lfs_work: ";
	loop_user_data_t *data = (loop_user_data_t *)userdata;
	CFRGWCETOFactory *fact = data->lud_this;
	CFRTable *scratch = data->lud_scratch_tbl;
	CFR *head_cfr = data->lud_head_cfr;

	cout << prefix << "headcfr: " << *head_cfr << endl
	     << prefix << "current cfr: " << *cfr << endl;
	if (head_cfr != cfr && cfrg.isHead(cfrg.findNode(cfr))) {
		cout << prefix << "current cfr belongs to another loop" << endl;
		/*
		 * The current CFR is a loop head, so it must have
		 * its LoopWCETO calculated first.
		 */
		fact->LoopWCETO(cfr);
		return;
	}

	WCETOMap *wmap = new WCETOMap();
	int threads = data->lud_threads;
	wmap->insert(make_pair(0, 0));
	for (int i = 1; i <= threads; i++) {
		uint32_t wcet = cfr->wcet(i) - cfr->wcet(i - 1);
		wmap->insert(make_pair(i, wcet));
	}
	cout << prefix << *cfr << " individual contribution" << endl
	     << *wmap << endl;
	/* wmap contains the WCETO for the isolated execution of this CFR */

	/* Special case, if this is the head of the loop its value can
	   only be calculated at the very end. */
	if (head_cfr == cfr) {
		cout << prefix << "handled the first CFR of the loop, returning"
		     << endl;
		(*scratch)[cfr] = wmap;
		return;
	}

	CFRTable local_table; // Local scratch pad
	if (!fact->justPreds(cfr, *scratch, fact->looptable(), local_table)) {
		throw runtime_error("lfs_work predecessor problem");
	}
	if (!fact->addMaxPreds(*wmap, local_table)) {
		throw runtime_error("lfs_work max selection problem");
	}
	/* Store the result in the scratch table for the search */
	(*scratch)[cfr] = wmap;

	cout << prefix << "returning, current table: " << endl
	     << *scratch << endl;
}

/**
 * Handles WCETO calculation for loops within the CFRG (user loops) 
 *
 * @param[in] cfr the CFR which is the loop head
 * @param[in] threads the number of threads to work with
 */
void
CFRGWCETOFactory::LoopWCETO(CFR *cfr) {
	string prefix = "LoopWCETO: ";

	/* Pre check */
	WCETOMap *lmap = inWMap(cfr, _looptable);
	if (lmap) {
		cout << prefix << *cfr << " already calculated as " << endl
		     << *lmap << endl;
		/* Already calculated, be done */
		return;
	}

	/* 
	 * cfr may be in a loop, but not a loop head. Calculation of loop costs
	 * begins at the head
	 */
	cout << prefix << "handling " << *cfr << endl;
	CFG *cfg = cfr->getCFG();
	if (!_cfrg.isHead(_cfrg.findNode(cfr))) {
		cout << prefix << "this call is not the start of the loop"
		     << " will use the CFG head" << endl;
		/* This CFR is not the head */
		ListDigraph::Node cfr_inode = cfr->getInitial();
		ListDigraph::Node cfg_inode = cfr->toCFG(cfr_inode);
		ListDigraph::Node cfg_head = cfg->getHead(cfg_inode);
		if (cfg_head == INVALID) {
			throw runtime_error("Node is not a head?");
		}
		cout << prefix << "CFG head: " << cfg->stringNode(cfg_head)
		     << endl;
		CFR *cfr_head = _cfrg.findCFRbyCFGNode(cfg_head);
		if (cfr_head == NULL) {
			throw runtime_error("Unable to find a loop head");
		}
		cout << prefix << "converted to CFR: " << *cfr_head << endl;
		/* Found the head of the loop, do the work there */
		LoopWCETO(cfr_head);
		WCETOMap *head_map = findWMap(cfr_head, _looptable);
		WCETOMap *copy = new WCETOMap(*head_map);
		/* Place the copy based on *this* CFR */
		_looptable.insert(make_pair(cfr, copy));
		return;
	}

	lmap = findWMap(cfr, _looptable);

	/* Step one calculate the individual member's of the loop WCETO's */
	CFRTable cfr_table; // Local scratch pad
	loop_user_data_t data;
	data.lud_scratch_tbl = &cfr_table;
	data.lud_this = this;
	data.lud_head_cfr = cfr;
	data.lud_threads = _nthreads;
	
	CFRGLFS lfs(_cfrg, lfs_filt, lfs_test, lfs_work, &data);
	cout << prefix << "LoopWCETO: LFS begin" << endl;
	lfs.search(cfr);
	cout << prefix << "LoopWCETO: LFS end" << endl;	

	cout << prefix << "LoopWCETO: CFRTable after looping" << endl
	     << "LoopWCETO: " << cfr_table << endl;
	
	/* Final result goes to the _looptable */
	ListDigraph::Node cfrgi = _cfrg.findNode(cfr);
	CFRTable pred_table;
	for (ListDigraph::InArcIt ait(_cfrg, cfrgi); ait != INVALID; ++ait) {
		ListDigraph::Node node = _cfrg.source(ait);
		CFR *pred_cfr = _cfrg.findCFR(node);
		pred_table[pred_cfr] = cfr_table[pred_cfr];
	}
	/*
	 * Some things to note at this point.
	 *   The WCETO tables for the predecessors include the execution cost of
	 *   the initial loop head, so the predecessors are the candidates for
	 *   the worst case cost.
	 */
	WCETOMap *wmap = findWMap(cfr, _looptable);
	for (uint32_t i=0; i <= _nthreads; i++) {
		(*wmap)[i] = 0;
	}
	addMaxPreds(*wmap, pred_table);

	CFRTable::iterator cit;
	for (cit = cfr_table.begin(); cit != cfr_table.end(); ++cit) {
		delete cit->second;
	}
}


bool
CFRGWCETOFactory::justPreds(CFR* cfr, CFRTable &singles, CFRTable &loops,
			    CFRTable &result) {
	result.clear();
	string prefix = "justPreds: ";
	ListDigraph::Node cfr_node = _cfrg.findNode(cfr);
	ListDigraph::InArcIt ait(_cfrg, cfr_node);
	for ( ; ait != INVALID; ++ait) {
		ListDigraph::Node pred_node = _cfrg.source(ait);
		CFR *pred_cfr = _cfrg.findCFR(pred_node);
		WCETOMap *pred_map;
		if (useLoopTable(cfr, pred_cfr)) {
			cout << prefix << *cfr << " preceded by loop "
			     << *pred_cfr << endl;
			LoopWCETO(pred_cfr);
			pred_map = findWMap(pred_cfr, loops);
		} else {
			cout << prefix << *cfr << " preceded by an individual "
			     << *pred_cfr << endl;
			pred_map = findWMap(pred_cfr, singles);
		}
		WCETOMap *pred_copy = new WCETOMap(*pred_map);
		result.insert(make_pair(pred_cfr, pred_copy));
	}
	return true;
fail:
	CFRTable::iterator cursor;
	for (cursor = result.begin(); cursor != result.end(); cursor++) {
		delete cursor->second;
	}
	return false;
}

bool
CFRGWCETOFactory::addMaxPreds(WCETOMap &wmap, CFRTable &preds) {
	cout << "addMaxPreds incoming table: " << endl << preds << endl;
	if (preds.size() == 0) {
		return true;
	}
	for (uint32_t i=1; i <= _nthreads; i++) {
		WCETOMap *max_map = NULL;
		uint32_t max_idx = 0, max_wceto = 0;
		cout << "addMaxpreds: Selecting max wcet for thread " << i
		     << endl;
		CFRTable::iterator tit;
		for (tit = preds.begin(); tit != preds.end(); ++tit) {
			WCETOMap *cur_map = tit->second;
			for (uint32_t j=1; j <= i; j++) {
				if ((*cur_map)[j] > max_wceto) {
					max_map = cur_map;
					max_idx = j;
					max_wceto = (*cur_map)[j];
				}
				if ((*cur_map)[j] != 0) {
					/* Haven't selected a thread at this 
					 * level can't go further */
					break;
				}
			}
		}
		if (max_map == NULL) {
			throw runtime_error("addMaxPreds: could not find "
					    "a WCETO value");
		}
		/* Add the current thread wcet to this node's */
		(wmap)[i] += max_wceto;
		/* Clear the selected entry from the scratch pad */
		(*max_map)[max_idx] = 0;
	}
	return true;
}


/**
 * To treat loops as a single node, some times the total WCETO must be used
 * instead of the WCETO from the node. This method returns true if for the cfr
 * preceded by pred_cfr when pred_cfr is the member of a loop that cfr is not a
 * member of.
 *
 * @param[in] cfr the CFR having it's WCETO calculated
 * @param[in] pred_cfr the CFR preceding it, unknown if it belongs to the loop
 *   of cfr or not
 *
 * @return true if the loop table should be used, false otherwise
 */
bool
CFRGWCETOFactory::useLoopTable(CFR *cfr, CFR *pred_cfr) {
	ListDigraph::Node cfrg_node = _cfrg.findNode(pred_cfr);
	if (!_cfrg.isHead(cfrg_node)) {
		/* Predecessor is not a loop head, so don't use the table */
		return false;
	}
	if (_cfrg.inLoop(pred_cfr, cfr)) {
		/* cfr is in the loop of the predecessor */
		return false;
	}

	cout << "useLoopTable: " << cfr->stringNode(cfr->getInitial())
	     << " is not in the same loop as "
	     << pred_cfr->stringNode(pred_cfr->getInitial()) << endl;
	return true;
}
