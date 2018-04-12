#include"JPGFactory.h"

JPGFactory::JPGFactory(DOTFactory &dot) {
	_src_path = dot.getPath();
	setPath(dot.getPath());
};

JPGFactory::JPGFactory(string path) {
	_src_path = path;
	setPath(path);
}

void
JPGFactory::setPath(string path) {
	_path = path;
	size_t found = _path.rfind(".dot");
	if (found != string::npos) {
		_path.replace(found, 4, ".jpg");
	} else {
		_path += ".jpg";
	}
}

string
JPGFactory::getPath() {
	return _path;
}


void
JPGFactory::produce() {
	string command = "dot -Tjpg " + _src_path + " > " + _path + "&2>/dev/null";
	#ifdef DEBUG
	cout << "Producing JPG: " << _path << " with command " << endl
	     << "    " << command << endl;
	#endif
	system(command.c_str());
}
