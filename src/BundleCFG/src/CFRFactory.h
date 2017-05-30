#ifndef CFR_FACTORY
#define CFR_FACTORY

#include "Program.h"
#include "Cache.h"
using namespace cfglib;

class CFRFactory {
public:
	static int makeCFRG(Program* prog, string dir,
	    map<int, Cache*> iCache, map<int, Cache*> dCache);
};

#endif /* CFR_FACTORY */
