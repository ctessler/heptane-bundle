/* ------------------------------------------------------------------------------------

   Copyright IRISA, 2003-2014

   This file is part of Heptane, a tool for Worst-Case Execution Time (WCET) estimation.
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

   ----------------------------------------------------------------------------------- */

/*****************************************************************
                             StackInfoAttribute
	This is a sharedAttribute generated by the MIPS address analysis

	This attribute is present for all cfgs to provide
    Information about stack pointer, size of frame  without and with the parameters (present in the caller's frame)
    and calling contexts with the corresponding register sp value
    Currently, attached to cfgs during the data address analysis
	It is a serialisable attribute


*****************************************************************/


#include <map>
#include <string>
#include <cassert>
#include "CfgLib.h"

using namespace std;
using namespace cfglib;


#ifndef STACKINFOATTRIBUTE
#define STACKINFOATTRIBUTE

class StackInfoAttribute:public cfglib::SerialisableAttribute
{
    
  int frameSizeWithoutCaller;	///< this size do not consider the parameters of the function which are present in the caller's frame
  int frameSizeWithCaller;	///< this size consider the parametters of the function which are present in the caller's frame
  map < string, long >context2sp;	///< <context,sp>
    
    public:
  /** constructor */
  StackInfoAttribute ();
    
  /**cloning function */
  StackInfoAttribute *clone ();
    
  //accessors
  int getFrameSizeWithoutCaller ();
  int getFrameSizeWithCaller ();
    
  /**return the sp register corresponding to the context ctx */
  long getSP (string ctx);
    
  /** set sizeFrameWithoutCaller */
  void setFrameSizeWithoutCaller (int);
  /** set sizeFrameWithCaller */
  void setFrameSizeWithCaller (int);
    
  /**add a context calling sequence for the sp value */
  void addContext (long, string);
    
  /** Debug */
  void Print (std::ostream &);
    
  SerialisableAttribute *create() ;
    
  // Serialisation function
  ostream& WriteXml(std::ostream&,cfglib::Handle&) ;
  void ReadXml(XmlTag const*xml_node,cfglib::Handle&) ;
};
#endif
