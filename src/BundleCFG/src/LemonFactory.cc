#include "LemonFactory.h"

/*
 * This conversion is sloppy, there's no sugar coating it. The Cfg,
 * Program, and Loop types have substantial issues with their
 * use. Converting to a LEMON based graph is tricky and ugly. 
 *
 * Any changes made within this file should be done with extreme care
 * and consideration -- if something looks like it's done confusingly,
 * there's probably a reason. Don't make modifications without
 * thorough testing.
 */

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

/**
 * Gets the address of the first instruction of the Heptane node
 *
 * @param[in] node the heptane node
 *
 * @return the address of the first instruction
 */
static t_address
getFirstAddress(Node *node) {
	Instruction *i = node->GetAsm()[0];

	return getInstructionAddress(i);
}

/**
 * Returns true if this node is the start of a loop
 */
static bool
isLoopStart(Node *node) {
	vector<Loop*> loops = node->GetCfg()->GetAllLoops();

	t_address addr = getFirstAddress(node);
	cout << "isLoopStart 0x" << hex << addr << dec;
	for (unsigned int i = 0; i < loops.size(); i++) {
		Node *head = loops[i]->GetHead();
		if (node == head) {
			cout << " starts a loop" << endl;
			return true;
		}
	}
	cout << " does not start a loop" << endl;
	return false;
}

/**
 * Depth first search for path finding
 */
static bool
heptane_dfs(Node *s, Node *t) {
	stack<Node*> pile;
	map<Node*,bool> visited;

	t_address saddr, taddr;
	saddr = getFirstAddress(s);
	cout << "heptane_dfs 0x " << hex << saddr << dec;
	
	pile.push(s);
	while(!pile.empty()) {
		Node *u = pile.top(); pile.pop();
		if (visited[u] == true) {
			continue;
		}
		visited[u] = true;
		
		if (u == t) {
			/* Found a path */
			taddr = getFirstAddress(s);
			cout << " has exit 0x" << hex << taddr << dec << endl;
			return true;
		}
		vector<Node*> outbound = u->GetCfg()->GetSuccessors(u);
		vector<Node*>::iterator nit;
		for (nit = outbound.begin(); nit != outbound.end(); nit++) {
			pile.push(*nit);
		}
	}
	cout << " has no exit" << endl;
	return false;
}

/**
 * Returns the exit instruction of the loop.
 *
 * @param[in] loop_start the node that starts the loop
 *
 * @return a pointer to the exit instruction if one exists, NULL otherwise.
 */
Node*
exitNode(Node *loop_start) {
	t_address ls_addr = getFirstAddress(loop_start);
	cout << "exitNode Looking for exit to 0x" << hex << ls_addr << dec;
	if (!isLoopStart(loop_start)) {
		cout << " does not start a loop" << endl;
		return NULL;
	}

	/* Exit nodes may not be at the start of the loop */
	stack<Node*> pile;
	map<Node*, bool> visited;

	pile.push(loop_start);
	while (!pile.empty()) {
		Node *cursor = pile.top(); pile.pop();
		/* Process the outgoing edges from this Heptane node */
		vector<Node*> outbound = cursor->GetCfg()->GetSuccessors(cursor);
		vector<Node*>::iterator nit;
		for (nit = outbound.begin(); nit != outbound.end(); nit++) {
			Node *next = (*nit);
			t_address next_addr = getFirstAddress(next);
			cout << "exitNode 0x " << hex << ls_addr << " -> 0x"
			     << next_addr << dec << endl;
			if (heptane_dfs(next, loop_start)) {
				/* In the loop */
				cout << "exitNode 0x " << hex << ls_addr << " -> 0x"
				     << next_addr << dec << " in loop" << endl;
				pile.push(next);
				continue;
			
			}
			/* Exit node */
			cout << "exitNode 0x " << hex << ls_addr << " exits through 0x"
			     << next_addr << dec << endl;
			return next;
		}
	}
	
	cout << "exitNode 0x" << hex << ls_addr << dec << " has no exit node" << endl;
	return NULL;
}

