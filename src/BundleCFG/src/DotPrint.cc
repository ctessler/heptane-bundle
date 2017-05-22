#include "DotPrint.h"

// -----------------------------------------------
// BackEdge: helper function to determine if an
// edge is a back edge
// NB: this function is not efficiently implemented
//     because the library does not allow to know
//     quickly is an edge is a back-edge or not
// -----------------------------------------------
static bool
BackEdge (Cfg * c, Node * s, Node * d)
{

	Edge *e = c->FindEdge (s, d);
	assert (e != NULL);
	vector <Loop*>vl = c->GetAllLoops ();

	for (unsigned int l = 0; l < vl.size (); l++) {
		vector < Edge * >vbe = vl[l]->GetBackedges();
		for (unsigned int be = 0; be < vbe.size (); be++) {
			if (vbe[be] == e) {
				return true;
			}
		}
	}
	return false;
}


/**
 * Gets the starting address of a node from the Control Flow Graph
 *
 * This is a cut and paste from the AnalysisHelper, but the
 * AnalysisHelper cannot be used due to it's dependency upon the
 * global configuration object.
 */
t_address getStartAddress(Node * n) {
	vector < Instruction * >vi = n->GetAsm();
	if (vi.size() <= 0) {
		throw runtime_error("Node with no instructions found");
	}
	if (!vi[0]->HasAttribute(AddressAttributeName)) {
		throw runtime_error("Node with address attribute");
	}
	AddressAttribute attr = (AddressAttribute &)
		vi[0]->GetAttribute(AddressAttributeName);
	return (attr.getCodeAddress());
}


// -------------------------------------------
// Displays the contents of one node and
// prints the name of its successors in the CFG
// -------------------------------------------
static bool
displayNode (Cfg * c, Node * n, ofstream & os)
{
	if (n->HasAttribute (InternalAttributeNameOK)) {
		os << "node" << n << ";" << endl;
		// Node already treated
		return true;
	}
	
	os << "node" << n << " [label = \"";

	// Node address and type
	t_address add = getStartAddress(n);
	if (add != INVALID_ADDRESS) {
		os << "@0x" << hex << add << " ";
	}
	if (n->IsBB ())	{
		os << "(BB)";
	} else {
		os << "(Call ";
		Cfg *callee = n->GetCallee ();
		assert (callee != NULL);
		os << callee->getStringName/*GetName*/ ();
		if (callee->IsExternal ())
			os << ": external)";
		else
			os << ")";
	}
	// Add-on: display CRPD information
	// displayCRPD(c,n,os);
	// os <<"\\n";
	NonSerialisableIntegerAttribute OK (0);
	n->SetAttribute (InternalAttributeNameOK, OK);

	os << "\"";

	Node *entry = c->GetStartNode ();
	if (entry == n)
		os << " ,color=\"blue\"";

	// Display loops heads in green
	vector <Loop *>vl = c->GetAllLoops ();
	for (unsigned int i = 0; i < vl.size (); i++) {
		Node *head = vl[i]->GetHead ();
		if (head == n) {
			os << " ,color=\"green\"";
			break;
		}
	}

	os << "];" << endl;

	return true;
}

static t_address
getInstructionAddress(Instruction* instr) {
	AddressAttribute attr = (AddressAttribute &)
		instr->GetAttribute(AddressAttributeName);
	return (attr.getCodeAddress());
}	

static void
displayNodeAsSubgraph(Node* node, ofstream& os)
{
	vector<Instruction*> vi = node->GetAsm();
	vector<Instruction*>::iterator it;

	/* The first instruction is special. */
	t_address addr = getInstructionAddress(vi[0]);
	os << "node" << node << " [shape=record, "
	   << "label = \"" << "@0x" << hex << addr << "\\n" << endl;
	it = vi.begin();
	for (++it; it != vi.end(); it++) {
		addr = getInstructionAddress(*it);
		os << "@0x" << hex << addr << "\\n" << endl;
	}
	os << "\"];" << endl;
}
		
	

// -------------------------------------------
// Displays the contents of one node and
// prints the name of its successors in the CFG
// -------------------------------------------
static bool
displayNode2(Cfg* c, Node* node, ofstream & os)
{
	if (node->HasAttribute (InternalAttributeNameOK)) {
		/*
		 * This node has already been visited.
		 * However, it may have only been linked to by name
		 * but does not yet exist as a node itself.
		 */
		os << "node" << node << ";" << endl;
		return true;
	}

	if (node->IsBB()) {
		displayNodeAsSubgraph(node, os);
	} else if (node->IsCall()) {
		os << "node" << node << " [label = \"";
		os << "(Call ";
		Cfg *callee = node->GetCallee ();
		assert (callee != NULL);
		os << callee->getStringName/*GetName*/ ();
		if (callee->IsExternal ())
			os << ": external)";
		else
			os << ")";
		os << "\"];";
	}
	NonSerialisableIntegerAttribute OK (0);
	node->SetAttribute (InternalAttributeNameOK, OK);

	return true;
}


