#include<iostream>
#include<getopt.h>
#include"BXMLCfg.h"
#include<libxml/xpath.h>

#include "arch.h"
#include "Program.h"
#include "GlobalAttributes.h"
#include "Logger.h"
#include "DotPrint.h"
#include "CFRFactory.h"
using namespace std;
using namespace cfglib;

void initcfglib() {
	AttributesFactory *af = AttributesFactory::GetInstance();
	af->SetAttributeType(AddressAttributeName, new AddressAttribute());
	af->SetAttributeType(SymbolTableAttributeName, new SymbolTableAttribute());
#if 0
	af->SetAttributeType(ARMWordsAttributeName, new ARMWordsAttribute());
	af->SetAttributeType(StackInfoAttributeName, new StackInfoAttribute());
	af->SetAttributeType(CodeLineAttributeName, new CodeLineAttribute());
	af->SetAttributeType(ContextListAttributeName, new ContextList());
	af->SetAttributeType(ContextTreeAttributeName, new ContextTree());
#endif
}	

void usage(void) {
	cout << "Usage: BundleCFG <OPTIONS> <XML Configuration File>" << endl
	     << "OPTIONS" << endl
	     << "	-h/--help	this message" << endl
	     << "	-t/--trace	enable tracing" << endl
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
	int vflag = 0, tflag = 0, hflag = 0;

	/* Long form command line options */
	static struct option long_options[] = {
		{"help", no_argument, &hflag, 1},
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

	string cfgfile;
	for (int i=optind; i < argc; i++) {
		cfgfile = argv[i];
	}
	if (cfgfile.length() == 0) {
		usage();
		return -1;
	} else {
		cout << "Using configuration file: " << cfgfile << endl;
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
	
	/* Read the configuration file */
	BXMLCfg cfg(cfgfile);

	map<int, Cache *> iCache, dCache;
	cfg.copyCaches(iCache, dCache);
	
	if (BXMLCfg::MIPS == cfg.getArch()) {
		Arch::init("MIPS", cfg.getEndian() == BXMLCfg::BIG);
	}
	if (BXMLCfg::ARM == cfg.getArch()) {
		Arch::init("ARM", cfg.getEndian() == BXMLCfg::BIG);
	}

	
	/* Read the CFG of the program */
	cout << "Using CFG file: " << cfg.getCfgFile() << endl; 
	Program *prog = Program::unserialise_program_file(cfg.getCfgFile());

	/* Intermediate Result */
	DotPrint dotprint(prog, cfg.getWorkDir(), iCache, dCache);
	dotprint.PerformAnalysis();

	CFRFactory::makeCFRG(prog, cfg.getWorkDir(), iCache, dCache);
	
	xmlCleanupParser();
	return 0;
}