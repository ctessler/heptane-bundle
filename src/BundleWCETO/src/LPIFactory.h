#ifndef LPIFACTORY_H
#define LPIFACTORY_H

#include "CFRG.h"
#include<fstream>
using namespace std;

/**
 * Generates DOT Files from CFG objects
 */
class LPIFactory {
public:
	LPIFactory(CFRG *cfrg, uint32_t threads, uint32_t ctx_cost, string path) {
		_cfrg = cfrg;
		setPath(path);
		setThreads(threads);
		setCTXCost(ctx_cost);
	}
	/* Gets and sets the path of the lp_solve file */
	string getPath() { return _path; }
	void setPath(string path) { _path = path; }

	uint32_t getThreads() { return _threads; }
	void setThreads(uint32_t t) { _threads = t; }

	uint32_t getCTXCost() { return _ctx_cost; }
	void setCTXCost(uint32_t c) { _ctx_cost = c; }
	
	void produce();

private:
	typedef enum {
		ERROR=0,
		PLAIN,
		LOOP,
		SKIP
	} wceto_type_t;

	/* Returns an identifier for a CFR */
	string makeId(CFR *cfr);
	string makeFalseId(CFR *cfr);	
	string makeObjective();
	/* The constraint for the CFR's WCETO */
	string makeWCETO(CFR *cfr);
	string makeLoopWCETO(CFR *cfr);
	string makeInnerWCETO(CFR *cfr);
	/* The constraints for the boolean selector variable */
	string makeBin(CFR *cfr);
	string makeLoopBin(CFR *cfr);
	string makeInnerBin(CFR *cfr);	
	/* The sum of predecessors threads contributions */
	string makePredThread(CFR *cfr);
	string makeLoopPredThread(CFR *cfr);
	string makeInnerPredThread(CFR *cfr);
	/* The sum of successor threads contributions */	
	string makeSuccThread(CFR *cfr);
	string makeLoopSuccThread(CFR *cfr);
	string makeInnerSuccThread(CFR *cfr);
	
	wceto_type_t findType(CFR *cfr);
	ECBs *getECBsOfLoop(CFR *cfr);
	
	CFRG *_cfrg;
	string _path;
	uint32_t _threads, _ctx_cost;
};


#endif /* LPIFACTORY_H */
