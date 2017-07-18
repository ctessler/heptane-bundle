#ifndef LEMONFACTORY_H
#define LEMONFACTORY_H

#include "Program.h"
#include "Node.h"
#include "Cache.h"
#include "Analysis.h"
#include "DotPrint.h"
#include "Generic/CallGraph.h"
#include "LemonCFG.h"
#include <stack>
using namespace std;
using namespace cfglib;


/**
 * @class LemonFactory
 *
 * Converts a Heptane program into a single graph using the LEMON
 * graph library.
 *
 * Example usage:
 *
 * Program* prog; 
 * ...
 *
 * LemonCFG* cfg = LemonFactory::convert(prog);
 */
class LemonFactory {
public:
	/**
	 * Converts a Heptane program object into a new LemonCFG
	 * object
	 *
	 * @param[in] prog a pointer to the Heptane program
	 *
	 * @return a pointer to a new LemonCFG object, caller must
	 * delete this later. If any error occurs NULL is returned.
	 */
	static LemonCFG* convert(Program *prog);
};


#endif /* LEMONFACTORY_H */
