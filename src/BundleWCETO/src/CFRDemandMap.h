#ifndef CFRDEMANDMAP_H
#define CFRDEMANDMAP_H

#include "ThreadWCETOMap.h"
#include "CFRECBs.h"
#include "CFR.h"

/**
 * Maps a CFR pointer to its different types of demand.
 *
 * Currently, there are three types of demand encapsulated in one class
 * the CFRDemand class, also defined in this file. Those types are
 *   ECB - the list of unique ECBs in a CFR
 *   EXE - the worst case execution time of a single thread (not
 *       considering cache interaction)
 *   WCETO - the mapping of number of threads to a worst case
 *       execution time, think of this as a map from
 *       (cfr, n threads) --> WCETO
 *
 * The interface to the demand map determines whether or not the
 * caller must release the memory allocated, or if they receive a real
 * pointer to the information.
 *
 * Methods that belong with 'copy' must be deleted by the caller.
 * Methods that belong with 'get' are pointers to the canonical maps
 * for the CFR.
 */
class CFRDemand {
public:
	friend std::ostream &operator<<(std::ostream&, const CFRDemand&);

	CFRDemand(CFR& cfr);
	CFRDemand(CFRDemand &src);
	~CFRDemand();
	ThreadWCETOMap& getWCETOMap();
	ThreadWCETOMap* copyWCETOMap();
	uint32_t& getEXE(); /* WCET for 1 Thread */
	uint32_t copyEXE();
	uint32_t& getLoad(); /* Load cost for entire CFR */
	uint32_t copyLoad();
	ECBs& getECBs(); /* ECBs of *this* CFR */
	ECBs* copyECBs();

	string str(string pfx="") const;
private:
	ThreadWCETOMap *_twmap;
	uint32_t _load_cycles;
	uint32_t _exe_cycles;
	ECBs *_ecbs;
	CFR &_cfr;
};


class CFRDemandMap : public std::map<CFR*, CFRDemand*> {
public:
	friend std::ostream &operator<<(std::ostream&, const CFRDemandMap&);
	~CFRDemandMap();

	/**
	 * Checks if the CFR has a DemandMap in the container.
	 *
	 * @param[in] cfr the CFR being looked for
	 *
	 * @return a pointer to  if one is present, NULL
	 * otherwise. 
	 */
	CFRDemand* present(CFR* cfr);
	/**
	 * Gets the DemandMap for a CFR, creating it if necessary.
	 *
	 * @param[in] cfr the CFR for which the Map is being requested.
	 *
	 * @return a pointer to the DemandMap associated with the CFR. 
	 */
	CFRDemand* request(CFR* cfr);
	string str(string pfx="") const;	
};
#endif /* CFRDEMANDMAP_H */
