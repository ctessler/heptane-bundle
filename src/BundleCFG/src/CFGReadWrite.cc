#include "CFGReadWrite.h"

string
CFGWriter::nodeHeader() {
	stringstream ss;
	ss << "NODES (order is important!)" << endl;
	ss << setw(9) << left  << "Address"
	   << setw(11) << left << "Loop-Head?"
	   << setw(11) << left << "Iterations"
	   << setw(13) << left << "Closest-Head"
	   << setw(9) << left  << "Frame-1"
	   << endl;
	ss << setw(52) << setfill('-') << '-';
	
	return ss.str();
}

string
CFGWriter::nodeString(ListDigraph::Node node) {
	stringstream ss;
	ss << "0x" << hex << setw(6) << _cfg.getAddr(node) << dec << " ";
	ss << setw(11) << left << (_cfg.isHead(node) ? "YES" : "NO");
	ss << setw(11) << left << _cfg.getIters(node);

	/* Closest Head */
	ss << "0x" << hex << setw(6) << left;
	ListDigraph::Node head = _cfg.getHead(node);
	if (head != INVALID) {
		ss << _cfg.getAddr(head);
	} else {
		ss << setfill('0') << 0;
	}
	ss << dec;
	ss << "     ";

	/* Frame -1 */
	FunctionCall call = _cfg.getFunction(node);
	ss << call;

	return ss.str();
}

string
CFGWriter::arcHeader() {
	stringstream ss;
	ss << "ARCS" << endl;
	ss << setw(9) << left << "Address"
	   << setw(8) << left << "Frame-1"
	   << "  -->    "
	   << setw(9) << left << "Address"
	   << setw(9) << left << "Frame-1" << endl;
	ss << setw(42) << setfill('-') << '-';

	return ss.str();
}

string
CFGWriter::arcString(ListDigraph::Arc arc) {
	stringstream ss;
	ListDigraph::Node src = _cfg.source(arc);
	ListDigraph::Node tgt = _cfg.target(arc);

	ss << "0x" << hex << setw(6) << _cfg.getAddr(src) << " ";
	ss << _cfg.getFunction(src);
	ss << " → ";

	ss << "0x" << hex << setw(6) << _cfg.getAddr(tgt) << " ";
	ss << _cfg.getFunction(tgt);

	return ss.str();
}


void
CFGWriter::write(string path) {
	ofstream ofile(path.c_str());

	ofile << nodeHeader() << endl;

	ListDigraph::NodeMap<bool> visited(_cfg);
	/* Loop Heads first, necessary */
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		if (!_cfg.isHead(nit)) {
			continue;
		}
		if (visited[nit]) {
			continue;
		}
		ListDigraph::Node head = _cfg.getHead(nit);
		while (head != INVALID) {
			if (!visited[head]) {
				ofile << nodeString(head) << endl;
				visited[head] = true;
			}
			head = _cfg.getHead(head);
		}
		ofile << nodeString(nit) << endl;
		visited[nit] = true;
	}
	
	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		if (_cfg.isHead(nit)) {
			continue;
		}
		ofile << nodeString(nit) << endl;
	}

	ofile << endl;
	ofile << arcHeader() << endl;
	for (ListDigraph::ArcIt ait(_cfg); ait != INVALID; ++ait) {
		ofile << arcString(ait) << endl;
	}

	ofile.flush();
	ofile.close();
}

static void
fillFunction(FunctionCall &function, ifstream &ifile) {
	string starter;
	ifile >> starter;
	size_t idx = starter.find(":T[");
	if (idx == string::npos) {
		throw runtime_error("No function call");
	}
	string fname; fname.assign(starter, 0, idx);
	function.setName(fname);

	starter.erase(0, idx+3);
	string rest = starter;
	idx = rest.find("]");
	bool done = false;
	if (idx != string::npos) {
		rest.erase(idx, idx);
		done = true;
	}
	size_t pos;
	uint32_t addr = stoul(rest, &pos, 16);
	list<uint32_t> L;
	L.push_front(addr);
	
	while (!done && ifile >> rest) {
		idx = rest.find("]");
		if (idx != string::npos) {
			rest.erase(idx, idx);
			done = true;
		}
		addr = stoul(rest, &pos, 16);
		L.push_front(addr);
		if (done) {
			break;
		}
	}
	for (list<uint32_t>::iterator it = L.begin(); it != L.end(); ++it) {
		function.stack().push(*it);
	}
	
}

