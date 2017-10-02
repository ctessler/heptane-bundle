#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "LemonCFG.h"
#include "CFG.h"
#include "CFR.h"
#include "CFRG.h"
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
	 *
	 * XXX-ct remove
	 */
	static map<ListDigraph::Node, LemonCFG*> separateCFRs(LemonCFG &cfg);

	CFRFactory(CFG &cfg, Cache &cache) : _cfg(cfg), _cache(cache),
		_initial(cfg), _visited(cfg) {
		cfrg = new CFRG(cfg);
	}
	~CFRFactory() {
		delete cfrg;
	}
	
	/* Gets the CFR of an instruction */
	CFR* getCFR(ListDigraph::Node cfg_node);

	map<ListDigraph::Node, CFR*> produce();
	/* Gets the CFRG a product of produce */ 
	CFRG *getCFRG() { return cfrg; }

	static bool test();
 private:
	CFG &_cfg;
	Cache &_cache;
	CFRG *cfrg;
	
	ListDigraph::NodeMap<bool> _initial;
	ListDigraph::NodeMap<bool> _visited;
	/* Initial instruction in the CFG -> CFR */
	map<ListDigraph::Node, CFR*> _cfrs;
	map<ListDigraph::Node, CFR*> _from_cfg_to_cfr;

	void markLoops();
	/* Marks the first exit nodes from this nodes loop as initial
	 * CFR instructions */
	void markLoopExits(ListDigraph::Node node);
	list<ListDigraph::Node> addToConflicts(ListDigraph::Node node, Cache &cache);
	bool conflicts(ListDigraph::Node node, Cache &cache);
	
	void visit(ListDigraph::Node node, bool yes=true);
	bool visited(ListDigraph::Node node);
	void visitClear();

	/* Adds a CFR starting at the given node */
	CFR* addCFR(ListDigraph::Node node);

	stringstream _debug;
	string _path, _indent;
};

#endif /* CFR_FACTORY */
