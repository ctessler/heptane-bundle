#include"LPIFactory.h"

void
LPIFactory::produce() {
	ofstream lpf(_path.c_str());

	lpf << makeObjective() << endl;

	lpf << "/* m = # of threads */" << endl;
	lpf << "m = " << _threads << ";" << endl;

	for (ListDigraph::NodeIt nit(*_cfrg); nit != INVALID; ++nit) {
		ListDigraph::Node cfr_node = nit;
		CFR *cfr = _cfrg->findCFR(cfr_node);
		switch (findType(cfr)) {
		case PLAIN:
			lpf << "/* PLAIN " << cfr->str() << "*/" << endl;
			lpf << makeWCETO(cfr) << endl;
			lpf << makePredThread(cfr) << endl;
			lpf << makeSuccThread(cfr) << endl;
			lpf << makeBin(cfr) << endl << endl;
			break;
		case LOOP:
			lpf << "/* LOOP HEAD " << cfr->str() << "*/" << endl;
			lpf << makeLoopWCETO(cfr) << endl;
			lpf << makeLoopPredThread(cfr) << endl;
			lpf << makeLoopSuccThread(cfr) << endl;
			lpf << makeLoopBin(cfr) << endl << endl;			
			break;
		case SKIP:
			lpf << "/* INNER " << cfr->str() << "*/" << endl;
			lpf << makeInnerWCETO(cfr) << endl;
			lpf << makeInnerPredThread(cfr) << endl;
			lpf << makeInnerSuccThread(cfr) << endl;
			lpf << makeInnerBin(cfr) << endl << endl;
			break;
		default:
			throw runtime_error("Unknown type");			
		}
		
	}

	lpf << "/* -- Variable Types -- */" << endl;
	lpf << "bin b";
	for (ListDigraph::NodeIt nit(*_cfrg); nit != INVALID; ++nit) {
		ListDigraph::Node cfr_node = nit;
		CFR *cfr = _cfrg->findCFR(cfr_node);
		lpf << "," << endl << "\t" << makeId(cfr) << ".b";
		if (_cfrg->isHeadCFR(cfr)) {
			lpf << "," << endl << "\t" << makeFalseId(cfr) << ".b";
		}
		
	}
	lpf << ";" << endl;
	lpf.close();
}

string
LPIFactory::makeId(CFR *cfr) {
	CFG *cfg = cfr->getCFG();
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_initial);

	FunctionCall call = cfg->getFunction(cfg_node);
	string fstr = call.str();
	replace(fstr.begin(), fstr.end(), ' ', '_');
	replace(fstr.begin(), fstr.end(), ':', '_');
	fstr.erase(std::remove(fstr.begin(), fstr.end(), ','), fstr.end());
	
	stringstream _id;
	_id << "n" << cfg->id(cfg_node) << "_" << cfg->stringAddr(cfg_node);

	return _id.str();
}

string
LPIFactory::makeFalseId(CFR *cfr) {
	CFG *cfg = cfr->getCFG();
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_initial);

	stringstream _id;
	_id << "f" << cfg->id(cfg_node) << "_" << cfg->stringAddr(cfg_node);

	return _id.str();
}

string
LPIFactory::makeObjective() {
	stringstream obj;
	obj << "max: 0 ";

	for (ListDigraph::NodeIt nit(*_cfrg); nit != INVALID; ++nit) {
		ListDigraph::Node cfr_node = nit;
		CFR *cfr = _cfrg->findCFR(cfr_node);

		switch(findType(cfr)) {
		case PLAIN:
			obj << "+ " << makeId(cfr) << ".c ";
			break;
		case LOOP:
			obj << "+ " << makeFalseId(cfr) << ".c ";
			break;
		case SKIP:
			break;
		}
	}
	obj << ";";

	return obj.str();
}


LPIFactory::wceto_type_t
LPIFactory::findType(CFR *cfr) {
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_initial);

	if (cfr->isHead(cfr_initial)) {
		return LOOP;
	}
	if (cfr->getHead(cfr_initial) != INVALID) {
		/* This CFR belongs to another loop */
		return SKIP;
	}
	return PLAIN;
}

