#ifndef DOTFACTORY_H
#define DOTFACTORY_H

#include "CFG.h"

/**
 * Generates DOT Files from CFG objects
 */
public class DOTFactory {
public
	DOTFactory(CFG &cfg) : _cfg(cfg) { }
	/* Gets and sets the path of the DOT file */
	string getPath() { return _path; }
	void setPath(string path) { _path = path; }

	void produce();
private:
	CFG _cfg;
	stringstream _debug;
	string _path, _indent;
};

#endif /* DOTFACTORY_H */
