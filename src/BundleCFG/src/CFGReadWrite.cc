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
	ss << "0x" << hex << setw(6) << setfill('0') << call.getCallSite()
	   << dec << " " << call.getName();

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
	ss << "0x" << setw(6) << setfill('0') << _cfg.getFunction(src).getCallSite();
	ss << "  -->   ";

	ss << "0x" << hex << setw(6) << _cfg.getAddr(tgt) << " ";
	ss << "0x" << setw(6) << setfill('0') << _cfg.getFunction(tgt).getCallSite();

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
			ofile << nodeString(head) << endl;
			visited[head] = true;
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
	string callsite, name;
	ifile >> callsite >> name;
	iaddr_t callsite_addr = stoul(callsite, &pos, 16);
	call.setCallSite(callsite_addr);
	call.setName(name);

	_cfg.setFunction(node, call);
	
	if (closest_head_addr != 0) {
		/* Because the file is ordered correctly, we're
		 * guaranteed the node will exist in the CFG already. 
		 */
		ListDigraph::Node head = _cfg.find(closest_head_addr, call);
		if (head == INVALID) {
			throw runtime_error("Could not find head");
		}
		_cfg.setHead(node, head);
	}
}


bool
CFGReader::addArc(ifstream &ifile) {
	string sNodeAddr, sNodeCallSite, dNodeAddr, dNodeCallSite, arrow;

	if (!(ifile >> sNodeAddr >> sNodeCallSite
	      >> arrow
	      >> dNodeAddr >> dNodeCallSite)) {
		return false;
	}
	size_t pos;
	iaddr_t src_addr = stoul(sNodeAddr, &pos, 16);
	iaddr_t src_calls = stoul(sNodeCallSite, &pos, 16);
	iaddr_t dst_addr = stoul(dNodeAddr, &pos, 16);
	iaddr_t dst_calls = stoul(dNodeCallSite, &pos, 16);

	ListDigraph::Node src = _cfg.find(src_addr, FunctionCall("NA", src_calls));
	ListDigraph::Node dst = _cfg.find(dst_addr, FunctionCall("NA", dst_calls));

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