/**
 * 
 *
 */
string
LPIFactory::makeWCETO(CFR *cfr) {
	switch(findType(cfr)) {
	case PLAIN:
		break;
	case LOOP:
	case SKIP:
	case ERROR:
	default:
		throw runtime_error("Unknown type");
	}

	stringstream wceto;
	string id = makeId(cfr);
	wceto << "\t/* WCETO */" << endl;
	wceto << "\t" << id << ".c = "
	      << cfr->loadCost() << " " << id << ".b + " /* Memory Load Cost */
	      << cfr->exeCost() << " " << id << ".t";    /* Execution cost */
	if (_ctx_cost > 0) {
		/* Context switch cost */		
		wceto << " + " << _ctx_cost << " " << id << ".t";
	}
	wceto << ";";

	return wceto.str();
}

string
LPIFactory::makeLoopWCETO(CFR *cfr) {
	switch(findType(cfr)) {
	case LOOP:
		break;
	case PLAIN:
	case SKIP:
	case ERROR:
	default:
		throw runtime_error("Unknown type");
	}
	string id = makeId(cfr);
	string fake = makeFalseId(cfr); 
	stringstream obj;
	obj << "\t/* Loop WCETO ";

	/* Get the maximum number of iterations */
	uint32_t iters = cfr->getIters(cfr->getInitial());
	/* Make a set of ECBs */
	ECBs *ecbs = getECBsOfLoop(cfr);
	obj << "ECBs[" << ecbs->size() << "]: " << ecbs->str() << " */" << endl;
	uint32_t brt = cfr->getCache()->memLatency();

	/* WCETO for the false head */
	obj << "\t" << fake << ".c = ";
	obj << ecbs->size() * brt << " " << fake << ".b"; // c.fn = gamma.fn * b.fn
	// I.n * ( sum of interior nodes WCETO )
	CFRList *list = _cfrg->inLoopOfCFR(cfr);
	CFRList::iterator it;
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *in_cfr = *it;
		string in_id = makeId(in_cfr);
		if (in_cfr != cfr && _cfrg->isHeadCFR(in_cfr)) {
			in_id = makeFalseId(in_cfr);
		}
		obj << endl << "\t\t+ " << iters << " " << in_id << ".c ";
	}
	obj << ";" << endl;
	obj << makeInnerWCETO(cfr) << endl;
	obj << makeInnerBin(cfr) << endl;
	delete list;
	delete ecbs;

	return obj.str();
}

string
LPIFactory::makeInnerWCETO(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ECBs *dupecbs = getECBsOfLoop(cfr);
	dupecbs->dupesOnly();
	ECBs *cfrecbs = cfr->getECBs();
	ECBs isect;
	set_intersection(dupecbs->begin(), dupecbs->end(),
			 cfrecbs->begin(), cfrecbs->end(), back_inserter(isect));
	ct << "\t/* G2 ECBs[" << dupecbs->size() << "] " << dupecbs->str() << " */"
	   << endl;
	ct << "\t/* CFR ECBs[" << cfrecbs->size() << "] " << cfrecbs->str() << " */"
	   << endl;
	ct << "\t/* isect ECBs[" << isect.size() << "] " << isect.str() << " */"
	   << endl;
	delete dupecbs;
	delete cfrecbs;
	      
	uint32_t brt = cfr->getCache()->memLatency();
	uint32_t ic = isect.size();
									   
	ct << "\t/* WCETO */" << endl << "\t";
	ct << id << ".c = "
	   << cfr->exeCost() << " " << id << ".t"; /* Execution cost */
	if (_ctx_cost > 0) {
		/* Context switch cost */		
		ct << " + " << _ctx_cost << " " << id << ".t";
	}
	uint32_t bi = brt * ic;
	if (ic > 0 && brt > 0) {
		/* per iteration penalty */		
		ct << " + " << bi << " " << id << ".b";
	}
	ct << ";";

	return ct.str();
}

