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
	CFRFactory(CFG &cfg, Cache &cache) : _cfg(cfg), _cache(cache),
		_initial(cfg), _visited(cfg) {
		cfrg = new CFRG(cfg);
	}
	~CFRFactory();
	
	/* Gets the CFR of an instruction */
	CFR* getCFR(ListDigraph::Node cfg_node);

	map<ListDigraph::Node, CFR*> produce();
	/* Gets the CFRG a product of produce */ 
	CFRG *getCFRG() { return cfrg; }
	bool debugOn = false;
private:
	CFG &_cfg;
	Cache &_cache;
	CFRG *cfrg;
	
	ListDigraph::NodeMap<bool> _initial;
	ListDigraph::NodeMap<bool> _visited;
	/* Initial instruction in the CFG -> CFR */
	map<ListDigraph::Node, CFR*> _cfrs;

	void markLoops();
	/* Marks the first exit nodes from this nodes loop as initial
	 * CFR instructions */
	void markLoopExits(ListDigraph::Node node);
	list<ListDigraph::Node> addToConflicts(ListDigraph::Node cfr_initial,
	    ListDigraph::Node node, Cache &cache);
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
