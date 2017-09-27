/* Standard includes */
#include<iostream>
#include<getopt.h>
#include<libxml/xpath.h>
#include<sstream>
#include<string>
using namespace std;

#include "BXMLCfg.h"
#include "CFGReadWrite.h"
#include "CFRFactory.h"

int
run_tests(void) {
	return 0;
}

void
usage(void) {
	cout << "Usage: BundleWCETO <OPTIONS> <XML Configuration File>" << endl
	     << "REQUIRED OPTIONS: --CFG --threads" << endl
	     << "OPTIONS" << endl
	     << "	-c/--CFG <file> Control Flow Graph from BundleCFG" << endl
	     << "	-m/--threads #	Number of threads to use" << endl
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
int
main(int argc, char** argv) {
	int vflag = 0, teflag = 0, tflag = 0, hflag = 0;

	/* Long form command line options */
	static struct option long_options[] = {
		{"CFG", required_argument, NULL, 'c'},
		{"help", no_argument, &hflag, 1},
		{"test", no_argument, &teflag, 1},
		{"threads", required_argument, NULL, 'm'},
		{"trace", no_argument, &tflag, 1},
		{"verbose", no_argument, &vflag, 1},
		{0, 0, 0, 0}
	};

	string cfgfile, bcfg_file;
	unsigned int n_threads = 0;
		
	while (1) {
		int opt_ind, c;
		c = getopt_long(argc, argv, "c:hm:tv", long_options, &opt_ind);
		if (c == -1) {
			/* End of parsed options */
			break;
		}
		
		switch(c) {
		case 0:
			/* Long option */
			break;
		/* Short options */
		case 'c':
			bcfg_file = optarg;
			break;
		case 'h':
			hflag = 1;
			break;
		case 'm':
			n_threads = atoi(optarg);
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

	for (int i=optind; i < argc; i++) {
		cfgfile = argv[i];
	}
	if (cfgfile.length() == 0) {
		usage();
		return -1;
	}
	if (bcfg_file.length() == 0) {
		usage();
		return -1;
	}
	if (n_threads == 0) {
		usage();
		return -1;
	}
	
	cout << "Configuration file: " << cfgfile << endl
	     << "Bundle CFG file: " << bcfg_file << "\t"
	     << "Thread Count: " << n_threads << endl;

	/*
	 * Command line arguments have been parsed.
	 */

	/* Initialize libxml2 */
	xmlInitParser();
	xmlKeepBlanksDefault(1);
	xmlXPathInit();

	/* Read the configuration file, setup the caches */
	BXMLCfg xml_config(cfgfile);
	map<int, Cache*> ins_cache, dat_cache;
	xml_config.copyCaches(ins_cache, dat_cache);

	/* Read the Control Flow Graph from BundleCFG */
	CFG cfg;
	CFGReader cfgr(cfg);
	cfgr.read(bcfg_file);

	map<int, Cache*>::iterator mit;
	for (mit = ins_cache.begin(); mit != ins_cache.end(); ++mit) {
		CFRFactory cfr_fact(cfg, *mit->second);
		map<ListDigraph::Node, CFR*> cfrs = cfr_fact.produce();
		break;
	}	
	

	/* Cleanup */
	for (mit = ins_cache.begin(); mit != ins_cache.end(); ++mit) {
		delete mit->second;
	}
	for (mit = dat_cache.begin(); mit != dat_cache.end(); ++mit) {
		delete mit->second;
	}
	
	xmlCleanupParser();
	return 0;
}