static bool
displaySucs (Cfg * c, Node * n, ofstream & os)
{

  // Print its successors in the CFG
  vector < Node * >sucs = c->GetSuccessors (n);
  for (unsigned int i = 0; i < sucs.size (); i++)
    {
      os << "node" << n << " -> " << "node" << sucs[i];
      if (BackEdge (c, n, sucs[i]))
	os << "[color=\"red\"]";
      os << ";" << endl;
    }
  return true;
}

static bool
displayLoop (Cfg * c, Loop * l, ofstream & os, vector < Loop * >&vl)
{

  // Print the loop if there is no loop in the list above the loop in the list
  if (l != NULL)
    {
      for (unsigned int nl = 0; nl < vl.size (); nl++)
	{
	  if (vl[nl] != l)
	    {
	      if (l->IsNestedIn (vl[nl]))
		{
		  return true;
		}
	    }
	}
    }

  // Remove the loop from the list of loops to be printed
  for (std::vector < Loop * >::iterator it = vl.begin (); it != vl.end (); it++)
    {
      if ((*it) == l)
	{
	  vl.erase (it);
	  break;
	}
    }

  // Print the loop nodes
  if (l != NULL)
    {
      vector < Node * >vn = l->GetAllNodes ();
      os << "subgraph " << "cluster_loop" << l << " {" << endl;
      int maxiter;
      SerialisableIntegerAttribute ai = (SerialisableIntegerAttribute &) l->GetAttribute (MaxiterAttributeName);
      maxiter = ai.GetValue ();
      os << "graph [label = \"loop [" << dec << maxiter << "]";
      // displayCRPDLoop(c,l,os);
      os << "\"];" << endl;
      for (unsigned int i = 0; i < vn.size (); i++)
	{
	  displayNode2 (c, vn[i], os);
	}
    }

  // Display subloops of loop l
  vector < Loop * >viter = vl;

  for (unsigned int nl = 0; nl < viter.size (); nl++)
    {
      bool stillthere = false;
      for (unsigned int tmp = 0; tmp < vl.size (); tmp++)
	if (vl[tmp] == viter[nl])
	  stillthere = true;
      if (stillthere && viter[nl]->IsNestedIn (l))
	{
	  displayLoop (c, viter[nl], os, vl);
	}
    }

  if (l != NULL)
    os << "}" << endl;

  return true;
}


static void
displayCfg (Cfg * c, ofstream & os)
{
	os << "subgraph cluster_" << c->getStringName() << " {" << endl;
	os << "graph [label = \"" << c->getStringName() << "\"];" << endl;

	vector < Loop * >vl = c->GetAllLoops ();
	displayLoop (c, NULL, os, vl);
	assert (vl.size () == 0);

	vector < Node * >vn = c->GetAllNodes ();
	for (unsigned int i = 0; i < vn.size (); i++) {
		displayNode2 (c, vn[i], os);
	}
	os << "}" << endl;
}

static void
displayAllSucs (ofstream & os, Program * p)
{
	vector < Cfg * >lc = p->GetAllCfgs ();
	for (unsigned int c = 0; c < lc.size (); c++) {
		vector < Node * >vn = lc[c]->GetAllNodes();
		for (unsigned int i = 0; i < vn.size (); i++) {
			displaySucs(lc[c], vn[i], os);
		}
	}
}


// ----------------------
// DotPrint class
// ----------------------
DotPrint::DotPrint (Program * p, string dir) : Analysis (p)
{
	directory = dir;
};

// ----------------------------------------------
// Performs the analysis
// Returns true if successful, false otherwise
// ----------------------------------------------
bool
DotPrint::PerformAnalysis ()
{
	CallGraph * call_graph = new CallGraph(p);
	Cfg * currentCfg;

	string filename = this->directory + "/" + p->GetName () + ".dot";
	ofstream os (filename.c_str ());
	os << "digraph G {" << endl;
	vector < Cfg * > lc = p->GetAllCfgs ();

	for (unsigned int c = 0; c < lc.size (); c++) {
		currentCfg = lc[c];
		if (! call_graph->isDeadCode(currentCfg))
			displayCfg (currentCfg, os);
	}
	displayAllSucs (os, p);
	os << "}" << endl;
	os.close ();
	// Isabelle: changed format to a pdf output, was not managing colors
	// properly with jpg export on version 2.32 (default color seemed to
	// be white, ...)
	string command = "dot -Tpdf " + filename + " > " + this->directory + "/" + p->GetName () + ".pdf";
	cout << "Running dot command: " << command << endl;
	system (command.c_str ());
	return true;
}

// Remove all private attributes
void
DotPrint::RemovePrivateAttributes ()
{
	vector < Cfg * >lc = p->GetAllCfgs ();
	for (unsigned int c = 0; c < lc.size (); c++) {
		vector < Node * >ln = lc[c]->GetAllNodes();

		for (unsigned int n = 0; n < ln.size (); n++) {
			if (ln[n]->HasAttribute (InternalAttributeNameOK)) {
				ln[n]->RemoveAttribute (InternalAttributeNameOK);
			}
		}
	}
}
