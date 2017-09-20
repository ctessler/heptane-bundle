#include"JPGFactory.h"

void
JPGFactory::produce() {
	string command = "dot -Tjpg " + _src_path + " > " + _path;
	system(command.c_str());
}
