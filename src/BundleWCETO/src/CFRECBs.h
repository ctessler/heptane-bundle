#ifndef CFRECBS_H
#define CFRECBS_H 

#include<sstream>
#include<iostream>
#include<cstdint>
#include<list>
#include<stdint.h>
using namespace std;

/**
 * ECBs are a (multi) list of cache set indexes
 */
class ECBs : public list<uint32_t> {
public:
	friend std::ostream &operator<<(std::ostream&, const ECBs&);
	ECBs ();
	ECBs (ECBs &src);
	ECBs (ECBs *src);
	void dupesOnly();
	uint32_t dupeCount();

	string str(string prefix="") const;
};

#endif /* CFRECBS_H */
