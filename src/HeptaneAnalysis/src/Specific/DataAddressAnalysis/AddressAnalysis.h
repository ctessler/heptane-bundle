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

   ------------------------------------------------------------------------------------ */

/*****************************************************************
                             AddressAnalysis

This is the entry point of the dataflow analysis of 
data and stack addresses for a architecture (independant of the architecture).
The MIPSAddressAnalysis and ARMAddressAnalysis classes inhrit of this class.

This analysis is very basic in the sense:
- it does not consider pointers 
- it is perfomed on an intra-basic block basis

*****************************************************************/


#ifndef ADDRESSANALYSIS_H
#define ADDRESSANALYSIS_H


#include <ostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <vector>
#include <set>
#include <map>

#include <errno.h>

#include "RegState.h"

#include "Analysis.h"

using namespace std;
class AddressAnalysis: public Analysis
{
  
 private:

    bool CheckExternalCfg(Program *p); ///< return true if the program reference an external CFG, false otherwise.

    bool VerifModifStack (Cfg * cfg);
    int getStackMaxOffset (Cfg * cfg, int stack_size);
    void intraBlockDataAnalysis ();

       // The folowings are architecture dependent.
    virtual int getStackMaxOffset(string s_instr, int StackMaxOffset)=0;
    virtual int getStackSize (Cfg * cfg)=0;///< return the size of the stack frame
    virtual void intraBlockDataAnalysis (Cfg * vCfg, Node * aNode)=0; ///< Intra basic block data analysis for a node(aNode) of a cfg (vCfg)
    virtual bool setLoadStoreAddressAttribute(Cfg * vCfg, Instruction* vinstr, RegState *state) = 0;
 protected:
    SymbolTableAttribute symbol_table; // Symbol table (set in intraBlockDataAnalysis())
    long spinit; ///< Initial value of the stack pointer
    bool isCfgWithCallNode (Cfg * cfg); ///< @return true if a cfg (cfg) calls an another cfg, "false" otherwise  
    bool stackAnalysis (); ///< Stack analysis starting point
    void setAddressAttribute(Instruction* vinstr, AddressInfo & info);
    void setPointerAccessInfo(Instruction* vinstr, string access);
    void intraBlockDataAnalysis (Cfg * vCfg, Node * aNode, RegState *state);
    void mkAddressInfoAttribute(Instruction *  Instr, string access, bool precision, string section_name, string var_name, long addr,  int size);
 public:
  AddressAnalysis (Program * p, int sp);
  bool PerformAnalysis ();
  bool CheckInputAttributes ();
  void RemovePrivateAttributes ();
};

#endif
