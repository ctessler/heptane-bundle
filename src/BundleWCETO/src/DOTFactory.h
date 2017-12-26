#ifndef DOTFACTORY_H
#define DOTFACTORY_H

#include "CFG.h"
#include "CFR.h"
#include<fstream>
using namespace std;

/**
 * Generates DOT Files from CFG objects
 */
class DOTFactory {
public:
 	DOTFactory(CFG &cfg) : _cfg(cfg), _color(cfg), _cfr(cfg) { }
  
	/* Gets and sets the path of the DOT file */
	string getPath() { return _path; }
	void setPath(string path) { _path = path; }
	/* Gets and sets the cache used in producing the DOT file */
	Cache* getCache() { return _cache; }
	void setCache(Cache *cache) { _cache = cache; }
	/* Gets and sets the color of an individual node */
	string getColor(ListDigraph::Node node) { return _color[node]; }
	void setColor(ListDigraph::Node node, string color) { _color[node] = color; }

	void produce();
	void succ(ListDigraph::Node node, stack<ListDigraph::Node> &followers);
	void labelNodesCFR(CFR *cfr);
private:
	CFG const &_cfg;
	Cache *_cache = NULL;
	ListDigraph::NodeMap<string> _color;
	ListDigraph::NodeMap<string> _cfr;
	stringstream _debug;
	string _path, _indent;

	void loopDOT(ListDigraph::Node head, ofstream &os,
		     stack<ListDigraph::Node> &calls, stack<ListDigraph::Node> &subsq,
		     ListDigraph::NodeMap<bool> &visited);	
	ListDigraph::Node nodeDOT(ofstream &os, ListDigraph::Node node);
	string edgeDOT(ListDigraph::Node u, ListDigraph::Node port_u,
		       ListDigraph::Node v, ListDigraph::Node port_v);
	string nodeDOTstart(ListDigraph::Node node);
	string nodeDOTend(ListDigraph::Node node);
	string nodeDOTrow(ListDigraph::Node node);
	string nodeLabel(ListDigraph::Node node);
	
};


#endif /* DOTFACTORY_H */