string
LPIFactory::makePredThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "\t/* Predecessor Threads */" << endl;
	ct << "\t" << id << ".t = ";

	if (_cfrg->getInitialCFR() == cfr) {
		ct << "m";
	}
	/*
	 * Predecessors may be part of a collapsed node
	 */
	CFRList *list = _cfrg->preds(cfr);
	CFRList::iterator it = list->begin();
	for ( ; it != list->end(); ++it) {
		CFR *pred_cfr = *it;
		ct << endl << "\t\t";
		if (it != list->begin()) {
			ct << "+ ";
		}
		if (_cfrg->isHeadCFR(pred_cfr) ||
		    !_cfrg->sameLoop(cfr, pred_cfr)) {
			pred_cfr = _cfrg->crown(pred_cfr);
			ct << makeFalseId(pred_cfr) << "." << id << ".t";
			continue;
		}
		
		string pred_id = makeId(pred_cfr);
		ct << pred_id << "." << id << ".t";
	}
	delete list;
	ct << ";";
	return ct.str();
}

string
LPIFactory::makeLoopPredThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	string fake = makeFalseId(cfr);
	ct << "\t/* Loop (False Head) Predecessor Thread Limit */" << endl;
	ct << "\t" << fake << ".t = ";
	CFRList *list = _cfrg->preds(cfr);
	CFRList::iterator it = list->begin();
	for ( ; it != list->end(); ++it) {
		CFR *pred_cfr = *it;
		if (_cfrg->inLoop(cfr, pred_cfr)) {
			continue;
		}
		string pred_id = makeId(pred_cfr);
		ct << endl << "\t\t";
		if (it != list->begin()) {
			ct << "+ ";
		}
		ct << pred_id << "." << fake << ".t";
	}
	delete list;
	ct << ";" << endl;

	ct << "\t/* Loop Head Predecessor Thread Limit */" << endl;
	ct << "\t" << id << ".t = " << fake << ".t;";
	return ct.str();
}

string
LPIFactory::makeInnerPredThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "\t/* Inner Predecessor Thread Limit */" << endl;
	ct << "\t" << id << ".t = ";

	CFRList *list = _cfrg->preds(cfr);
	CFRList::iterator it = list->begin();
	CFR *head = _cfrg->getHead(cfr);
	for ( ; it != list->end(); ++it) {
		CFR *pred_cfr = *it;
		if (!_cfrg->inDerivedLoop(head, pred_cfr)) {
			/* Predecessor comes from outside of the loop */
			continue;
		}
		while (!_cfrg->inLoop(head, pred_cfr)) {
			/* Find the same loop level predecessor */
			pred_cfr = _cfrg->getHead(pred_cfr);
		}
		string pred_id = makeId(pred_cfr);
		if (pred_cfr != head && _cfrg->isHeadCFR(pred_cfr)) {
			pred_id = makeFalseId(pred_cfr);
		}
		ct << endl << "\t\t";
		if (it != list->begin()) {
			ct << "+ ";
		}
		ct << pred_id << "." << id << ".t";
	}
	delete list;
	ct << ";";
	return ct.str();
}

string
LPIFactory::makeSuccThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "\t/* Successor Threads */";

	CFRList *list = NULL;
	switch(findType(cfr)) {
	case PLAIN:
	case SKIP:
		list = _cfrg->succs(cfr);
		break;
	case LOOP:
		list = _cfrg->exitOfCFR(cfr);
		break;
	case ERROR:
	default:
		throw runtime_error("Unknown type");
	}
	if (!list) {
		return ct.str();
	}
	if (list->size() == 0) {
		return ct.str();
	}
	CFRList::iterator it;
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *succ = *it;
		string succ_id = makeId(succ);
		ct << endl << "\t";
		if (it != list->begin()) {
			ct << "\t+ ";
		}

		if (_cfrg->isHeadCFR(succ) ||
		    !_cfrg->sameLoop(cfr, succ)) {
			succ = _cfrg->crown(succ);
			ct << id << "." << makeFalseId(succ) << ".t" << endl;
			continue;
		}
		ct << id << "." << succ_id << ".t " << endl;
	}
	ct << "\t\t= " << id << ".t;";
	return ct.str();
}

