#include "DOTFactory.h"

DOTFactory::produce() {
	ofstream dot(_path.c_str());

	dot << "digraph G {"
}
