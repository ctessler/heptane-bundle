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
#include "CFRGFactory.h"
#include "LemonCFG.h"
#include "LemonFactory.h"


#include "CFG.h"
#include "CFGFactory.h"
#include "DOTFactory.h"
#include "JPGFactory.h"
#include "CFRFactory.h"
#include "DOTfromCFR.h"
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

	FunctionCall::test();
	CFG::test();

	CFGFactory cfgFact(prog);
	CFG *cfg = cfgFact.produce();

	DOTFactory dot(*cfg);
	dot.setPath("test.dot");
	dot.setCache(iCache[1]);
	dot.produce();

	JPGFactory jpg(dot);
	jpg.produce();

	CFRFactory cfr_fact(*cfg, *iCache[1]);
	map<ListDigraph::Node, CFR*> cfrs = cfr_fact.produce();
	map<ListDigraph::Node, CFR*>::iterator cfrit;
	cout << "CFR Identification: " << endl;
	int count = 0;
	for (cfrit = cfrs.begin(); cfrit != cfrs.end(); ++cfrit) {
		DOTfromCFR cfrDOT(*cfrit->second);
		stringstream name;
		name << "cfr-" << ++count << ".dot";
		cfrDOT.setPath(name.str());
		cfrDOT.setCache(iCache[1]);
		cfrDOT.produce();
		JPGFactory cfrJPG(cfrDOT);
		cfrJPG.produce();
		name.str("");
		name << "cfr-" << count << ".dat";
		(*cfrit->second).dump(name.str());
		cout << "  " << cfg->stringNode(cfrit->first) << endl
		     << "    --> " << *cfrit->second
		     << name.str() << endl;
		
	}
	CFRG *cfrg = cfr_fact.getCFRG();

	DOTfromCFRG dot_cfrg_fact(*cfrg);
	dot_cfrg_fact.setPath("cfrg.dot");
	dot_cfrg_fact.produce(4);
	
	JPGFactory cfrg_jpg(dot_cfrg_fact);
	cfrg_jpg.produce();
	
	return 0;

	/* Convert to Control Flow Graph that can be analyzed */
	LemonCFG *cvtd = LemonFactory::convert(prog);

	/* Display the LemonCFG for each level of the cache */
	for (map<int, Cache*>::iterator it = iCache.begin();
	     it != iCache.end(); ++it) {
		LemonCFG lcfg(*cvtd);
		int level = it->first;
		stringstream jpg_name, dot_name, dat_name, cfr_name;
		jpg_name << base << "-level-" << level << ".jpg";
		dot_name << base << "-level-" << level << ".dot";
		dat_name << base << "-level-" << level << ".dat";
		cfr_name << base << "-level-" << level << ".cfr";

		cout << "Working on Cache level " << level << endl;

		/* Perform the cache assignment */
		lcfg.cacheAssign(it->second);
		/* Assign CFR Membership */
		lcfg.getCFRMembership(it->second);

		lcfg.toCFR(cfr_name.str());
		lcfg.toFile(dat_name.str());
		lcfg.toJPG(jpg_name.str());
		lcfg.toDOT(dot_name.str());

		/* Let's look at the CFRs */
		map<ListDigraph::Node, LemonCFG*> cfrm = CFRFactory::separateCFRs(lcfg);
		map<ListDigraph::Node, LemonCFG*>::iterator cfrit;
		for (cfrit = cfrm.begin(); cfrit != cfrm.end(); cfrit++) {
			cout << "CFR [" << lcfg.getStartString(cfrit->first) << "] has nodes: " << endl;
			LemonCFG *cfr = cfrit->second;
			for (ListDigraph::NodeIt nit(*cfr); nit != INVALID; ++nit) {
				ListDigraph::Node cursor = nit;
				cout << "\t" << cfr->getStartString(cursor) << endl;
			}
		}
		for (cfrit = cfrm.begin(); cfrit != cfrm.end(); cfrit++) {
			LemonCFG *cfr = cfrit->second;
			cout << "CFR [" << cfr->getStartString(cfr->getRoot()) 
			     << "] WCET : " << cfr->CFRWCET(cfr->getRoot(), 5, *it->second)
			     << endl;
		}
		for (cfrit = cfrm.begin(); cfrit != cfrm.end(); cfrit++) {
			delete cfrit->second;
		}

		LemonCFG cfg_copy(*cvtd);
		CFRGFactory fact(cfg_copy, *it->second);
		LemonCFRG *cfrg = fact.run();
		cfrg->dump();
		
	}


	/* Clean up and shutdown */
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
