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
#ifdef DBG_LEMONFACTORY
				cout << "addBBIns Adding a.1: " << cfg->getStartString(last) << " -> "
				     << cfg->getStartString(newNode) << endl;
#endif /* DBG_LEMONFACTORY */
				cfg->addArc(last, newNode);
			}
		}
#ifdef DBG_LEMONFACTORY
		if (last == INVALID) {
			cout << "addBBIns Adding n.1: " << cfg->getStartString(newNode) << " (new)" << endl;
		} else {
			cout << "addBBIns Adding n.2: " << cfg->getStartString(last) << " -> "
			     << cfg->getStartString(newNode) << " (new)" << endl;
		}
#endif /* DBG_LEMONFACTORY */
		/* Don't forget to increment last */
		last = newNode;
	}
#ifdef DBG_LEMONFACTORY
	cout << "addBBIns last node: " << cfg->getStartString(last) << endl;
#endif /* DBG_LEMONFACTORY */

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

#ifdef DBG_DOCALL
	cout << "doCall with predecessor: ";
	if (last != INVALID) {
		cout << cfg->getStartString(last) << endl;
	} else {
		cout << "none" << endl;
	}
#endif /* DBG_DOCALL */
	t_address addr = getFirstAddress(node);
	ListDigraph::Node entry = cfg->getNode(addr);
	if (entry != INVALID) {
#ifdef DBG_DOCALL
		cout << "doCall " << cfg->getStartString(entry) << " exists" << endl;
#endif /* DBG_DOCALL */
		/* Check for the arc */
		ListDigraph::Arc a = INVALID;
		if (last != INVALID) {
			findArc(*cfg, last, entry);
		}
		if (a == INVALID) {
#ifdef DBG_DOCALL
			cout << "doCall Adding a.1: "
			     << cfg->getStartString(last) << " -> "
			     << cfg->getStartString(entry) << endl;
#endif /* DBG_DOCALL */
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
#ifdef DBG_DOCALL
			cout << "doCall Adding a.2: "
			     << cfg->getStartString(last) << " -> "
			     << cfg->getStartString(entry) << endl;
#endif /* DBG_DOCALL */
			cfg->addArc(last, entry);
		}
	}
	int succ_count = 0;
	last = INVALID;

	/* Process the outgoing edges from this Heptane node */
	vector<Node*> outbound = node->GetCfg()->GetSuccessors(node);
	vector<Node*>::iterator nit;
#ifdef DBG_DOCALL
	cout << "doCall Finding successors of 0x" << hex << getFirstAddress(node) << endl;
