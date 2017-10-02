#include "test_cfg.h"

FunctionCall::FunctionCall(string name, iaddr_t call_site) :
	_function_name(name), _call_addr(call_site) {

}

FunctionCall::FunctionCall(const FunctionCall &other) {
	*this = other;
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

std::ostream&
operator<< (std::ostream &stream, const FunctionCall& fcall) {
	stream << fcall.getName() << ":"
	       << "0x" << hex << fcall.getCallSite() << dec;
	return stream;
}

/**
 * CFG Class
 */
CFG::CFG() : ListDigraph(), _function(*this)
{
  //_initial = INVALID;
}