string
LPIFactory::makeLoopSuccThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	string fake = makeFalseId(cfr);
	ct << "\t/* Loop (False Head) Successor Thread Limit */";
	CFRList *list = _cfrg->exitOfCFR(cfr);
	CFRList::iterator it = list->begin();
	for ( ; it != list->end(); ++it) {
		CFR *succ_cfr = *it;
		string succ_id = makeId(succ_cfr);
		ct << endl << "\t";
		if (it != list->begin()) {
			ct << " + ";
		}
		ct << fake << "." << succ_id << ".t";
	}
	if (list->size() > 0) {
		ct << endl << "\t\t= " << fake << ".t;";
	}
	delete list;
	int count=0;
	list = _cfrg->succs(cfr);
	ct << "\t/* Loop Head Node Successor Thread Limit */";
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *succ_cfr = *it;
		if (_cfrg->getHead(succ_cfr) != cfr) {
			continue;
		}
		++count;
		string succ_id = makeId(succ_cfr);
		if (_cfrg->isHeadCFR(succ_cfr)) {
			succ_id = makeFalseId(succ_cfr);
		}
		ct << endl << "\t";
		if (it != list->begin()) {
			ct << " + ";
		}
		ct << id << "." << succ_id << ".t";
	}
	if (count > 0) {
		ct << endl << "\t\t= " << id << ".t;";
	}
	delete list;
	
	return ct.str();
}

string
LPIFactory::makeInnerSuccThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);

	ct << "\t/* Successor (inner) Threads */";

	CFR *head = _cfrg->getHead(cfr);
	CFRList *list = _cfrg->succs(cfr);
	CFRList::iterator it;
	int count = 0;
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *succ = *it;
		string succ_id = makeId(succ);
		if (_cfrg->getHead(succ) != head) {
			/* Outside of this loop */
			continue;
		}
		count++;
		if (_cfrg->isHeadCFR(succ)) {
			succ_id = makeFalseId(succ);
		}
		ct << endl << "\t";
		if (it != list->begin()) {
			ct << "\t+ ";
		}
		ct << id << "." << succ_id << ".t " << endl;
	}
	if (count > 0) {
		ct << "\t\t= " << id << ".t;";
	}
	return ct.str();
}

string
LPIFactory::makeBin(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);

	ct << "\t/* Selector */" << endl << "\t";
	ct << id << ".b <= " << id << ".t;";


	return ct.str();
}

string
LPIFactory::makeLoopBin(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	string fake = makeFalseId(cfr);

	ct << "\t/* Selector (False Head) */" << endl << "\t";
	ct << fake << ".b <= " << fake << ".t;";

	return ct.str();
}

string
LPIFactory::makeInnerBin(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "\t/* Selector (Inner) */" << endl << "\t";
	ct << id << ".b <= " << id << ".t;";

	return ct.str();
}


/**
 * Caller must free return value
 *
 * Returns the gamma(2) set of the loop to which cfr belongs
 */
ECBs *
LPIFactory::getECBsOfLoop(CFR *cfr) {
	/* Make a set of ECBs */
	ECBs *ecbs = new ECBs();

	CFR *loopHead = cfr;
	if (!_cfrg->isHeadCFR(cfr)) {
		loopHead = _cfrg->getHead(cfr);
	} 
	CFRList *list = _cfrg->inLoopOfCFR(loopHead);
	CFRList::iterator it;
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *in_cfr = *it;

		/* Collect the ECBS from this CFR */
		in_cfr->calcECBs();		
		ECBs *in_ecbs = in_cfr->getECBs();
		ecbs->insert(ecbs->begin(), in_ecbs->begin(), in_ecbs->end());
		ecbs->sort();
		delete in_ecbs;
	}
	return ecbs;
}
