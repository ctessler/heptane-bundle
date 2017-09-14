#ifndef CFGFACTORY_H
#define CFGFACTORY_H

#include "Program.h"
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
	CFGFactory(Program *prog) : _prog(prog) { }
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
};

#endif /* CFGFACTORY_H */
