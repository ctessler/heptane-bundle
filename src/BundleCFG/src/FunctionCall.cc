#include "FunctionCall.h"

CallStack::CallStack(initializer_list<uint32_t> list) {
	for (auto elem : list) {
		push_back(elem);
	}
}

ostream&
operator<<(ostream &stream, const CallStack &cs) {
	stream << cs.str();
	return stream;
}

string
CallStack::str() const {
	stringstream ss;
	CallStack::const_iterator it;
	ss << "T[" << hex;
	for (it=begin(); it != end(); ++it) {
		if (it != begin()) {
			ss << ", ";
		}
		ss << "0x" << *it;
	}
	ss << "]" << dec;
	return ss.str();
}

bool
CallStack::operator==(const CallStack &other) const {
	CallStack::const_iterator this_it;
	CallStack::const_iterator other_it;
	for (this_it = begin(), other_it = other.begin();
	     this_it != end() && other_it != other.end();
	     ++this_it, ++other_it) {
		if (*this_it != *other_it) {
			return false;
		}
	}
	return true;
}

void
CallStack::push(uint32_t addr) {
	push_front(addr);
}

uint32_t
CallStack::peek() const {
	return front();
}

uint32_t
CallStack::pop() {
	uint32_t rval = peek();
	pop_front();
	return rval;
}


FunctionCall::FunctionCall(string name, iaddr_t call_site) :
    _function_name(name), _call_addr(call_site) {
}

FunctionCall::FunctionCall(const FunctionCall &other) :
    _function_name(other._function_name), _call_stack(other._call_stack) {
	_call_addr = other._call_addr;
}

FunctionCall::FunctionCall(string name, const CallStack &cs) :
    _function_name(name), _call_stack(cs) {
}

FunctionCall::FunctionCall(const FunctionCall &prev, string name, uint32_t site) :
    _function_name(name), _call_stack(prev._call_stack) {
	_call_stack.push(site);	
}

string
FunctionCall::getName() const {
	return _function_name;
}

void
FunctionCall::setName(string name) {
	_function_name = name;
}

iaddr_t
FunctionCall::getCallSite() const {
	return _call_addr;
}
void
FunctionCall::setCallSite(iaddr_t call_site) {
	_call_addr = call_site;
}

FunctionCall&
FunctionCall::operator=(const FunctionCall &other) {
	_function_name = other._function_name;
	_call_addr = other._call_addr;

	return *this;
}

bool
FunctionCall::operator==(const FunctionCall &other) const {
	/* Function names don't matter in comparisons just the address */
	#if 0
	if (0 != _function_name.compare(other._function_name)) {
		return false;
	}
	#endif
	return (_call_addr == other._call_addr);
}

string
FunctionCall::str() const {
	stringstream ss;
	ss << getName() << ":" << _call_stack;
	return ss.str();
}

std::ostream&
operator<< (std::ostream &stream, const FunctionCall& fcall) {
	stream << fcall.str();
	return stream;
}