#endif /* DBG_DOCALL */
	for (nit = outbound.begin(); nit != outbound.end(); nit++) {
		Node *next = (*nit);
#ifdef DBG_DOCALL
		t_address next_node = getFirstAddress(next);
		cout << "  0x" << hex << next_node << endl;
#endif /* DBG_DOCALL */

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
		
#ifdef DBG_DOCALL
		cout << "doCall Adding a.3: " << cfg->getStartString(terminal) << " -> "
		     << cfg->getStartString(ret_to) << endl;
#endif /* DBG_DOCALL */
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

/**
 * Adds head to the "list" of loop heads of node
 *
 * There is no list, but the loop head being added to this node may
 * also have a loop head. It may be the case that node already has
 * been assigned a loop head. This creates a chain of loop heads,
 * which should be ordered by their place in the DFS path to node.
 *
 * @param[in] cfg LemonCFG of the node and head
 * @param[in] node the node being given a new head
 * @param[in] head the head being added to the list
 * @param[in] pathp the path position
 *
 */
static void
tagHead(LemonCFG &cfg, ListDigraph::Node node, ListDigraph::Node head,
	ListDigraph::NodeMap<int> &pathp) {
	if (node == head) {
		return;
	}
	if (head == INVALID) {
		return;
	}

	while (cfg.getLoopHead(node) != INVALID) {
		ListDigraph::Node in_head = cfg.getLoopHead(node);
		if (in_head == head) {
			/* 
			 * Stopping condition found *this* head at the
			 * right position
			 */
			return;
		}
		if (pathp[in_head] < pathp[head]) {
			/* in_head is earlier in the path than head,
			 * swap positions */
			cfg.setLoopHead(node, head);
			node = head;
			head = in_head;
			continue;
		}
		node = in_head;
	}
	cfg.setLoopHead(node, head);
}

/**
 * Depth First Search and ordering of nodes to identify loops
 * 
 * @param[in] node the current node being added
 * @param[in] pathp a mapping from nodes to their position in the path
 * @param[in] visited a mapping from nodes to boolean indicating if
 *     they've been visited 
 * @param[in] pos position in the depth first search
 *
 * @return the (current) innermost loop header of node
 */
static ListDigraph::Node
loopDFS(LemonCFG &cfg, ListDigraph::Node node, ListDigraph::NodeMap<int> &pathp,
	ListDigraph::NodeMap<bool> &visited, unsigned int pos) {

	if (visited[node]) {
		return cfg.getLoopHead(node);
	}
	visited[node] = true;
	pathp[node] = pos;
	/*
	 * Examine all successors
	 */
	for (ListDigraph::OutArcIt a(cfg, node); a != INVALID; ++a) {
		ListDigraph::Node tgt = cfg.runningNode(a);
		if (!visited[tgt]) {
			/* Recursive call */
			ListDigraph::Node header =
				loopDFS(cfg, tgt, pathp, visited, pos + 1);
			tagHead(cfg, node, header, pathp);
			continue;
		}
		if (pathp[tgt] > 0) {
			/*
			 * tgt is on the DFSP to node, node -> tgt
			 * exists, therefor tgt is a loop header of
			 * node
			 */
			cfg.setLoopHead(node, tgt);
			cfg.markLoopHead(tgt);
			tagHead(cfg, node, tgt, pathp);
			continue;
		}
		if (cfg.getLoopHead(tgt) == INVALID) {
			/* tgt has no inner loop header, done */
			continue;
		}
		ListDigraph::Node header = cfg.getLoopHead(tgt);
		if (pathp[header] > 0) {
			/* Header of tgt is on the path to node */
			tagHead(cfg, node, header, pathp);
			continue;
		}
		/* Reentry case, find the proper header */
		while (cfg.getLoopHead(header) != INVALID) {
			header = cfg.getLoopHead(header);
			if (pathp[header] > 0) {
				/* Found the header for node */
				tagHead(cfg, node, header, pathp);
				break; /* the while */
			}
		}
	}
	pathp[node] = 0;
	return cfg.getLoopHead(node);
}

/**
 * Identifies the loops in the control flow graph
 *
 * @param[in] cfg the LemonCFG
 */
static void
identifyLoops(LemonCFG &cfg) {
	ListDigraph::Node root = cfg.getRoot();

	ListDigraph::NodeMap<int> pathp(cfg);
	ListDigraph::NodeMap<bool> visited(cfg);
	loopDFS(cfg, root, pathp, visited, 1);
}


/**
 *
 * Applies loop bounds from the Heptane graph to the LemonCFG
 *
 * @param[in] cfg the LemonCFG
 * @param[in] hep_cfg the Heptane CFG
 */
static void
boundLoops(LemonCFG &cfg, Cfg* hep_cfg) {
	vector<Loop*> hep_loops = hep_cfg->GetAllLoops();
#ifdef DBG_BOUNDLOOPS
	cout << "boundLoops found " << hep_loops.size() << " loops in this CFG" << endl;
#endif /* DBG_BOUNDLOOPS */
	for (unsigned int i = 0; i < hep_loops.size(); i++) {
		/* Get the maximum number of iterations */
		SerialisableIntegerAttribute ai = (SerialisableIntegerAttribute &)
			hep_loops[i]->GetAttribute(MaxiterAttributeName);
		int maxiter = ai.GetValue ();

		/* Get the first node */
		vector<Node*> hep_nodes = hep_loops[i]->GetAllNodes();
		Node *first = hep_nodes[0];
		
		t_address addr = getFirstAddress(first);
		ListDigraph::Node lemon_node = cfg.getNode(addr);
		if (lemon_node == INVALID) {
			throw runtime_error("Unable to find node");
		}
#ifdef DBG_BOUNDLOOPS
		cout << "boundLoops " << cfg.getStartString(lemon_node) << " has "
		     << dec << maxiter << " iterations" << endl;
#endif /* DBG_BOUNDLOOPS */
		if (!cfg.isLoopHead(lemon_node)) {
			throw runtime_error("Node not marked as a loop header");
		}
		cfg.setLoopBound(lemon_node, maxiter);
	}
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

	identifyLoops(*cfg);

	vector<Cfg*> hep_cfgs = prog->GetAllCfgs();
	for (unsigned int c = 0; c < hep_cfgs.size(); c++) {
		boundLoops(*cfg, hep_cfgs[c]);
	}
	
	return cfg;
}
