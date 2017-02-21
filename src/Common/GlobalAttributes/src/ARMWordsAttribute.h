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

/*****************************************************************
                             ARMWordsAttribute
	This is a sharedAttribute generated by the Extractor

	This attribute is attached to ARM instructions using 
    .word information

*****************************************************************/

#include <utility>
#include <string>
#include <map>
#include "CfgLib.h"

#ifndef ARMWORDATTRIBUTE
#define ARMWORDATTRIBUTE

// table = {< address, [ type, value]>} , type ::= imm | .bss | .data
typedef pair< string, unsigned long>  ARMWordsAttributeElemTableValue;
typedef map<unsigned long, ARMWordsAttributeElemTableValue >  ARMWordsAttributeTable; 

class ARMWordsAttribute : public cfglib::SerialisableAttribute
{
 private:

  ARMWordsAttributeTable words; //addr -> <type,value>
    
 public:

  /*!constructor*/
  ARMWordsAttribute();

  /*!cloning function*/
  ARMWordsAttribute *clone();

  // Serialisation function
  ostream& WriteXml(std::ostream&,cfglib::Handle&);
  void ReadXml(XmlTag const*xml_node,cfglib::Handle&) ;

  SerialisableAttribute *create() ;

  //debug
  void Print(std::ostream&);
    
  //add a word
  void addWord(unsigned long addr,string type,unsigned long value);


  ARMWordsAttributeTable getTable ();

}; 
#endif
