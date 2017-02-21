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

#include "Logger.h"
#include "AddressAnalysis.h"
#include "Generic/CallGraph.h"
#include "Generic/ContextHelper.h"
#include "arch.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


void AddressAnalysis::setAddressAttribute(Instruction* vinstr, AddressInfo & info)
{ 
  AddressAttribute attribute;

  if (vinstr->HasAttribute (AddressAttributeName))
    {
      attribute = (AddressAttribute &) vinstr->GetAttribute (AddressAttributeName);
    }
  attribute.addInfo (info);
  vinstr->SetAttribute (string (AddressAttributeName), attribute);
}

/**
   Check if a cfg is a leaf in the call graph (no dedicated stack frame in mips).
   @return true if a cfg (cfg) calls an another cfg, "false" otherwise.
 */
bool AddressAnalysis::isCfgWithCallNode (Cfg * cfg)
{
  vector < Node * >nodes = cfg->GetAllNodes ();
  for (size_t i = 0; i < nodes.size (); i++)
    {
      if (nodes[i]->IsCall ()) return true;
    }
  return false;
}



/**
 * This is used to check if the stack pointer is modified in the middle of a function
 *
 * @return true if sp is modified, false otherwise.
 */
bool AddressAnalysis::VerifModifStack (Cfg * cfg)
{
  Node *firstNode = cfg->GetStartNode ();
  const vector < Node * >&endNodes = cfg->GetEndNodes ();
  const vector < Node * >&listNodes = cfg->GetAllNodes ();

  for (size_t n = 0; n < listNodes.size (); n++)
    if (listNodes[n] != firstNode)	// not first node
      if (std::find (endNodes.begin (), endNodes.end (), listNodes[n]) == endNodes.end ())	//not in endNodes
	{
	  const vector < Instruction * >&listInstr = listNodes[n]->GetAsm ();
	  // For each instruction in node
	  for (size_t i = 0; i < listInstr.size (); i++)
	    {
	      string s_instr = listInstr[i]->GetCode ();
	      vector < string > output_registers = Arch::getResourceOutputs (s_instr);
	      for (size_t j = 0; j < output_registers.size (); j++)
		if (output_registers[j] == "sp") return true;
	    }
	}
  return false;
}

/**
 * This is used to get the maximum offset relative to sp for instruction like : mnemonic $1,val($sp) of a cfg
 * @return the size of the stack frame including the parameter present in the caller function if used.
 */
int AddressAnalysis::getStackMaxOffset (Cfg * cfg, int stack_size)
{
  int result = stack_size;

  const vector < Node * >&nodes = cfg->GetAllNodes ();
  for (size_t n = 0; n < nodes.size (); n++)
    {
      const vector < Instruction * >&instructions = nodes[n]->GetAsm ();

      // For each instruction in the current node
      for (size_t i = 0; i < instructions.size (); i++)
	{
	  string s_instr = instructions[i]->GetCode ();
	  if (Arch::isLoad (s_instr) || Arch::isStore (s_instr))
	    result = getStackMaxOffset(s_instr, result); // ARCH Dependant.
	}
    }
  return result;
}

/**
   Stack analysis starting point
*/
bool
AddressAnalysis::stackAnalysis ()
{
  CallGraph callgraph (p);

  vector < Cfg * >listCfg = p->GetAllCfgs ();

  // Check that modifications of the stack are limited to first and end nodes.
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      Cfg * current_cfg = listCfg[cfg];
      if ( ! callgraph.isDeadCode (current_cfg)) // Ignore dead cfgs
	if (VerifModifStack (current_cfg))
	  Logger::addError ("modification of the stack frame in the middle of function:" + current_cfg->getStringName());
    }

  if (Logger::getErrorState ()) { Logger::print (); return false; }


  //now the analysis

  //compute the stack size information
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      Cfg * current_cfg = listCfg[cfg];
      if (! callgraph.isDeadCode (current_cfg)) // Ignore dead cfgs
	{
	  assert (! current_cfg->HasAttribute (StackInfoAttributeName));
	  StackInfoAttribute attribute;

	  //set the stackFrameSizeWithoutCaller
	  int stack_size = getStackSize (current_cfg);
	  attribute.setFrameSizeWithoutCaller (stack_size);

	  int maxOffset = getStackMaxOffset (current_cfg, stack_size);
	  attribute.setFrameSizeWithCaller (maxOffset);

	  //attach the attribute to the current cfg
	  current_cfg->SetAttribute (string (StackInfoAttributeName), attribute);
	}
    }
  // compute the [sp ; contexts] values
  map < Context *, long >sp_values;
  sp_values[NULL] = spinit;

  const ContextTree & contexts = (ContextTree &) p->GetAttribute (ContextTreeAttributeName);

  // For each context in the tree
  for (size_t c = 0; c < contexts.getContextsCount (); ++c)
    {
      Context *context = contexts.getContext (c);
      Cfg *currentCfg = context->getCurrentFunction ();

      //get the StackInfoAttributeName of current cfg
      assert (currentCfg->HasAttribute (StackInfoAttributeName));
      StackInfoAttribute attribute = (StackInfoAttribute &) currentCfg->GetAttribute (StackInfoAttributeName);

      //get the stack size without caller of current cfg
      long stack_size = attribute.getFrameSizeWithoutCaller ();

      // Find caller sp and current function stack size
      assert (sp_values.find (context->getCallerContext ()) != sp_values.end ());
      long caller_sp = sp_values[context->getCallerContext ()];

      // Compute and save current context sp value
      long sp_value = caller_sp - stack_size;
      sp_values[context] = sp_value;

      //add (sp_value ; ctx_str) to the attribute if not already present
      attribute.addContext (sp_value, context->getStringId ());

      //attach the attribute to the current cfg
      currentCfg->SetAttribute (string (StackInfoAttributeName), attribute);
    }

  return true;
}

