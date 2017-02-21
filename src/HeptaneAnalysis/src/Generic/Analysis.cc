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

#include <queue>
#include "Analysis.h"
#include "SharedAttributes/SharedAttributes.h"
#include "Generic/CallGraph.h"
#include "Specific/HtmlPrint/HtmlPrint.h"

Analysis::~Analysis ()
{ }

bool Analysis::CheckPerformCleanup ()
{
  // cout << " BEG--Analysis::CheckPerformCleanup () " << endl;
  if ( ! CheckInputAttributes () ) 
    { 
      Logger::addFatal ("Analysis: Error when checking attributes for analysis, exiting"); 
    }
  bool res = PerformAnalysis ();
  if (! res ) 
    {
      Logger::addFatal ("Analysis: Error when performing analysis, exiting"); 
    }
  RemovePrivateAttributes ();

  // cout << " END-- Analysis::CheckPerformCleanup () " << endl;
  return res;
}



