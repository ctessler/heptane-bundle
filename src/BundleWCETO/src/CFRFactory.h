#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "LemonCFG.h"
#include "CFG.h"
#include "CFR.h"
#include "CFRG.h"
#include "DBG.h"
#include "CFGDFS.h"
#include "CFGTopSort.h"
#include "PQueue.h"
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
		xlog.open("asstx.log");
		bcfr.open("buildcfr.log");
		prdc.open("cfrprdc.log");
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
	NodeCFRMap _cfrs;
	/* Any instruction in CFG -> CFR */
	NodeCFRMap _cfg_to_cfr;

	void markLoops();
	/* Marks the first exit nodes from this nodes loop as initial
	 * CFR instructions */
	void markLoopExits(ListDigraph::Node node);
 	bool conflicts(ListDigraph::Node node, Cache &cache);

	NodeList assignToConflicts(CFR *cfr, ListDigraph::Node cursor,
	    Cache &cache);
	void preen(ListDigraph::Node starter);
	CFRList buildCFR(CFR *cfr);
	CFRList CFRsuccs(CFR *cfr);
	void ensureArc(CFR* source, CFR* target);
	
	void visit(ListDigraph::Node node, bool yes=true);
	bool visited(ListDigraph::Node node);
	void visitClear();

	/* Adds a CFR starting at the given node */
	CFR* addCFR(ListDigraph::Node node);

	stringstream _debug;
	string _path, _indent;

	DBG dbg;
	ofstream xlog, bcfr, prdc;
	bool newCFRTest(ListDigraph::Node node);
};

#endif /* CFR_FACTORY */
