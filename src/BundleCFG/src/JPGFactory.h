#ifndef JPGFACTORY_H
#define JPGFACTORY_H

#include"DOTFactory.h"

class JPGFactory {
public:
	JPGFactory(DOTFactory &dot) {
		_src_path = dot.getPath();
		setPath(dot.getPath());
	};
	JPGFactory(DOTfromCFRG &cfrgdot) {
		_src_path = cfrgdot.getPath();		
		setPath(cfrgdot.getPath());
	}
	/* Gets and sets the path */
	void setPath(string path) {
		_path = path;
		size_t found = _path.rfind(".dot");
		if (found != string::npos) {
			_path.replace(found, 4, ".jpg");
		} else {
			_path += ".jpg";
		}
	}
	string getPath() { return _path; }
	void produce();
private:
	string _src_path;
	string _path;
};

#endif
