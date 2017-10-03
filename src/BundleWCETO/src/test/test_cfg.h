#ifndef CFG_H
#define CFG_H

#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <string>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <fstream>
#include "BundleTypes.h"
#include "Cache.h"
#include "FunctionCall.h"
using namespace lemon;
using namespace std;

/**
 * @class CFG
 *
 * A class for representing Control Flow Graphs using the LEMON graph
 * library. Inherits from lemon::ListDigraph.
 *
 * Nodes are individual instructions
 */
class CFG : public ListDigraph {
public:
	/* Default Constructor */
	CFG();
	/* Copy constructor */
	CFG(CFG &other);
	friend std::ostream &operator<< (std::ostream &stream,
					 const CFG& cfg);
	/**
	 * Override to protect against incorrect node addition
	 */
	ListDigraph::Node addNode(void);
private:
};

#endif /* CFG_H */
