#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "BundleTypes.h"

#include<string>
using namespace std;

/**
 * Represents the function call that is on the stack for an instruction
 */
class FunctionCall {
public:
	FunctionCall(string name="UNASSIGNED", iaddr_t call_site=0);
	FunctionCall(const FunctionCall &other);
	bool operator==(const FunctionCall &other) const;
	bool operator!=(const FunctionCall &other) const { return !(*this == other); }
	FunctionCall& operator=(const FunctionCall &other);
	friend std::ostream &operator<< (std::ostream &stream,
					 const FunctionCall& fcall);

	string getName() const;
	void setName(string name="UNASSIGNED");

	iaddr_t getCallSite() const;
	void setCallSite(iaddr_t call_site=0);
	static bool test();

private:
	/* Function name */
	string _function_name;
	/* Call site address of the function */
	iaddr_t _call_addr;
};

#endif /* FUNCTION_CALL_H */