/*
 * Adds the instructions of a basic block to the CFG from the given
 * node.
 *
 * @param[in|out] cfg the control flow graph being added to.
 * @param[in] cursor the node within the cfg corresponding to the node
 *     PRECEDING the ones being added. INVALID if there is no
 *     predecessor.
 * @param[in] node the heptane node containing instructions to be
 * added
 *
 * @return the *last* node added
 */
static ListDigraph::Node
addBBIns(LemonCFG *cfg, ListDigraph::Node cursor, Node *node) {
	ListDigraph::Node last = cursor;
	
	vector<Instruction*> vi = node->GetAsm();
	vector<Instruction*>::iterator it;
	for (it = vi.begin(); it != vi.end(); it++) {
		t_address addr = getInstructionAddress(*it);

		/* Check to see if the node exists in the LemonCFG */
		ListDigraph::Node newNode = cfg->getNode(addr);
		if (newNode == INVALID) {
			newNode = cfg->addNode();
		}
		/* Add instruction attributes here */
		cfg->start(newNode, addr);
		cfg->setFunc(newNode, node->GetCfg()->getStringName());

		if (last != INVALID) {
			ListDigraph::Arc a = findArc(*cfg, last, newNode);
			/* Check to see if the arc was already added */
			if (a == INVALID) {
				cout << "addBBIns Adding a.1: " << cfg->getStartString(last) << " -> "
				     << cfg->getStartString(newNode) << endl;
				cfg->addArc(last, newNode);
			}
		}

		if (last == INVALID) {
			cout << "addBBIns Adding n.1: " << cfg->getStartString(newNode) << " (new)" << endl;
		} else {
			cout << "addBBIns Adding n.2: " << cfg->getStartString(last) << " -> "
			     << cfg->getStartString(newNode) << " (new)" << endl;
		}
		
		/* Don't forget to increment last */
		last = newNode;
	}
	cout << "addBBIns last node: " << cfg->getStartString(last) << endl;

	return last;
}


/**
 * Handles a call
 *
 * @param[in|out] cfg the control flow graph being added to
 * @param[in] a call stack, with the return address as the top element
 * @param[in] node the heptane node of the starting block for the
 * function call
 *
 * @return the last node added
 */
