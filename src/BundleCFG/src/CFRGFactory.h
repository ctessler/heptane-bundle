#ifndef CFRG_FACTORY
#define CFRG_FACTORY
#include "LemonCFG.h"
#include "LemonCFR.h"
#include "CFRFactory.h"
#include "Cache.h"

class CFRGFactory {
public:
	CFRGFactory(LemonCFG &cfg, Cache &cache);
	~CFRGFactory();
	/**
	 * Converts the CFG into a CFRG
	 *
	 * Assumes that the CFG has not yet been manipulated.
	 *
	 * Proper usage:
	 *     LemonCFG cfg;
	 *     ...
	 *     CFRGFactory cfrgf(cfg, cache);
	 *     cfrgf.run();
	 *
	 * Improper usage:
	 *     LemonCFG cfg;
	 *     ...
	 *     cfg.cacheAssign(cache);
	 *     cfg.getCFRMembership(cache);
	 *     CFRGFactory cfrgf(cfg, cache);
	 */
	LemonCFRG* run();
	/**
	 * Gets the CFR successors of the CFR, nodes must be from the original CFG.
	 *
	 * @return those CFRs one-edge away in the CFRG from cfr
	 */
	list<ListDigraph::Node> succ(LemonCFG &cfg, ListDigraph::Node node);
private:
	LemonCFG &_cfg;
	Cache &_cache;
	map<ListDigraph::Node, LemonCFG*> _cfrs;
	map<ListDigraph::Node, LemonCFG*>::iterator _cfrit;	
};

#endif /* CFRG_FACTORY */
