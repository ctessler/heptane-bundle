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
#include "DOTFactory.h"
#include "JPGFactory.h"
#include "DOTfromCFR.h"
#include "DOTfromCFRG.h"
#include "EntryFactory.h"
#include "WCETOFactory.h"

void
usage(void) {
	cout << "Usage: BundleWCETO <OPTIONS> <XML Configuration File>" << endl
	     << "REQUIRED OPTIONS: --CFG --threads" << endl
	     << "OPTIONS" << endl
	     << "	-c/--CFG <file> Control Flow Graph from BundleCFG" << endl
	     << "	-m/--threads #	Number of threads to use" << endl
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
int
main(int argc, char** argv) {
	int vflag = 0, teflag = 0, tflag = 0, hflag = 0;

	/* Long form command line options */
	static struct option long_options[] = {
		{"CFG", required_argument, NULL, 'c'},
		{"help", no_argument, &hflag, 1},
		{"threads", required_argument, NULL, 'm'},
		{"trace", no_argument, &tflag, 1},
		{"verbose", no_argument, &vflag, 1},
		{0, 0, 0, 0}
	};

	string cfgfile, bcfg_file, base;
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

	for (int i=optind; i < argc; i++) {
		cfgfile = argv[i];
	}
	if (cfgfile.length() == 0) {
		cout << "No XML Config File given." << endl;		
		usage();
		return -1;
	}
	if (bcfg_file.length() == 0) {
		cout << "No Control Flow Graph [-c] given." << endl;
		usage();
		return -1;
	}
	if (n_threads == 0) {
		cout << "No number of threads [-m] given." << endl;
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

	/* Get a base name */
	base = bcfg_file.substr(0, bcfg_file.find(".cfg"));

	/* Read the configuration file, setup the caches */
	BXMLCfg xml_config(cfgfile);
	map<int, Cache*> ins_cache, dat_cache;
	xml_config.copyCaches(ins_cache, dat_cache);
	int mem_load_latency = xml_config.getLoadLatency();
	xmlCleanupParser();

	/* Read the Control Flow Graph from BundleCFG */
	CFG cfg;
	CFGReader cfgr(cfg);
	cfgr.read(bcfg_file);

	map<int, Cache*>::iterator mit;
	for (mit = ins_cache.begin(); mit != ins_cache.end(); ++mit) {
		stringstream ss;

		CFG copy(cfg);
		Cache *cache = mit->second;
		
		ss.str("");
		ss << base << "-level-" << mit->first;
		string pre = ss.str();
		ss.str("");  ss << pre << ".dot";
		DOTFactory dot(copy);
		dot.setPath(ss.str());
		dot.setCache(cache);
		cout << "DOT : " << ss.str() << endl;
		
		/* Export CFRs to JPGs */
		CFRFactory cfr_fact(copy, *cache);
		map<ListDigraph::Node, CFR*> cfrs = cfr_fact.produce();
		map<ListDigraph::Node, CFR*>::iterator cfrit;
		for (cfrit = cfrs.begin(); cfrit != cfrs.end(); ++cfrit) {
			ss.str("");
			ss << pre << "-cfr-";
			CFR* cfr = cfrit->second;
			cout << "CFRLIST " << mit->first << " CFR "
			     << *cfr << endl; 
			ListDigraph::Node cfr_initial = cfr->getInitial();
			ss << "0x" << hex << cfr->getAddr(cfr_initial) << dec
			   << ".dot";

			DOTfromCFR cfrdot(*cfr);
			cfrdot.setPath(ss.str());
			cfrdot.setCache(cache);
			cfrdot.produce();

			ListDigraph::Node cfg_initial =
				cfr->membership(cfr_initial);
			dot.setColor(cfg_initial, "yellow");
			dot.labelNodesCFR(cfr);

			JPGFactory cfrjpg(cfrdot);
			cfrjpg.produce();
		}
		/* Assigns generation IDs to CFRG nodes */
		CFRG *cfrg = cfr_fact.getCFRG();
		cfrg->order();

		/* Drop the WCET table per cache level */
		EntryFactory entries(*cfrg);
		ss.str(""); ss << pre << ".entry";
		entries.setPath(ss.str());
		entries.produce();

		/* Calculate the WCETO for each CFR */
		WCETOFactory wceto_fact(*cfrg);
		wceto_fact.setThreads(n_threads);
		CFR *initial_cfr = cfrg->findCFR(cfrg->getInitial());
		wceto_fact.produce();
		
		/* Produce the images for the Control Flow Region Graph */
		DOTfromCFRG cfrg_dot(*cfrg, wceto_fact);
		ss.str(""); ss << pre << "-cfrg.dot";
		cfrg_dot.setPath(ss.str());
		cfrg_dot.produce(n_threads);

		string path = cfrg_dot.getPath();
		JPGFactory cfrg_jpg(path);
		cfrg_jpg.produce();

		/* Produce images for the Control Flow Graphs */
		dot.produce();
		JPGFactory jpg(dot);
		jpg.produce();
		cout << "JPG : " << jpg.getPath() << endl;

		wceto_fact.dumpCFRs();
	}

	/* Cleanup */
	for (mit = ins_cache.begin(); mit != ins_cache.end(); ++mit) {
		delete mit->second;
	}
	for (mit = dat_cache.begin(); mit != dat_cache.end(); ++mit) {
		delete mit->second;
	}
	
	return 0;
}
