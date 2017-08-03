/* Standard includes */
#include<iostream>
#include<getopt.h>
#include<libxml/xpath.h>
#include<sstream>

/* Heptane includes */
#include"BXMLCfg.h"

#include "arch.h"
#include "Program.h"
#include "GlobalAttributes.h"
#include "Logger.h"

/* Bundle includes */
#include "DotPrint.h"
#include "CFRFactory.h"
#include "LemonCFG.h"
#include "LemonFactory.h"
using namespace std;
using namespace cfglib;

/** Should probably make this CUNit */
int run_tests();

void initcfglib() {
	AttributesFactory *af = AttributesFactory::GetInstance();
	af->SetAttributeType(AddressAttributeName, new AddressAttribute());
	af->SetAttributeType(SymbolTableAttributeName, new SymbolTableAttribute());
}	

void usage(void) {
	cout << "Usage: BundleCFG <OPTIONS> <XML Configuration File>" << endl
	     << "OPTIONS" << endl
	     << "	-h/--help	this message" << endl
	     << "	-t/--trace	enable tracing" << endl
	     << "	--test		perform tests and exit " << endl
	     << "	-v/--verbose	enable verbose output" << endl
	     << endl;
}

/**
 * Entrypoint
 *
 * It's purpose is to parse the command line arguments, ensure their
 * validity and instigate the analysis.
 */
int main(int argc, char** argv) {
	int vflag = 0, teflag = 0, tflag = 0, hflag = 0;

	/* Long form command line options */
	static struct option long_options[] = {
		{"help", no_argument, &hflag, 1},
		{"test", no_argument, &teflag, 1},
		{"trace", no_argument, &tflag, 1},
		{"verbose", no_argument, &vflag, 1},
		{0, 0, 0, 0}
	};

	while (1) {
		int opt_ind, c;
		c = getopt_long(argc, argv, "htv", long_options, &opt_ind);
		if (c == -1) {
			/* End of parsed options */
			break;
		}
		
		switch(c) {
		case 0:
			/* Long option */
			break;
		/* Short options */	
		case 'h':
			hflag = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			/* Problem with argument parsing */
			usage();
			return -1;
		}
	}
	if (hflag) {
		usage();
		return -1;
	}
	if (teflag) {
		return run_tests();
	}
	string cfgfile;
	for (int i=optind; i < argc; i++) {
		cfgfile = argv[i];
	}
	if (cfgfile.length() == 0) {
		usage();
		return -1;
	} 

	/*
	 * Command line arguments have been parsed.
	 */

	/* Initialize libxml2 */
	xmlInitParser();
	xmlKeepBlanksDefault(1);
	xmlXPathInit();

	/* Initialize Heptane's cfglib */
	initcfglib();

	/*
	 * Present small status to the user
	 */
	cout << "Using CFG file: " << cfgfile << endl;
	
	streambuf *old_cout = cout.rdbuf();
	stringstream output;
	if (!vflag) {
		/* Redirect all cout calls to this string stream */
		cout.rdbuf(output.rdbuf());
	}

	/* Begin work */
	
	/* Read the configuration file */
	BXMLCfg config(cfgfile);

	map<int, Cache *> iCache, dCache;
	config.copyCaches(iCache, dCache);
	
	if (BXMLCfg::MIPS == config.getArch()) {
		Arch::init("MIPS", config.getEndian() == BXMLCfg::BIG);
	}
	if (BXMLCfg::ARM == config.getArch()) {
		Arch::init("ARM", config.getEndian() == BXMLCfg::BIG);
	}

	/* Read the CFG of the program */
	Program *prog = Program::unserialise_program_file(config.getCfgFile());

	/* Intermediate Result */
	string base = prog->GetName();
	DotPrint dotprint(prog, config.getWorkDir(), base, iCache, dCache);
	dotprint.PerformAnalysis();

	/* Convert to Control Flow Graph that can be analyzed */
	LemonCFG *cvtd = LemonFactory::convert(prog);
	for (map<int, Cache*>::iterator it = iCache.begin();
	     it != iCache.end(); ++it) {
		int level = it->first;
		stringstream ss;
		ss << base << "-level-" << level << ".jpg";
		cvtd->cacheAssign(it->second);
		cvtd->toJPG(ss.str());
	}

	CFRFactory::makeCFRG(prog, config.getWorkDir(), iCache, dCache);
	
	xmlCleanupParser();

	delete cvtd;

	if (!vflag) {
		/* Restore cout, now you can output status */
		cout.rdbuf(old_cout);
	}
	
	return 0;
}


int run_tests() {
	return 0;
}
