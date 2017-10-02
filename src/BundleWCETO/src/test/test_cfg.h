#ifndef TEST_CFG_H
#define TEST_CFG_H

#include <lemon/core.h>
#include <lemon/list_graph.h>
using namespace lemon;

class CFG : public ListDigraph {
public:
	CFG() : ListDigraph() { }
private:
};

#endif
