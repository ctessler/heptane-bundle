#ifndef CFGREADWRITE_H
#define CFGREADWRITE_H

#include "CFG.h"
#include <sstream>
#include <iomanip>
#include <string>
using namespace std;


/**
 * Factories to read and write CFGs
 *
 * Usage:
 *   CFG cfg;
 *   ...
 *   CFGWriter cfgw(cfg);
 *   cfgrw.write("alpha.cfg");
 *
 *   ...
 *   CFGReader cfgr(cfg);
 *   cfgr.read("alpha.cfg");
 */

class CFGRWBase {
public:
	CFGRWBase(CFG &cfg) : _cfg(cfg) { }
protected:
	CFG &_cfg;
};

class CFGWriter : public CFGRWBase {
public:
	CFGWriter(CFG &cfg) : CFGRWBase(cfg) {}
	void write(string path);
private:
	string nodeHeader();
	string arcHeader();		
	string nodeString(ListDigraph::Node node);
	string arcString(ListDigraph::Arc arc);
	void doHeads(ofstream &ofile, ListDigraph::NodeMap<bool> &visited,
	    ListDigraph::Node head);
};

class CFGReader : public CFGRWBase {
public:
	CFGReader(CFG &cfg) : CFGRWBase(cfg) {}
	void read(string path);
private:
	void addNode(string addr, ifstream &ifile);
	bool addArc(ifstream &ifile);	
};

#endif /* CFGREADWRITE_H */
