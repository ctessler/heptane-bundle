#ifndef JPGFACTORY_H
#define JPGFACTORY_H

#include <string>
#include"DOTFactory.h"
using namespace std;

class JPGFactory {
public:
	JPGFactory(DOTFactory &dot);
	JPGFactory(string path);
	void setPath(string path);
	string getPath();
	void produce();
private:
	string _src_path;
	string _path;
};

#endif
