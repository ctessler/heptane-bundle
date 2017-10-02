#ifndef TEST_CFG_H
#define TEST_CFG_H


typedef unsigned long iaddr_t;

#include <lemon/core.h>
#include <lemon/list_graph.h>

#include <string>
using namespace lemon;
using namespace std;

/**
 * FunctionCall -- Represents the callsite for each instruction
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

class CFG : public ListDigraph {
public:
	CFG();
private:
	/* Every instruction belongs to a function, functions are identified by
	   their calling address and their name */
	ListDigraph::NodeMap<FunctionCall> _function;
	
};

#endif
