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

void
CallStack::copy(const CallStack &copy) {
	CallStack::const_iterator it;
	clear();
	for (it = copy.begin(); it != copy.end(); ++it) {
		push_back(*it);
	}
}


FunctionCall::FunctionCall() {
	_function_name = "-NA-";
}

FunctionCall::FunctionCall(const FunctionCall &other) :
    _function_name(other._function_name), _call_stack(other._call_stack) {
}

void
FunctionCall::copy(const FunctionCall &other) {
	_function_name = other._function_name;
	_call_stack.copy(other._call_stack);
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

void
FunctionCall::fillStack(CallStack &other) const {
	other.copy(_call_stack);
}

bool
FunctionCall::stacksMatch(FunctionCall const &other) const {
	return _call_stack == other._call_stack;
}

FunctionCall&
FunctionCall::operator=(const FunctionCall &other) {
	_function_name = other._function_name;
	_call_stack.copy(other._call_stack);
	return *this;
}

bool
FunctionCall::operator==(const FunctionCall &other) const {

	if (_call_stack == other._call_stack) {
		/* Function names don't matter in comparisons just the address */
		if (0 != _function_name.compare(other._function_name)) {
			throw runtime_error("Unexpected difference in function names");
		}
	}
	return (_call_stack == other._call_stack);
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

