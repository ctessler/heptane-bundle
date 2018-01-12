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

class NodeNodeMap : public map<ListDigraph::Node, ListDigraph::Node> {
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
		prdc.open("produce.log");
		preenlog.open("preen.log");
	}
	~CFRFactory();
	
	/* Gets the CFR of an instruction */
	CFR* getCFR(ListDigraph::Node cfg_node);

	NodeCFRMap produce();
	map<ListDigraph::Node, CFR*> produce_old();	
	/* Gets the CFRG a product of produce */ 
	CFRG *getCFRG() { return cfrg; }
	bool debugOn = false;
private:
	CFG &_cfg;
	Cache &_cache;
	CFRG *cfrg;
	
	ListDigraph::NodeMap<bool> _initial;
	ListDigraph::NodeMap<bool> _visited;

	/*
	 * Maps from the CFG node -> CFG node that begins the CFR the
	 * node is a part of
	 */ 
	NodeNodeMap _cfr_addr;
	
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
	ofstream xlog, bcfr, prdc, preenlog;
	bool newCFRTest(ListDigraph::Node node);

	void produce_prep();
	void produce_assign();
	void produce_create();
	void produce_link();		
	NodeList labelCFR(ListDigraph::Node entry, Cache &cache);
	NodeList expandCFR(ListDigraph::Node entry);
	
};

#endif /* CFR_FACTORY */
