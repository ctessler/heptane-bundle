#include "CFRDemandMap.h"

std::ostream &
operator<<(std::ostream& stream, const CFRDemand& cfrd) {
	stream << cfrd.str();
	return stream;
}

string
CFRDemand::str(string pfx) const {
	stringstream ss;
	pfx = pfx + "dmnd ";
	ss << pfx << _cfr.str() << endl
	   << pfx << "single thread WCET: " << _exe_cycles 
	   << " load cost: "
	   << _ecbs->size() * _cfr.getCache()->memLatency() << endl
	   << pfx << "ecbs: " << _ecbs->str() << endl
	   << _twmap->str(pfx);
	return ss.str();
}

CFRDemand::CFRDemand(CFR& cfr) :
	_cfr(cfr) {
	cfr.calcECBs();
	_exe_cycles = cfr.exeCost();
	_load_cycles = cfr.loadCost();
	_ecbs = cfr.getECBs();
	_twmap = new ThreadWCETOMap();
}
CFRDemand::CFRDemand(CFRDemand &src) :
	_cfr(src._cfr) {
	_twmap = src.copyWCETOMap();
	_exe_cycles = src._exe_cycles;
	_load_cycles = src._load_cycles;
	_ecbs = src.copyECBs();
}
CFRDemand::~CFRDemand() {
	delete _twmap;
	_twmap = NULL;
	delete _ecbs;
	_ecbs = NULL;
}

ThreadWCETOMap&
CFRDemand::getWCETOMap() {
	return *_twmap;
}
ThreadWCETOMap*
CFRDemand::copyWCETOMap() {
	return new ThreadWCETOMap(_twmap);
}

uint32_t
CFRDemand::copyEXE() {
	return _exe_cycles;
}
uint32_t&
CFRDemand::getEXE() {
	return _exe_cycles;
}
uint32_t
CFRDemand::copyLoad() {
	return _load_cycles;
}
uint32_t&
CFRDemand::getLoad() {
	return _load_cycles;
}

ECBs&
CFRDemand::getECBs() {
	return *_ecbs;
}
ECBs*
CFRDemand::copyECBs() {
	return new ECBs(_ecbs);
}

CFRDemand*
CFRDemandMap::present(CFR *cfr) {
	CFRDemandMap::iterator it = find(cfr);
	if (it != end()) {
		return it->second;
	}
	return NULL;
}

#define PARANOIA
#undef PARANOIA
CFRDemand*
CFRDemandMap::request(CFR *cfr) {
	CFRDemand *cfrd = present(cfr);
	if (cfrd) {
		return cfrd;
	}
	cfrd = new CFRDemand(*cfr);
	insert(pair<CFR*, CFRDemand*>(cfr, cfrd));

#ifdef PARANOIA
	CFRDemandMap::iterator it = find(cfr);
	if (it->second != cfrd) {
		throw runtime_error("CFRDemandMap::request mismatch in rval");
	}
	cout << "CFRDemandMap::request inserted a new DemandMap ("
	     << cfr << ", " << cfrd << ")" << endl;
	cout << "CFRDemandMap::request " << *cfr << endl;
	cout << cfrd->str("CFRDemandMap::request ") << endl;
#endif
	return cfrd;

}

CFRDemandMap::~CFRDemandMap() {
	for (CFRDemandMap::iterator it = begin(); it != end(); ++it) {
		delete it->second;
	}
}
