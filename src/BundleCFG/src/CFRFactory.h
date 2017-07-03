#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "Program.h"
#include "Node.h"
#include "Cache.h"
#include "Analysis.h"
#include "DotPrint.h"
#include "Generic/CallGraph.h"
using namespace cfglib;

class CFRFactory {
public:
	static int makeCFRG(Program* prog, string dir,
	    map<int, Cache*> &iCache, map<int, Cache*> &dCache);
private:
	static Program* BundleExtraction(Program* prog, Cache* cache);
	/**
	 * Extracts the Conflict Free Region starting at target.
	 *
	 * @param[in|out] Cfg the control flow graph the nodes within the
	 *	conflict free region will be added to 
	 * @param[in] parent the node preceding the target in a
	 *	conflict free region, may be NULL when there is no
	 *	parent. 
	 * @param[in] target the entry point for the new conflict free
	 *	region
	 * @param[in] cache the current cache (and state) to determine
	 *	where conflicts will occur
	 *
	 * @return a mapping of conflicts that includes the address
	 * 	and Node. A conflict occurs at the instruction level,
	 * 	the first element of the mapping is the address of the
	 * 	conflicting instruction. The second element is the
	 * 	Node from the original control flow graph containing
	 * 	that instruction. Each instruction of the map should
	 * 	be used as an entry point for subsequent conflict free
	 * 	regions.
	 */
	static map<t_address, Node*> extractNode(Cfg *cfg, Node* parent, Node* node, Cache *cache);
	
};

#endif /* CFR_FACTORY */
