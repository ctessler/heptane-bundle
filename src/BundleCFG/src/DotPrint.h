/* ---------------------------------------------------------------------

   Copyright IRISA, 2003-2014

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET)
   estimation.
   APP deposit IDDN.FR.001.510039.000.S.P.2003.000.10600

   Heptane is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Heptane is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details (COPYING.txt).

   See CREDITS.txt for credits of authorship

   ------------------------------------------------------------------------ */

#ifndef DOT_PRINT_H
#define DOT_PRINT_H

#include "Cache.h"
#include "Analysis.h"
#include "Generic/CallGraph.h"

// Name of internal attributes used (and removed at the end of the analysis)
#define InternalAttributeNameOK "OK"

/**
 * Graphical printing of CFG structure, using graphwiz
 */
class DotPrint:public Analysis {
private:
	string working_directory, filename_base;
	map<int, Cache*> &_icache;
	map<int, Cache*> &_dcache;

	void displayCfg(Cfg * c, ofstream & os);
	bool displayNode(Cfg* c, Node* node, ofstream & os);
	void displayNodeAsSubgraph(Node* node, ofstream& os);
	bool displayLoop(Cfg* c, Loop* l, ofstream& os, vector<Loop *>& vl);
public:
	/**
	 * Constructor
	 *
	 * @param[in] p the program being turned into a DOT and rendered files
	 * @param[in] wdir working directory where the intermediate
	 *            DOT file and rendered files will be placed 
	 * @param[in] fbase filename base for the DOT and rendered
	 *            files
	 * @param[in] iCache reference to a multi-level instruction cache
	 * @param[in] dCache reference to a multi-level data cache
	 *
	 *
	 * wdir and fbase work together in the following manner.
	 * If wdir is "/home/happy/working" and
	 * fbase is "whistle" then the following files will be created
	 *
	 *    /home/happy/working/whistle.dot
	 *    /home/happy/working/whistle.jpg
	 *    /home/happy/working/whistle.pdf
	 */
	DotPrint(Program *p, string wdir, string fbase, map<int, Cache*> &iCache, map<int, Cache*> &dCache);
	/**
	 *  Checks if all required attributes are in the CFG.
	 * @return always true (nothing specific to do) 
	 */
	bool CheckInputAttributes() { return true; }
	/**
	 * Performs the analysis
	 * @return true if successful, false otherwise
	 */
	bool PerformAnalysis();
	/**
	 * Remove all private attributes
	 */
	void RemovePrivateAttributes();
};

#endif /* DOT_PRINT_H */

