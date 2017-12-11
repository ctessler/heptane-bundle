#ifndef THREADWCETOMAP_H
#define THREADWCETOMAP_H

#include<limits.h>
#include<map>
#include<iostream>

#include "CFR.h"
using namespace std;

/**
 * Thread -> WCETO mapping
 *
 * This table carries a thread to WCETO mapping, however the
 * information is stored in a convenient for calculation but
 * surprising manner.
 *
 * Thread # | WCETO
 *        0 | 0
 *        1 | WCETO for 1 threads - WCETO[0]
 *        2 | WCETO for 2 threads - WCETO[1]
 *        .
 *        .
 *
 * In concept, the value for thread n is the delta from thread n-1 to
 * thread n. 
 */
class ThreadWCETOMap : public std::map<uint32_t, uint32_t> {
public:
	friend std::ostream &operator<<(std::ostream&, const ThreadWCETOMap&);
	ThreadWCETOMap() {};
	ThreadWCETOMap(ThreadWCETOMap &);
	ThreadWCETOMap(ThreadWCETOMap *);	
	/**
	 * Finds the cumulative WCETO for a given number of threads
	 *
	 * @param[in] thread the number of threads to calculate the
	 * WCETO for. 
	 *
	 * @return the WCETO for all thread-s
	 */
	uint32_t wceto(uint32_t thread);
	bool fill(CFR *cfr, uint32_t threads);
	string str(string pfx="") const;
};

#endif /* THREADWCETOMAP_H */
