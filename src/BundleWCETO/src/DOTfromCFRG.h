#ifndef DOTFROMCFRG_H
#define DOTFROMCFRG_H

#include "CFRG.h"
#include "WCETOFactory.h"

class DOTfromCFRG {
public:
	DOTfromCFRG(CFRG &cfrg, WCETOFactory &fact)
		: _cfrg(cfrg), _fact(fact) { }
	/* Gets and sets the path */
	string getPath() { return _path; }
	void setPath(string path) { _path = path; }

	void produce(unsigned int threads=1);
private:
	CFRG &_cfrg;
	WCETOFactory &_fact;
	string _path;
};

#endif
