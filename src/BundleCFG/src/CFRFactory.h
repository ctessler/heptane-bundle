#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "Program.h"
#include "Cache.h"
#include "Analysis.h"
#include "DotPrint.h"
#include "Generic/CallGraph.h"
using namespace cfglib;

class CFRFactory {
public:
	static int makeCFRG(Program* prog, string dir,
	    map<int, Cache*> &iCache, map<int, Cache*> &dCache);
private:
	static Program* BundleExtraction(Program* prog, Cache* cache);
	static bool extractNode(Node* node, Cache *cache);
	
};

#endif /* CFR_FACTORY */