ListDigraph::Node
doCall(LemonCFG *cfg, ListDigraph::Node last, Node *node) {
	stack<unsigned long> pcs;
	stack<ListDigraph::Node> returns;
	stack<Node*> hepStack;
	
	cout << "doCall with predecessor: ";
	if (last != INVALID) {
		cout << cfg->getStartString(last) << endl;
	} else {
		cout << "none" << endl;
	}

	t_address addr = getFirstAddress(node);
	ListDigraph::Node entry = cfg->getNode(addr);
	if (entry != INVALID) {
		cout << "doCall " << cfg->getStartString(entry) << " exists" << endl;
		/* Check for the arc */
		ListDigraph::Arc a = INVALID;
		if (last != INVALID) {
			findArc(*cfg, last, entry);
		}
		if (a == INVALID) {
			cout << "doCall Adding a.1: "
			     << cfg->getStartString(last) << " -> "
			     << cfg->getStartString(entry) << endl;
			cfg->addArc(last, entry);
		}
		return INVALID;
	}

	/* Create the LemonCFG nodes from this Heptane node's instructions */
	ListDigraph::Node final = addBBIns(cfg, last, node);
	
	/* If there's a predecessor, add the arc from it to this LemonCFG start node */
	entry = cfg->getNode(addr);
	if (last != INVALID) {
		ListDigraph::Arc a = findArc(*cfg, last, entry);
		if (a == INVALID) {
			cout << "doCall Adding a.2: "
			     << cfg->getStartString(last) << " -> "
			     << cfg->getStartString(entry) << endl;
			cfg->addArc(last, entry);
		}
	}
	int succ_count = 0;
	last = INVALID;

	/* Process the outgoing edges from this Heptane node */
	vector<Node*> outbound = node->GetCfg()->GetSuccessors(node);
	vector<Node*>::iterator nit;
	cout << "doCall Finding successors of 0x" << hex << getFirstAddress(node) << endl;
	for (nit = outbound.begin(); nit != outbound.end(); nit++) {
		Node *next = (*nit);
		t_address next_node = getFirstAddress(next);
		cout << "  0x" << hex << next_node << endl;

		/* Recursive call */
		ListDigraph::Node cand = doCall(cfg, final, next);
		if (cand != INVALID) {
			/* Only one Hetane node in a function will be terminal */
			last = cand;
		}
		succ_count++;
	}
	if (node->IsCall()) {
		Cfg* callto = node->GetCallee();
		Node *next = callto->GetStartNode();

		/* Recursive call */
		ListDigraph::Node terminal = doCall(cfg, final, next);

		/* The return address is 4 past the final instruction of the call */
		unsigned long new_pc = cfg->getStartLong(final) + 4;
		ListDigraph::Node ret_to = cfg->getNode(new_pc);
		if (ret_to == INVALID) {
			throw runtime_error("Cannot find return node");
		}
		
		cout << "doCall Adding a.3: " << cfg->getStartString(terminal) << " -> "
		     << cfg->getStartString(ret_to) << endl;
		cfg->addArc(terminal, ret_to);

		/* This will seem odd, but Heptane connects the basic blocks of
		   functions even if there is no linkage. Here, those linkages
		   are broken. */
		succ_count++;
	}
	if (succ_count == 0) {
		/* 
		 * This node had no successors, it must be the final node in the
		 * function it's contained in
		 */
		last = final;
	}


	/*
	 * Loops are marked in the Heptane graph, and stored in the CFG multiple
	 * times. The loop bounds are also stored, those we want to save.
	 *
	 * The next task is to extract the starting instructions of the loops
	 * and store them in the LEMON graph
	 *
	 * Yes, this loop will be processed multiple times.
	 */
	vector<Loop*> loops = node->GetCfg()->GetAllLoops();
	for (unsigned int i = 0; i < loops.size(); i++) {
		Node *head = loops[i]->GetHead();

		t_address head_instr = getFirstAddress(head);

		int bound;
		SerialisableIntegerAttribute int_attribute =
			(SerialisableIntegerAttribute &)
			loops[i]->GetAttribute(MaxiterAttributeName);
		bound = int_attribute.GetValue();
		
		ListDigraph::Node start = cfg->getNode(head_instr);
		if (start == INVALID) {
			throw runtime_error("Could not find node to start loop");
		}
		if (cfg->isLoopStart(start)) {
			/* Already processed */
			continue;
		}

		/* Find the exit node */
		Node *exit = exitNode(head);
		t_address exit_instr = getFirstAddress(head);		
		if (exit == NULL) {
			throw runtime_error("No exit node");
		}
		ListDigraph::Node end = cfg->getNode(exit_instr);
		if (end == INVALID) {
			throw runtime_error("Could not find node to end loop");
		}

		cout << "doCall " << cfg->getStartString(start)
		     << " starts a loop up to " << bound << " iterations" << endl;
		cfg->markLoop(start, end, true, bound);
	}
	
	return last;
}


/*
 * Entry point for conversion
 */
LemonCFG*
LemonFactory::convert(Program *prog) {
	LemonCFG *cfg = new LemonCFG();

	Cfg* hep_cfg = prog->GetEntryPoint();
	if (cfg == NULL) {
		throw runtime_error("No CFG for program");
	}
	Node *node = hep_cfg->GetStartNode();
	if (node == NULL) {
		throw runtime_error("No start node for CFG");
	}

	stack<ListDigraph::Node> callstack;
	doCall(cfg, INVALID, node);

	ListDigraph::Node root = cfg->getNode(getFirstAddress(node));
	cfg->setRoot(root);
	
	return cfg;
}
