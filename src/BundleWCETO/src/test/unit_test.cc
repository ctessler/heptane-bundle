#include <cppunit/TextOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include "CFG.h"
#include "CFR.h"

int
main(int argc, char** argv) {
	CppUnit::Test *suite =
		CppUnit::TestFactoryRegistry::getRegistry().makeTest();

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(suite);

	if (runner.run()) {
		return 0;
	}
	return 1;
}
