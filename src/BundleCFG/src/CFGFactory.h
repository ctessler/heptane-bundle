#ifndef CFGFACTORY_H
#define CFGFACTORY_H

#include "Program.h"
#include "DBG.h" 
#include "Node.h"
#include "Analysis.h"
#include "Generic/CallGraph.h"

#include "CFG.h"
#include <stack>
using namespace std;
using namespace cfglib;

/**
 * Converts a Heptane Program into a CFG
 */
class CFGFactory {
public:
	CFGFactory(Program *prog) : _prog(prog) {
		log.open("creation.log");
		mlog.open("makeCall.log");
  	}
	~CFGFactory() {
		log.close();
		mlog.close();
	}
	CFG* produce();
private:
	Program *_prog;
	ListDigraph::Node makeCall(CFG *cfg, ListDigraph::Node call_site, Node *node);
	ListDigraph::Node makeBB(CFG *cfg, const FunctionCall &call, Node *node);
	stringstream _debug;
	string _indent;

	void tagHead(CFG &cfg, ListDigraph::Node node,
		     ListDigraph::Node head, ListDigraph::NodeMap<int> &pathp);
	ListDigraph::Node loopDFS(CFG &cfg, ListDigraph::Node node,
				  ListDigraph::NodeMap<int> &pathp,
				  ListDigraph::NodeMap<bool> &visited,
				  unsigned int pos);
	void identifyLoops(CFG &cfg);
	void boundLoops(CFG &cfg, Cfg* hep_cfg);
	DBG dbg;
	ofstream log;
	ofstream mlog;

	void dmc1(CFG *cfg, ListDigraph::Node call_site, Node *node);
	void dmc2(CFG *cfg, ListDigraph::Node call_site, Node *node,
		  ListDigraph::Node first, ListDigraph::Node last);
	void dmc3(CFG *cfg, ListDigraph::Node call_site, Node *node,
		  vector<Node*> &succs);
	void dmc4(CFG *cfg, ListDigraph::Node call_site, Node *node,
		  Node *next);
	void dmc5(CFG *cfg, ListDigraph::Node call_site, Node *node,
		  Node *next);
	void dmc6(CFG *cfg, ListDigraph::Node call_site, Node *node,
		  ListDigraph::Node last, ListDigraph::Node succ_first);
	void dmc7(CFG *cfg, ListDigraph::Node call_site, Node *node,
		  ListDigraph::Node last);
	void dmc10(CFG *cfg, ListDigraph::Node call_site, Node *node,
		   ListDigraph::Node final);
};

#endif /* CFGFACTORY_H */
