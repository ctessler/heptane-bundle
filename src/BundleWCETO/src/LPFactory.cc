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
		lpf << "/* " << cfr->str() << "*/" << endl;
		lpf << makeWCETO(cfr) << endl;
		lpf << makePredThread(cfr) << endl;
		lpf << makeSuccThread(cfr) << endl;
		lpf << makeBin(cfr) << endl;
		lpf << endl;
	}

	lpf << "/* -- Variable Types -- */" << endl;
	lpf << "bin b";
	for (ListDigraph::NodeIt nit(*_cfrg); nit != INVALID; ++nit) {
		ListDigraph::Node cfr_node = nit;
		CFR *cfr = _cfrg->findCFR(cfr_node);
		lpf << "," << endl << "\t" << makeId(cfr) << ".b";
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
	_id << fstr << "_" << cfg->stringAddr(cfg_node);

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
			obj << "+ " << makeId(cfr) << ".c ";
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

	if (cfr->getHead(cfr_initial) != INVALID) {
		/* This CFR belongs to another loop */
		return SKIP;
	}
	if (cfr->isHead(cfr_initial)) {
		return LOOP;
	}
	return PLAIN;
}


string
LPFactory::makeWCETO(CFR *cfr) {
	stringstream wceto;
	string id = makeId(cfr);
	switch(findType(cfr)) {
	case PLAIN:
		wceto << id << ".c = "
		      << cfr->loadCost() << " " << id << ".b + "
		      << cfr->exeCost() << " " << id << ".t;";
		break;
	case LOOP:
		return makeLoopWCETO(cfr);
		break;
	case SKIP:
		wceto << id << ".c = 0;";
		break;
	case ERROR:
	default:
		throw runtime_error("Unknown type");
	}
	
	return wceto.str();
}

string
LPFactory::makeLoopWCETO(CFR *cfr) {
	string id = makeId(cfr);
	stringstream obj;

	obj << "/* Loop head WCETO */" << endl;

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
	obj << "/* ECBs: " << ecbs.str() << " */" << endl;
	uint32_t brt = cfr->getCache()->memLatency();
	uint32_t dupeC = ecbs.dupeCount();
	obj << id << ".c = ";
	obj << ecbs.size() * brt << " " << id << ".b"
	    << " + " << iters * dupeC * brt << " ";
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *in_cfr = *it;
		obj << endl << "\t+ ";
		if (in_cfr == cfr) {
			/* special case for the head of the loop */
			obj << iters * in_cfr->exeCost() << " " << id << ".t";
		} else {
			string in_id = makeId(in_cfr);
			obj << iters << " " << in_id << ".c ";
		}
	}
	obj << ";" << endl;
	obj << "/*    In loop WCET values */" << endl;
	/* WCET assignments for individual nodes of the loop */
	for (it = list->begin(); it != list->end(); ++it) {
		CFR *in_cfr = *it;
		if (in_cfr == cfr) {
			/* Newp, not the startnig node */
			continue;
		}
		string id = makeId(in_cfr);
		obj << id << ".c = " << in_cfr->exeCost()
		    << " " << id << ".t " << id << ".b;" << endl;
	}
	delete list;

	return obj.str();

}

string
LPFactory::makePredThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "/* Predecessor Threads */" << endl;
	ct << id << ".t = ";

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
		if (_cfrg->sameLoop(cfr, pred_cfr)) {
			string pred_id = makeId(pred_cfr);
			ct << endl << "\t";
			if (it != list->begin()) {
				ct << "+ ";
			}
			ct << pred_id << "." << id << ".t ";
			continue;
		}
		/* Not in the same loop level */
		if (_cfrg->isLoopPartCFR(pred_cfr)) {
			pred_cfr = _cfrg->crown(pred_cfr);
		}
	}
	delete list;
	ct << ";";
	return ct.str();
}

string
LPFactory::makeSuccThread(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);
	ct << "/* Successor Threads */" << endl;

	CFRList *list = NULL;
	switch(findType(cfr)) {
	case PLAIN:
		list = _cfrg->succs(cfr);
		break;
	case LOOP:
		list = _cfrg->exitOfCFR(cfr);
		break;
	case SKIP:
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
		if (it != list->begin()) {
			ct << "+ ";
		}
		ct << id << "." << succ_id << ".t " << endl;
	}
	ct << "\t= " << id << ".t;";
	return ct.str();
}

string
LPFactory::makeBin(CFR *cfr) {
	stringstream ct;
	string id = makeId(cfr);

	ct << "/* Selector */" << endl;
	ct << id << ".b <= " << id << ".t;";


	return ct.str();
}
