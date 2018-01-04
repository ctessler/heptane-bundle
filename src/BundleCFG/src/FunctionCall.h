#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include "BundleTypes.h"

#include<string>
#include<list>
#include<sstream>
#include<initializer_list>
using namespace std;

/**
 * Represents a call stack for a function
 */
class CallStack : list<uint32_t> {
public:
	/* Has a default copy constructor, may not be exposed */
	CallStack() {
	};
	/**
	 * Creates a call stack with
	 *   Example: CallStack cs({0x4080, 0x4020, 0x4000});
	 *
	 *   Stack Will be T[0x4080, 0x4020, 0x4000] 
	 *   Where 0x4080 is the top of the stack.
	 */
	CallStack(initializer_list<uint32_t> list);
	friend ostream &operator<<(ostream &stream, const CallStack &cs);
	bool operator==(const CallStack &other) const;
	string str() const;

	/**
	 * Peeks the top of the stack
	 */
	uint32_t peek() const;
	/**
	 * Pops the top of the stack
	 */
	uint32_t pop();
	/**
	 * Pushes an element on the top of the stack
	 */
	void push(uint32_t);
	/**
	 * Copies another stack
	 */
	void copy(const CallStack& other);
	/**
	 * Count of the elements on the stack
	 */
	int size() {
		return list::size();
	}
};

/**
 * Represents the function call that is on the stack for an instruction
 */
class FunctionCall {
public:
	FunctionCall();
	FunctionCall(string name, const CallStack &cs);
	/**
	 * Copy constructor
	 */
	FunctionCall(const FunctionCall &other);
	void copy(const FunctionCall &other);
	/**
	 * Push constructor
	 *
	 * @param[in] prev - the current function call
	 * @param[in] name - the name of *this* function call
	 * @param[in] call_site - the address of the instruction
	 *     calling *this* function (ie, top of the stack)
	 */
	FunctionCall(const FunctionCall &prev, string name, uint32_t call_site);
	bool operator==(const FunctionCall &other) const;
	bool operator!=(const FunctionCall &other) const {
		return !(*this == other);
	}
	FunctionCall& operator=(const FunctionCall &other);
	friend std::ostream &operator<< (std::ostream &stream,
					 const FunctionCall& fcall);
	string str() const ;
	string getName() const;
	void setName(string name);

	CallStack& stack() { return _call_stack; };
	bool stacksMatch(const FunctionCall &other) const;
	void fillStack(CallStack &other) const;

private:
	/* Function name */
	string _function_name;
	/* Call stack for *this* function
	 *   If this function was called by an instruction at 0x402,
	 *   the top of the stack will be 0x402 
	 */
	CallStack _call_stack;
};

#endif /* FUNCTION_CALL_H */
