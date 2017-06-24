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

static t_address
getInstructionAddress(Instruction* instr) {
	AddressAttribute attr = (AddressAttribute &)
		instr->GetAttribute(AddressAttributeName);
	return (attr.getCodeAddress());
}	

void
DotPrint::displayNodeAsSubgraph(Node* node, ofstream& os)
{
	vector<Instruction*> vi = node->GetAsm();
	vector<Instruction*>::iterator it;

	/* The first instruction is special. */
	t_address addr = getInstructionAddress(vi[0]);

	os << "node" << node << " [shape=plaintext];" << endl;
	os << "node" << node
	   << " [label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">" << endl;

	if (node->IsCall()) {
		Cfg *callTo = node->GetCallee();
		string callToName = callTo->getStringName();
		if (callTo->IsExternal()) {
			callToName = callToName + " external";
		}
		os << "\t<TR><TD colspan=\"3\">Call " << callToName << "</TD></TR>" << endl;
	}
	os << "\t<TR><TD>Address</TD>";

	map<int, Cache*>::iterator cit;
	for (cit = _icache.begin(); cit != _icache.end(); cit++) {
		os << "<TD> L" << cit->first << " Set</TD>";
	}
	os << "</TR>" << endl;
	
	for (it = vi.begin(); it != vi.end(); it++) {
		addr = getInstructionAddress(*it);
		os << "\t<TR><TD>0x" << hex << addr << "</TD>";
		for (cit = _icache.begin(); cit != _icache.end(); cit++) {
			os << "<TD>" << dec << cit->second->setIndex(addr) << "</TD>";
		}
		os << "</TR>" << endl;
	}
	os << "\t</TABLE>> ];" << endl;
}
		
	

// -------------------------------------------
// Displays the contents of one node and
// prints the name of its successors in the CFG
// -------------------------------------------
bool
DotPrint::displayNode(Cfg* c, Node* node, ofstream & os)
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

	displayNodeAsSubgraph(node, os);

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

bool
DotPrint::displayLoop(Cfg * c, Loop * l, ofstream & os, vector < Loop * >&vl)
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
	  displayNode(c, vn[i], os);
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


void
DotPrint::displayCfg (Cfg * c, ofstream & os)
{
	os << "subgraph cluster_" << c->getStringName() << " {" << endl;
	os << "graph [label = \"" << c->getStringName() << "\"];" << endl;

	vector < Loop * >vl = c->GetAllLoops ();
	displayLoop (c, NULL, os, vl);
	assert (vl.size () == 0);

	vector < Node * >vn = c->GetAllNodes ();
	for (unsigned int i = 0; i < vn.size (); i++) {
		displayNode(c, vn[i], os);
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
DotPrint::DotPrint (Program * p, string wdir, string fbase,
		    map<int, Cache*> &iCache, map<int, Cache*> &dCache)
	: Analysis (p), _icache(iCache), _dcache(dCache)
{
	working_directory = wdir;
	filename_base = wdir + "/" + fbase;
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

	string dot_file = filename_base + ".dot";
	ofstream os (dot_file.c_str());

	os << "digraph G {" << endl;
	vector<Cfg*> lc = p->GetAllCfgs();
	for (unsigned int c = 0; c < lc.size (); c++) {
		currentCfg = lc[c];
		if (! call_graph->isDeadCode(currentCfg))
			displayCfg(currentCfg, os);
	}
	displayAllSucs (os, p);
	os << "}" << endl;
	os.close();

	string pdf_file = filename_base + ".pdf";
	string jpg_file = filename_base + ".jpg";
	
	string command = "dot -Tpdf " + dot_file + " > " + pdf_file;
	cout << "Running dot command: " << command << endl;
	system(command.c_str());

	command = "dot -Tjpg " + dot_file + " > " + jpg_file;
	cout << "Running dot command: " << command << endl;
	system(command.c_str());
	
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
