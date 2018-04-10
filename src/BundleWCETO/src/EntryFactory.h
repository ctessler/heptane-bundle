#ifndef ENTRY_FACTORY_H
#define ENTRY_FACTORY_H

#include "CFRG.h"

class EntryFactory {
public:
	EntryFactory(CFRG &cfrg) : _cfrg(cfrg) { }
	void setPath(string path) {
		_path = path;
	}
	string getPath() {
		return _path;
	}
	void produce();
	void produceAllSwitched();	
private:
	CFRG &_cfrg;
	string _path;
	CFRList *nextSwitches(ListDigraph::Node cfrg_node);
};

#endif /* ENTRY_FACTORY_H */
