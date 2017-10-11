#ifndef WCETO_FACTORY_H
#define WCETO_FACTORY_H

#include "CFRG.h"

class WCETOFactory {
public:
	WCETOFactory(CFRG &cfrg) : _cfrg(cfrg) { }
	void setPath(string path) {
		_path = path;
	}
	string getPath() {
		return _path;
	}
	void produce();
private:
	CFRG &_cfrg;
	string _path;
};

#endif /* WCETO_FACTORY_H */
