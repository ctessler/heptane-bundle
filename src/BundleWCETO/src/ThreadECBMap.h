#ifndef THREADECBMAP_H
#define THREADECBMAP_H

#include <list>
#include <map>
#include "CFR.h"
using namespace std;


/**
 * Thread -> ECB mapping
 *
 * This table carries a thread to ECB mapping, however the
 * information is stored in a convenient for calculation but
 * surprising manner.
 *
 * Thread # | ECBs
 *        0 | 0
 *        1 | Worst Case ECBs for 1 thread to reach and execute this CFR
 *        2 | Additional ECBs for a 2nd thread to reach and exe. this CFR
 *        .
 *        .
 *
 * In concept, the value for thread n is the delta from thread n-1 to
 * thread n. 
 */
class ThreadECBMap : public std::map<uint32_t, ECBs*> {
public:
	friend std::ostream &operator<<(std::ostream&, const ThreadECBMap&);
	ThreadECBMap() {};
	ThreadECBMap(ThreadECBMap &);
	ThreadECBMap(ThreadECBMap *);
	~ThreadECBMap();
	/**
	 * Finds the cumulative ECBs for a given number of threads
	 *
	 * @note the list must be delete'd by the caller.
	 *
	 * @param[in] thread the number of threads to calculate the
	 *   ECBs for
	 *
	 * @return the ECBs for the given number of threads, an ECB
	 * will be repeated in the list if it is included for multiple
	 * threads 
	 */
	ECBs* ecbs(uint32_t thread);
	bool fill(CFR *cfr, uint32_t threads);
	bool fill(ThreadECBMap &src);
	string str(string pfx="") const;
private:
	void _empty();
};


#endif /* THREADECBMAP_H */
