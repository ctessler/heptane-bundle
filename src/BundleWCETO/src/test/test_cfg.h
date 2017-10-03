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

	/**
	 * Override to protect against incorrect node addition
	 */
	ListDigraph::Node addNode(void);

	void setInitial(ListDigraph::Node node);	
private:
	ListDigraph::Node _initial;
	/* Every instruction belongs to a function, functions are identified by
	   their calling address and their name */
	ListDigraph::NodeMap<FunctionCall> _function;
	
};

#endif /* TEST_CFG_H */
