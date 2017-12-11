#ifndef CFRWCETOMAP_H
#define CFRWCETOMAP_H

#include "ThreadWCETOMap.h"
#include "CFR.h"

/**
 * Maps a CFR pointer to its ThreadWCETOMap 
 *
 * ThreadWCETOMap's may be referred to by their pointer value, however
 * they will disappear when this object is destroyed.
 *
 */
class CFRWCETOMap : public std::map<CFR *, ThreadWCETOMap*> {
public:
	friend std::ostream &operator<<(std::ostream&, const CFRWCETOMap&);
	~CFRWCETOMap();

	/**
	 * Checks if the CFR has a ThreadWCETO map in the container.
	 *
	 * @param[in] cfr the CFR being looked for
	 *
	 * @return a pointer to ThreadWCETOMap if one is present, NULL
	 * otherwise. 
	 */
	ThreadWCETOMap* present(CFR* cfr);
	/**
	 * Gets the ThreadWCETOMap for a CFR, creating it if necessary.
	 *
	 * @param[in] cfr the CFR for which the TWMap is being requested.
	 *
	 * @return a pointer to the TWMap associated with the CFR. 
	 */
	ThreadWCETOMap* request(CFR* cfr);
	string str(string pfx="") const;	
};
#endif /* CFRWCETOMAP_H */
