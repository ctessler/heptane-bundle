#include "CFRFactory.h"

/**
 * Attributes are not statically defined interfaces.
 *
 * Their use is "private" in a sense, take care when they change.
 */
static t_address
getInstructionAddress(Instruction* instr) {
	AddressAttribute attr = (AddressAttribute &)
		instr->GetAttribute(AddressAttributeName);
	return (attr.getCodeAddress());
}	


int CFRFactory::makeCFRG(Program* prog, string dir,
    map<int, Cache*> &iCache, map<int, Cache*> &dCache) {

	Program *cfrg;
	map<int, Cache*>::iterator it;
	for (it = iCache.begin(); it != iCache.end(); it++) {
		int level = it->first;
		Cache *cache = it->second;

		cfrg = BundleExtraction(prog, cache);

		stringstream sstm;
		sstm << prog->GetName() << "-lvl-" << level;
		string base = sstm.str();
		DotPrint dotprint(cfrg, dir, base, iCache, dCache);
		dotprint.PerformAnalysis();
	}
	
	
	return 0;
}

Program*
CFRFactory::BundleExtraction(Program* prog, Cache* cache) {
	Cfg* cfg = prog->GetEntryPoint();
	if (cfg == NULL) {
		throw runtime_error("No CFG for program");
	}

	Node *node = cfg->GetStartNode();
	if (node == NULL) {
		throw runtime_error("No start node for CFG");
	}

	extractNode(node, cache);
	
	return prog;
}

bool
CFRFactory::extractNode(Node* node, Cache *cache) {
	vector<Instruction*> vi = node->GetAsm();
	vector<Instruction*>::iterator it;

	for (it = vi.begin(); it != vi.end(); it++) {
		t_address addr = getInstructionAddress(*it);
		CacheSet *cache_set = cache->setOf(addr);
		CacheLine cline(cache->getLineSize());
		cline.store(addr);
		uint32_t offset = cline.offset(addr);

		cout << "Inspecting address: 0x" << hex << addr << dec 
		     << "\tCache Set: " << cache->setIndex(addr) 
		     << "\tCache Line Offset: " << offset
		     << "\tXFlict: ";

		if (cache_set->present(addr)) {
			cout << "Hit (Loop!)" << endl;
			return false;
		}
		if (cache_set->evicts(addr)) {
			/* Would be a conflict */
			cout << "Yes" << endl;
			return false;
		} else {
			/* No conflict, insert the address */
			cache_set->insert(addr);
			cout << "No" << endl;
		}
	}

	/*
	 * DO NOT assume a node can only branch or call.
	 */
	bool go_on = true;
	
	if (node->IsCall()) {
		Cfg* callto = node->GetCallee();
		Node *next = callto->GetStartNode();
		/* Recurse */
		extractNode(next, cache);
	}

	vector<Edge*> outbound = node->GetCfg()->GetOutgoingEdges(node);
	vector<Edge*>::iterator eit;
	for (eit = outbound.begin(); eit != outbound.end(); eit++) {
		Node *next = (*eit)->GetTarget();
		/* Recurse */
		extractNode(next, cache);
	}
	
	return true;
}
