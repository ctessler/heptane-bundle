#include "CFGFactory.h"
/**
 * Attributes are not statically defined interfaces.
 *
 * Their use is "private" in a sense, take care when they change.
 */
static t_address
instrAddr(Instruction* instr) {
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
firstAddr(Node *node) {
	Instruction *i = node->GetAsm()[0];

	return instrAddr(i);
}


CFG*
CFGFactory::produce() {
	CFG *cfg = new CFG();

	Cfg* hep_cfg = _prog->GetEntryPoint();
	if (cfg == NULL) {
		throw runtime_error("No Heptane CFG for program");
	}
	Node *node = hep_cfg->GetStartNode();
	if (node == NULL) {
		throw runtime_error("No start node for CFG");
	}
	
	ListDigraph::Node terminal = makeCall(cfg, INVALID, node);
	cout << _debug.str() ;

	cfg->setTerminal(terminal);
	ListDigraph::Node initial = cfg->find(firstAddr(node),
					      FunctionCall("main", 0x0));
	cfg->setInitial(initial);

	identifyLoops(*cfg);

	string indent_save = _indent;
	_debug << "Loop Bounds:" << endl;
	_indent += "| ";
	vector<Cfg*> hep_cfgs = _prog->GetAllCfgs();
	for (unsigned int c = 0; c < hep_cfgs.size(); c++) {
		boundLoops(*cfg, hep_cfgs[c]);
	}
	_indent = indent_save;
	cout << _debug.str();

	return cfg;
}

/**
 * Takes a function call from the Heptane CFG and adds it to the CFG.
 *
 * @param[in|out] cfg the CFG being modified
 * @param[in] call_site the address of the instruction making the call
 * @param[in] node the heptane node that begins the function
 *
 * @return the final node in the function
 */
ListDigraph::Node
CFGFactory::makeCall(CFG *cfg, ListDigraph::Node call_site, Node *node) {
	string indent_save = _indent;
	_debug << _indent << "makeCall(" << *cfg << ", ";
	_indent += "| ";

	string fname = node->GetCfg()->getStringName();
	FunctionCall call(fname);
	if (call_site == INVALID) {
		/* Special case for the first function call */
		call.setCallSite(0x0);
	} else {
		/* Otherwise the call site must exist in the CFG */
		call.setCallSite(cfg->getAddr(call_site));
	}
	_debug << call << ", " << node << ")" << endl;

	ListDigraph::Node first, last;
	last = makeBB(cfg, call, node);

	t_address addr = firstAddr(node);
	first = cfg->find(addr, call);
	if (first == INVALID) {
		throw runtime_error("Unable to find starting node");
	}

	/* Prioritize successors of this Heptane node */
	vector<Node*> outbound = node->GetCfg()->GetSuccessors(node);
	vector<Node*>::iterator nit;

	/* Needed to determine if *this* node is terminal */
	int succ_count = 0;
	ListDigraph::Node final = INVALID;
	for (nit = outbound.begin(); nit != outbound.end(); nit++) {
		Node *next = (*nit);
		addr = firstAddr(next);
		ListDigraph::Node succ_first, succ_last;

		succ_first = cfg->find(addr, call);
		if (succ_first == INVALID) {
			succ_last = makeCall(cfg, call_site, next);
			succ_first = cfg->find(addr, call);
			if (first == INVALID) {
				throw runtime_error("Unable to find starting node");
			}
			if (succ_last != INVALID) {
				/* Only possible for one node to terminate a function */
				final = succ_last; 
			}
		}
		if (!node->IsCall()) {
			cfg->addArc(last, succ_first);
			_debug << _indent << "makeCall added #2 "
			       << cfg->stringNode(last) << " -- > "
			       << cfg->stringNode(succ_first) << endl;
		}
		succ_count++;
	}
	/* Handle calls from this function */
	if (node->IsCall()) {
		_debug << _indent << "makeCall " << cfg->stringNode(last) << " is a call" << endl;
		Cfg* callto = node->GetCallee();
		Node *next = callto->GetStartNode();

		ListDigraph::Node terminal = makeCall(cfg, last, next);
		ListDigraph::Node fstart = cfg->find(firstAddr(next), cfg->getFunction(terminal));
		if (fstart == INVALID) {
		} 
		/* Call edge */
		cfg->addArc(last, fstart);
		_debug << _indent << "makeCall add #3 "
		       << cfg->stringNode(last) << " -- > "
		       << cfg->stringNode(fstart) << endl;

		/* The return address is 4 past the final instruction of the call */
		unsigned long new_pc = cfg->getAddr(last) + 4;
		ListDigraph::Node ret_to = cfg->find(new_pc, call);
		if (ret_to == INVALID) {
			throw runtime_error("Cannot find return node");
		}

		/* Return edge */
		cfg->addArc(terminal, ret_to);
		_debug << _indent << "makeCall add #4 "
		       << cfg->stringNode(terminal) << " -- > "
		       << cfg->stringNode(ret_to) << endl;
		succ_count++;
	}

	if (succ_count == 0) {
		/* This node had no successors, it must be final */
		final = last;
	}
	_debug << _indent << "makeCall final node " << cfg->stringNode(final) << endl;
	_indent = indent_save;
	return final;
}

/**
 * Adds the instructions of the basic block to the CFG
 *
 * @param[in|out] cfg the CFG being added to
 * @param[in] call FunctionCall to use for all instructions of this node.
 * @param[in] node the node containing the instructions
 * @param[out] debug string buffer for debugging.
 */
ListDigraph::Node
CFGFactory::makeBB(CFG *cfg, const FunctionCall &call, Node *node) {
	ListDigraph::Node last = INVALID;
	
	vector<Instruction*> vi = node->GetAsm();
	vector<Instruction*>::iterator it;
	for (it = vi.begin(); it != vi.end(); it++) {
		t_address addr = instrAddr(*it);

		/* Nodes should not exist, if they are visited multiple times
		 * there are multiple call sites which creates distinct nodes.
		 */
		ListDigraph::Node new_node = cfg->find(addr, call);
		if (new_node != INVALID) {
			throw runtime_error("Node exists " + cfg->stringNode(new_node));
		}
		new_node = cfg->addNode();
		
		/* Add instruction attributes here */
		cfg->setAddr(new_node, addr);
		cfg->setFunction(new_node, call);
		_debug << _indent << "makeBB created node " << cfg->stringNode(new_node) << endl;

		/* Add an arc from the previous instruction */
		if (last != INVALID) {
			cfg->addArc(last, new_node);
			_debug << _indent << "makeBB added " << cfg->stringNode(last) << " --> "
			       << cfg->stringNode(new_node) << endl;
		}

		/* Don't forget to increment last */
		last = new_node;
	}

	return last;
}

/**
 * Identifies the loops in the control flow graph
 *
 * @param[in] cfg the LemonCFG
 */
void
CFGFactory::identifyLoops(CFG &cfg) {
	_debug.str("");
	ListDigraph::Node root = cfg.getInitial();

	ListDigraph::NodeMap<int> pathp(cfg);
	ListDigraph::NodeMap<bool> visited(cfg);
	loopDFS(cfg, root, pathp, visited, 1);
	cout << _debug.str();
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
ListDigraph::Node
CFGFactory::loopDFS(CFG &cfg, ListDigraph::Node node,
		    ListDigraph::NodeMap<int> &pathp,
		    ListDigraph::NodeMap<bool> &visited, unsigned int pos) {
	string indent_save = _indent;
	_debug << _indent << "loopDFS " << cfg.stringNode(node);
	_indent += ".";
		
	if (visited[node]) {
		_debug << " already visited" << endl;
		return cfg.getHead(node);
	}
	_debug << " first visit" << endl;
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
			_debug << _indent << "loopDFS " << cfg.stringNode(node)
			       << " assigned head " << cfg.stringNode(tgt) << endl; 
			cfg.setHead(node, tgt);
			cfg.markHead(tgt);
			tagHead(cfg, node, tgt, pathp);
			continue;
		}
		if (cfg.getHead(tgt) == INVALID) {
			/* tgt has no inner loop header, done */
			continue;
		}
		ListDigraph::Node header = cfg.getHead(tgt);
		if (pathp[header] > 0) {
			/* Header of tgt is on the path to node */
			tagHead(cfg, node, header, pathp);
			continue;
		}
		/* Reentry case, find the proper header */
		while (cfg.getHead(header) != INVALID) {
			header = cfg.getHead(header);
			if (pathp[header] > 0) {
				/* Found the header for node */
				tagHead(cfg, node, header, pathp);
				break; /* the while */
			}
		}
	}

	_indent = indent_save;
	pathp[node] = 0;
	return cfg.getHead(node);
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
void
CFGFactory::tagHead(CFG &cfg, ListDigraph::Node node,
		    ListDigraph::Node head, ListDigraph::NodeMap<int> &pathp) {
	string pre = "tagHead ";
	_debug << _indent << pre
	       << cfg.stringNode(node) << " pos:" << pathp[node] << " "
	       << cfg.stringNode(head) << " pos: " << (head != INVALID ? pathp[head] : -1) << endl;
	if (node == head) {
		return;
	}
	if (head == INVALID) {
		_debug << _indent << pre
		       << cfg.stringNode(node) << " no head assigned, aborting "
		       << "existing head " << cfg.stringNode(cfg.getHead(node))
		       << endl;
		return;
	}
	string indent_save = _indent;
	_indent += ".";

	while (cfg.getHead(node) != INVALID) {
		ListDigraph::Node in_head = cfg.getHead(node);
		if (in_head == head) {
			/* 
			 * Stopping condition found *this* head at the
			 * right position
			 */
			_debug << _indent << pre << cfg.stringNode(node)
			       << " already has head " << cfg.stringNode(head) << endl;
			return;
		}
		_debug << _indent << pre
		       << cfg.stringNode(head) << " pos:" << pathp[head] << " "
		       << cfg.stringNode(in_head) << " pos:" << pathp[in_head]
		       << endl;
		if (pathp[in_head] < pathp[head]) {
			/* in_head is earlier in the path than head,
			 * swap positions */
			_debug << _indent << pre
			       << cfg.stringNode(node) << " new head " << cfg.stringNode(head) << endl;
			cfg.setHead(node, head);
			node = head;
			head = in_head;
			continue;
		}
		node = in_head;
	}
	_debug << _indent << pre
	       << cfg.stringNode(node) << " assigned head " << cfg.stringNode(head) << endl;
	cfg.setHead(node, head);
	_indent = indent_save;
}

/**
 *
 * Applies loop bounds from the Heptane graph to the LemonCFG
 *
 * @param[in] cfg the LemonCFG
 * @param[in] hep_cfg the Heptane CFG
 */
void
CFGFactory::boundLoops(CFG &cfg, Cfg* hep_cfg) {
	vector<Loop*> hep_loops = hep_cfg->GetAllLoops();
	for (unsigned int i = 0; i < hep_loops.size(); i++) {
		/* Get the maximum number of iterations */
		SerialisableIntegerAttribute ai = (SerialisableIntegerAttribute &)
			hep_loops[i]->GetAttribute(MaxiterAttributeName);
		int maxiter = ai.GetValue ();

		/* Get the first node */
		vector<Node*> hep_nodes = hep_loops[i]->GetAllNodes();
		Node *first = hep_nodes[0];
		
		t_address addr = firstAddr(first);
		list<ListDigraph::Node> cfg_nodes = cfg.find(addr);
		list<ListDigraph::Node>::iterator lit;
		for (lit = cfg_nodes.begin(); lit != cfg_nodes.end(); ++lit) {
			ListDigraph::Node cfg_node = *lit;
			if (cfg_node == INVALID) {
				throw runtime_error("Unable to find node");
			}
			if (!cfg.isHead(cfg_node)) {
				throw runtime_error("Node not marked as a loop header");
			}
			cfg.setIters(cfg_node, maxiter);
			_debug << _indent << cfg.stringNode(cfg_node) << endl;
		}
	}
}
