#ifndef LPFACTORY_H
#define LPFACTORY_H

#include "CFRG.h"
#include<fstream>
using namespace std;

/**
 * Generates DOT Files from CFG objects
 */
class LPFactory {
public:
	LPFactory(CFRG *cfrg, uint32_t threads, uint32_t ctx_cost, string path)	{
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
	string makeObjective();
	/* The constraint for the CFR's WCETO */
	string makeWCETO(CFR *cfr);
	string makeLoopWCETO(CFR *cfr);
	/* The constraints for the boolean selector variable */
	string makeBin(CFR *cfr);
	/* The sum of predecessors threads contributions */
	string makePredThread(CFR *cfr);
	/* The sum of successor threads contributions */	
	string makeSuccThread(CFR *cfr);
	
	wceto_type_t findType(CFR *cfr);
	
	CFRG *_cfrg;
	string _path;
	uint32_t _threads, _ctx_cost;
};


#endif /* LPFACTORY_H */
