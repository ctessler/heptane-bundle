#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "LemonCFG.h"
#include <map>
using namespace std;

class CFRFactory {
public:
	/**
	 * Converts a CFG (with CFR annotations) into CFRs
	 *
	 * The CFRs are themselves subsets of the original CFG, connectivity between 
	 *
	 * @param[in] cfg the LemonCFG which has had getCFRmembership() called on it
	 *
	 * @return <node -> CFR> where the node's belong to cfg, and
	 * the CFRs are new LemonCFGs
	 */
	static map<ListDigraph::Node, LemonCFG*> separateCFRs(LemonCFG &cfg);
};

#endif /* CFR_FACTORY */
