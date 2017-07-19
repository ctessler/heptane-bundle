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
