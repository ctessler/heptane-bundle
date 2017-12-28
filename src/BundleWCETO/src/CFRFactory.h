#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "LemonCFG.h"
#include "CFG.h"
#include "CFR.h"
#include "CFRG.h"
#include "DBG.h"
#include <map>
using namespace std;

class NodeList : public list<ListDigraph::Node> {
};

class NodeCFRMap : public map<ListDigraph::Node, CFR*> {
public:
	void replace(ListDigraph::Node, CFR*);
};

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
	/* Any instruction in CFG -> CFR */
	NodeCFRMap _cfg_to_cfr;

	void markLoops();
	/* Marks the first exit nodes from this nodes loop as initial
	 * CFR instructions */
	void markLoopExits(ListDigraph::Node node);
	list<ListDigraph::Node> addToConflicts(ListDigraph::Node cfr_initial,
	    ListDigraph::Node node, Cache &cache);
	bool conflicts(ListDigraph::Node node, Cache &cache);

	NodeList assignToConflicts(CFR *cfr, ListDigraph::Node cursor,
	    Cache &cache);
	CFRList buildCFR(CFR *cfr);
	void ensureArc(CFR* source, CFR* target);
	
	void visit(ListDigraph::Node node, bool yes=true);
	bool visited(ListDigraph::Node node);
	void visitClear();

	/* Adds a CFR starting at the given node */
	CFR* addCFR(ListDigraph::Node node);

	stringstream _debug;
	string _path, _indent;

	DBG dbg;
	bool newCFRTest(ListDigraph::Node node);
};

#endif /* CFR_FACTORY */