void
CFGReader::addNode(string addr, ifstream &ifile) {
	ListDigraph::Node node = _cfg.addNode();

	size_t pos;
	iaddr_t instr_addr = stoul(addr, &pos, 16);
	_cfg.setAddr(node, instr_addr);

	string isHead;
	ifile >> isHead;
	if (isHead.compare("YES") == 0) {
		_cfg.markHead(node, true);
	} else {
		_cfg.markHead(node, false);
	}

	unsigned int iters;
	ifile >> iters;
	_cfg.setIters(node, iters);

	string closestHead;
	ifile >> closestHead;
	iaddr_t closest_head_addr = stoul(closestHead, &pos, 16);

	FunctionCall call;
	fillFunction(call, ifile);

	_cfg.setFunction(node, call);
	
	if (closest_head_addr != 0) {
		/* Because the file is ordered correctly, we're
		 * guaranteed the node will exist in the CFG already. 
		 */
		ListDigraph::Node head = _cfg.find(closest_head_addr, call);
		FunctionCall copy(call);
		while (head == INVALID) {
			copy.stack().pop();
			if (copy.stack().size() == 0) {
				stringstream ss;
				ss << "Adding instruction 0x" << hex
				   << instr_addr;
				ss << " Could not find head: 0x" << hex
				   << closest_head_addr << " " << copy
				   << dec;
				throw runtime_error(ss.str());
			}
			head = _cfg.findIgnoreName(closest_head_addr, copy);
		}
		_cfg.setHead(node, head);
	}
}


bool
CFGReader::addArc(ifstream &ifile) {
	string sNodeAddr, dNodeAddr, arrow;

	if (!(ifile >> sNodeAddr)) {
		return false;
	}
	FunctionCall srcFn;
	fillFunction(srcFn, ifile);
	if (!(ifile >> arrow >> dNodeAddr)) {
		return false;
	}
	FunctionCall dstFn;
	fillFunction(dstFn, ifile);

	size_t pos;
	iaddr_t src_addr = stoul(sNodeAddr, &pos, 16);
	iaddr_t dst_addr = stoul(dNodeAddr, &pos, 16);

	ListDigraph::Node src = _cfg.find(src_addr, srcFn);
	ListDigraph::Node dst = _cfg.find(dst_addr, dstFn);

	if (src == INVALID || dst == INVALID) {
		throw runtime_error("Could not find nodes to make an arc");
	}
	_cfg.addArc(src, dst);				  
	return true;
}

void
CFGReader::read(string path) {
	ifstream ifile(path.c_str());

	string line;
	for (int i=0; i < 4; i++) {
		ifile >> line; // NODES
	}
	for (int i=0; i < 5; i++) {
		ifile >> line; // Node Header
	}
	ifile >> line; // Divider

	string addr;
	while (true) {
		ifile >> addr;
		if (addr.compare("ARCS") == 0) {
			break;
		}
		addNode(addr, ifile);
	}

	for (int i=0; i < 5; i++) {
		ifile >> line; // Arc Header
	}
	ifile >> line; //Divider

	while (addArc(ifile)) {
		// Let 'er work
	}
	
	ifile.close();

	for (ListDigraph::NodeIt nit(_cfg); nit != INVALID; ++nit) {
		if (countInArcs(_cfg, nit) == 0) {
			_cfg.setInitial(nit);
		}
		if (countOutArcs(_cfg, nit) == 0) {
			_cfg.setTerminal(nit);
		}
	}
}
