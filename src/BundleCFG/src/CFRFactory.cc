#include "CFRFactory.h"

SerialisableIntegerAttribute CFRVisited = 1;

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
	t_address first = getInstructionAddress(node->GetAsm()[0]);
	map<t_address, Node*> terminals;
	terminals.insert(pair<t_address, Node*>(first, node));

	Program *p = new Program();
	cfg = p->CreateNewCfg(cfg->GetName());
	do {
		map<t_address, Node*>::iterator start = terminals.begin();
		node = start->second;
		terminals.erase(start);

		map<t_address, Node*> conflicts = extractNode(cfg, NULL, node, cache);
		terminals.insert(conflicts.begin(), conflicts.end());
	} while (terminals.size() > 0);
	return p;
}

map<t_address, Node*>
CFRFactory::extractNode(Cfg *cfg, Node* parent, Node* node, Cache *cache) {
	map<t_address, Node*> rval;
	/*
	 *  Nodes cannot be cloned or copied, otherwise they will
	 *  belong to the previous CFG
	 */
	Node *newNode = NULL;
	node_type ntype = node->GetType();

	vector<Instruction*> vi = node->GetAsm();
	vector<Instruction*>::iterator it;
	for (it = vi.begin(); it != vi.end(); it++) {
		t_address addr = getInstructionAddress(*it);
		CacheSet *cache_set = cache->setOf(addr);
		CacheLine cline(cache->getLineSize());
		cline.store(addr);
		uint32_t offset = cline.offset(addr);

		cout << "Addr: 0x" << hex << addr << dec 
		     << "\tC.Set: " << cache->setIndex(addr) 
		     << "\tC.Offset: " << offset
		     << "\tPres: " << cache_set->present(addr)
		     << "\tXFlict: ";

		if (cache_set->present(addr)) {
			/* This value is cached */
			if (cache_set->visited(addr)) {
				/* Already executed, loop! */
				cout << "Loop!" << endl;
				Node *target = cfg->findNode(addr);
				if (!target) {
					throw runtime_error("Loop found without an existing node");
				}
				
				
				return rval;
			}
		} else {
			if (cache_set->evicts(addr)) {
				/* Certainly would cause an eviction */
				cout << "Yes(w)" << endl;
				if (newNode != NULL) {
					cfg->SetEndNode(newNode);
				}
				return rval;
			}
			if (!cache_set->empty()) {
				/* Could cause an eviction */
				cout << "Yes(c)" << endl;
				if (newNode != NULL) {
					cfg->SetEndNode(newNode);
				}
				return rval;
			}
		}

		cout << "No" << endl;
		/* Record the access to the instruction */
		cache_set->insert(addr);

		/*
		 * This is where it gets ugly
		 *
		 * The node *cannot* be allocated without at least one
		 * instruction to go within it. That is why newNode is
		 * set to NULL earlier and allocated when the first
		 * instruction is about to be added.
		 */
		if (newNode == NULL) {
			newNode = cfg->CreateNewNode(ntype);
			if (parent) {
				cout << "Adding edge: " << parent << " -> " << node << endl;
				cfg->CreateNewEdge(parent, newNode);
			}
		}
		if (ntype == Call) {
			newNode->SetCall(node->GetCalleeName(), false);
			cout << "Node is a call " << newNode->GetCalleeName() << endl;
		}
		/* Add the instruction to the node in the new CFG */
		Instruction *i = (*it)->Clone();
		newNode->addInstruction(i);
	}

	/*
	 * DO NOT assume a node can only branch or call.
	 */
	if (node->IsCall()) {
		Cfg* callto = node->GetCallee();
		Node *next = callto->GetStartNode();
		/* Recurse */
		extractNode(cfg, NULL, next, cache);
	}

	vector<Edge*> outbound = node->GetCfg()->GetOutgoingEdges(node);
	vector<Edge*>::iterator eit;
	for (eit = outbound.begin(); eit != outbound.end(); eit++) {
		Node *next = (*eit)->GetTarget();
		/* Recurse */
		extractNode(cfg, newNode, next, cache);
	}
	
	return rval;
}
