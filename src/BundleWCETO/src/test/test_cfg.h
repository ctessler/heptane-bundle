#ifndef TEST_CFG_H
#define TEST_CFG_H


#include <lemon/core.h>
#include <lemon/list_graph.h>

#include <string>

#include "BundleTypes.h"
#include "FunctionCall.h"

using namespace lemon;
using namespace std;

class CFG : public ListDigraph {
public:
	CFG();
private:
	/* Every instruction belongs to a function, functions are identified by
	   their calling address and their name */
	ListDigraph::NodeMap<FunctionCall> _function;
	
};

#endif