bool AddressAnalysis::CheckExternalCfg(Program *p)
{

  vector < Cfg * >listCfg = p->GetAllCfgs ();
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      if (listCfg[cfg]->GetStartNode () == NULL) return true;
    }
  return false;
}

void AddressAnalysis::intraBlockDataAnalysis (Cfg * vCfg, Node * aNode, RegState *state)
{
  // For all instructions of the current node
  vector < Instruction * >asm_instr = aNode->GetAsm ();
  for (size_t j = 0; j < asm_instr.size (); j++)
    {
      Instruction* vinstr= asm_instr[j];
      string vinstr_code = vinstr->GetCode();
      TRACE( cout << "\n >>>>>> ---BEGIN---AddressAnalysis::intraBlockDataAnalysis (" << vinstr_code << ")" << endl);
      // Setting the address attribute for load and Store instructions.
      if (setLoadStoreAddressAttribute(vCfg, vinstr, state))
	TRACE(cout << "   setLoadStoreAddressAttribute APPLIED " << endl);
      // Simulate the execution of the instruction
      state->simulate (vinstr);
      TRACE( cout << " <<<<<< --- END ---AddressAnalysis::intraBlockDataAnalysis (" << vinstr_code << ")" << endl);
    }
}

// intra block data analysis starting point
void AddressAnalysis::intraBlockDataAnalysis ()
{
  Cfg *CurrentCfg;
  
  CallGraph callgraph (p);

  //get the symbolTable
  assert (p->HasAttribute (SymbolTableAttributeName));
  symbol_table = (SymbolTableAttribute &) p->GetAttribute (SymbolTableAttributeName);

  vector < Cfg * >listCfg = p->GetAllCfgs ();
  for (size_t cfg = 0; cfg < listCfg.size (); cfg++)
    {
      CurrentCfg = listCfg[cfg];
      // Ignore dead cfgs
      if (callgraph.isDeadCode (CurrentCfg)) {continue;}
      //for all nodes of the current CFG.
      vector < Node * >listNodes = CurrentCfg->GetAllNodes ();
      for (size_t i = 0; i < listNodes.size (); i++)
	{
	  TRACE(cout << "========================================================================================= " << endl);
	  TRACE(cout << " ++++++++++++++++++++++ intraBlockDataAnalysis:: New CFG:reset of  the registers ! +++" << endl);
	  TRACE(cout << "========================================================================================= " << endl);
	  intraBlockDataAnalysis (CurrentCfg, listNodes[i]); // Architexture dependent.
	}
    }
}

void AddressAnalysis::setPointerAccessInfo(Instruction* vinstr, string access)
{
  Logger::addWarning ("pointer analysis does not exist: stub all addresses can be accessed");
  
  long addr_begin = symbol_table.getCodeStartAddr ();
  long addr_end = spinit;
  
  // Fill access address info
  AddressInfo info;
  info.setPrecision(false);
  info.setSegment ("all");
  info.setName ("pointer fallback");
  info.setType (access);
  ostringstream ossAdr, ossSize;
  ossAdr << addr_begin;
  ossSize << addr_end - addr_begin + 1;
  // For MIPS, values are (addr = 4194328, Size = 2143288809)
  info.addAdrSize (ossAdr.str (), ossSize.str ());
  setAddressAttribute(vinstr, info);
  TRACE(info.print());
}


void AddressAnalysis::mkAddressInfoAttribute(Instruction *  Instr, string access, bool precision, string section_name, string var_name, long addr,  int size)
{
  ostringstream ossAdr, ossSize;
  AddressInfo info;

  info.setType (access);
  info.setPrecision(precision);
  info.setSegment (section_name);
  info.setName (var_name);
  ossAdr << addr;
  ossSize << size;
  info.addAdrSize (ossAdr.str (), ossSize.str ());
  setAddressAttribute(Instr, info);
  TRACE(info.print());
}



//-----------Public -----------------------------------------------------------------

AddressAnalysis::AddressAnalysis (Program * p, int sp):Analysis (p)
{
  spinit = sp;
  //initialization of sp value with 7FFF FFFF - virtual pages alignement
  //long spinit=2147483647 - ( 2147483647 % TAILLEPAGE);
}

bool
AddressAnalysis::PerformAnalysis ()
{
  if (CheckExternalCfg(p)) { Logger::addFatal ("AddressAnalysis: external Cfg detected in DataAddressAnalysis");}
  stackAnalysis ();
  intraBlockDataAnalysis ();
  Logger::print ();

  return !(Logger::getErrorState ());
}

// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool
AddressAnalysis::CheckInputAttributes ()
{
  return p->HasAttribute (SymbolTableAttributeName);
}

//nothig to remove
void
AddressAnalysis::RemovePrivateAttributes ()
{}

