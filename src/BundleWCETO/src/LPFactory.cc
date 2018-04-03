#include"LPFactory.h"

void
LPFactory::produce() {
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

	lpf << "int i";
	for (ListDigraph::NodeIt nit(*_cfrg); nit != INVALID; ++nit) {
		ListDigraph::Node cfr_node = nit;
		CFR *cfr = _cfrg->findCFR(cfr_node);
		lpf << "," << endl << "\t" << makeId(cfr) << ".c";
		lpf << "," << endl << "\t" << makeId(cfr) << ".t";		
		if (_cfrg->isHeadCFR(cfr)) {
			lpf << "," << endl << "\t" << makeFalseId(cfr) << ".c";
			lpf << "," << endl << "\t" << makeFalseId(cfr) << ".t";			
		}
		
	}
	lpf << ";" << endl;

	lpf.close();
}

string
LPFactory::makeId(CFR *cfr) {
	CFG *cfg = cfr->getCFG();
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_initial);

	

	FunctionCall call = cfg->getFunction(cfg_node);
	string fstr = call.str();
	replace(fstr.begin(), fstr.end(), ' ', '_');
	replace(fstr.begin(), fstr.end(), ':', '_');
	fstr.erase(std::remove(fstr.begin(), fstr.end(), ','), fstr.end());
	#if 0
	fstr.erase(std::remove(fstr.begin(), fstr.end(), '['), fstr.end());
	fstr.erase(std::remove(fstr.begin(), fstr.end(), ']'), fstr.end());
	#endif 
	
	stringstream _id;
	#if 0
	_id << fstr << "_" << cfg->stringAddr(cfg_node);
	#endif
	_id << "n" << cfg->id(cfg_node) << "_" << cfg->stringAddr(cfg_node);

	return _id.str();
}

string
LPFactory::makeFalseId(CFR *cfr) {
	CFG *cfg = cfr->getCFG();
	ListDigraph::Node cfr_initial = cfr->getInitial();
	ListDigraph::Node cfg_node = cfr->toCFG(cfr_initial);

	stringstream _id;
	_id << "f" << cfg->id(cfg_node) << "_" << cfg->stringAddr(cfg_node);

	return _id.str();
}

string
LPFactory::makeObjective() {
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


LPFactory::wceto_type_t
LPFactory::findType(CFR *cfr) {
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
LPFactory::makeWCETO(CFR *cfr) {
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
	      << cfr->loadCost() << " " << id << ".b + "
	      << cfr->exeCost() << " " << id << ".t;";

	return wceto.str();
}

string
LPFactory::makeLoopWCETO(CFR *cfr) {
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
	ECBs ecbs;
	
	CFRList *list = _cfrg->inLoopOfCFR(cfr);
	CFRList::iterator it;
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *in_cfr = *it;

		/* Collect the ECBS from this CFR */
		in_cfr->calcECBs();		
		ECBs *in_ecbs = in_cfr->getECBs();
		ecbs.insert(ecbs.begin(), in_ecbs->begin(), in_ecbs->end());
		ecbs.sort();
		delete in_ecbs;
	}
	obj << "ECBs[" <<ecbs.size() << "]: " << ecbs.str() << " */" << endl;
	uint32_t brt = cfr->getCache()->memLatency();
	uint32_t dupeC = ecbs.dupeCount();
	obj << "\t" << fake << ".c = ";
	obj << ecbs.size() * brt << " " << fake << ".b + "
	    << iters * dupeC * brt << " ";
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *in_cfr = *it;
		obj << endl << "\t\t+ ";
		string in_id = makeId(in_cfr);
		obj << iters << " " << in_id << ".c ";
	}
	obj << ";" << endl;
	obj << makeInnerWCETO(cfr);
	delete list;

	return obj.str();
}

string
LPFactory::makeInnerWCETO(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "\t/* WCETO */" << endl << "\t";
	ct << id << ".c = "
	   << cfr->exeCost() << " " << id << ".t " << id << ".b;";

	return ct.str();
}

string
LPFactory::makePredThread(CFR *cfr) {
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
LPFactory::makeLoopPredThread(CFR *cfr) {
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
LPFactory::makeInnerPredThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "\t/* Inner Predecessor Thread Limit */" << endl;
	ct << "\t" << id << ".t = ";
	CFRList *list = _cfrg->preds(cfr);
	CFRList::iterator it = list->begin();
	CFR *head = _cfrg->getHead(cfr);
	for ( ; it != list->end(); ++it) {
		CFR *pred_cfr = *it;
		if (!_cfrg->inLoop(head, pred_cfr)) {
			continue;
		}
		string pred_id = makeId(pred_cfr);
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
LPFactory::makeSuccThread(CFR *cfr) {
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
LPFactory::makeLoopSuccThread(CFR *cfr) {
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
	
	return ct.str();
}

string
LPFactory::makeInnerSuccThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);

	return ct.str();
}

string
LPFactory::makeBin(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);

	ct << "\t/* Selector */" << endl << "\t";
	ct << id << ".b <= " << id << ".t;";


	return ct.str();
}

string
LPFactory::makeLoopBin(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	string fake = makeFalseId(cfr);

	ct << "\t/* Selector (False Head) */" << endl << "\t";
	ct << fake << ".b <= " << fake << ".t;" << endl;
	ct << "\t/* Selector */" << endl << "\t";	
	ct << id << ".b <= " << id << ".t;";

	return ct.str();
}

string
LPFactory::makeInnerBin(CFR *cfr) {
	stringstream ct;

	ct << "\t/* Selector (Inner) */";

	return ct.str();
}
