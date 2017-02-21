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

#include "ARMAddressAnalysis.h"
#include "Utl.h"
#include "ARM.h"
#include "arch.h"


int ARMAddressAnalysis::getStackSize( Instruction * vinstr)
{
  // Split the first instruction (instr1)
  vector < string > v_instr = Arch::splitInstruction (vinstr->GetCode ());
  string  codeop = v_instr[0];

  if (codeop == "sub")  // allocating locals sub sp, sp, VALUE
    {
      if (v_instr[1] != "sp") return 0;
      if (v_instr[2] != "sp") return 0;
      assert (Utl::isDecNumber (v_instr[3]));
      int vsize = atoi (v_instr[3].c_str ());
      return abs(vsize);
    }
  // -----
  if ( codeop == "push") // saving registers
    { 
      int n = Arch::getSizeRegisterList(v_instr[1]);  // push general , ie { rI, rJ-rK, rL, ...}
      return (n * 4);
    }

  // Not yet implemented  stm*   sp!, {r0, r2-r4} : multiple store
  assert(codeop.find("stm") == EOS);
  return 0;
}

int ARMAddressAnalysis::getStackSize (Cfg * cfg)
{ 
  int i, stackSize = 0;
  Instruction * vinstr;
  bool todo =true;
  Node *firstNode = cfg->GetStartNode ();
  const vector < Instruction * >&listInstr = firstNode->GetAsm ();
  int n = listInstr.size ();
  long vaddr1, vaddr2;

  TRACE(cout << " DEBUG__LB ARMAddressAnalysis::getStackSize (" << cfg->getStringName() << ")" << endl);
  assert (n != 0);
  if (n >= 2)
    {
      if (InstructionARM::getCodeAddress(listInstr[0], &vaddr1) && InstructionARM::getCodeAddress(listInstr[1], &vaddr2))
	if (vaddr1 == vaddr2)
	  {
	    // The push has been replaced by a set of store (str) and sub sp, sp ,<value> instructions.
	    stackSize = getStackSize(listInstr[0]);
	    i = 1;
	    vinstr = listInstr[i];
	    while ( i < n && (vaddr1 == vaddr2))
	      { 
	       stackSize = stackSize + getStackSize(vinstr);
	       i++;
	       vinstr = listInstr[i];
	       if (!InstructionARM::getCodeAddress(vinstr, &vaddr2)) vaddr2=-1;
	    }
	    if ( i < n ) stackSize = stackSize + getStackSize(vinstr);  //  may be a sub sp, sp ,<value>
	    todo = false;
	  }
    }
  if (todo)
    {
      // first instr is a push. Second may be a sub sp, sp ,<value>
      stackSize = getStackSize(listInstr[0]);
      if (n >= 2) stackSize = stackSize + getStackSize(listInstr[1]);
    }
  TRACE(cout << " DEBUG__LB ARMAddressAnalysis::getStackSize() = " << stackSize << endl);

  // a leaf in the call graph (no dedicated stack frame in ARM) is accepted.
  if (stackSize == 0) { assert ( ! isCfgWithCallNode(cfg));}
  return stackSize;
}

//  Waiting for  mnemonic regout [regin, val]
bool ARMAddressAnalysis::extractRegVal(string instr, string &reg, string &val)
{
  vector < string > v_instr = Arch::splitInstruction (instr);
  assert (v_instr.size () == 3);	// mnemonic op1, [xxx]
  string mem_pattern = v_instr[2];
  if ( ! Utl::replace(mem_pattern, '[', ' ')) return false;
  if ( ! Utl::replace(mem_pattern, ']', ' ')) return false;
  Utl::replace(mem_pattern, ',', ' ');
  val ="";  // case "codeop [sp]"
  istringstream parse (mem_pattern);
  parse >> reg >> val;
  return true;
}


long ARMAddressAnalysis::GetOffsetValue(string asm_code)
{
  string reg, val;
  assert(extractRegVal( asm_code, reg, val));
  return  Utl::string2long(val);
}


int ARMAddressAnalysis::getStackMaxOffset(string s_instr, int StackMaxOffset)
{
  string val, reg;

  //  LBesnard : on peut acceder à des paramètres empilés ? dépendant compilateur
  //    On peut avoir mnemonic op1, [sp] OU mnemonic op1, [sp, val]
  //    Voir pour   mnemonic op1, [PC, val] ???
  if (s_instr.find ("[sp") != EOS)
    {
      if (extractRegVal( s_instr, reg, val))
	{
	  assert (reg == "sp");
	  return Utl::imax(abs(Utl::string2int(val)),StackMaxOffset);
	}
    }
  return StackMaxOffset;
}

// COPY of MIPS
void ARMAddressAnalysis::analyzeStack (Cfg * cfg, Instruction * Instr, long offset, string access, int sizeOfMemoryAccess, bool precision)
{
  //get the StackInfoAttributeName of current cfg
  assert (cfg->HasAttribute (StackInfoAttributeName));
  StackInfoAttribute attribute = (StackInfoAttribute &) cfg->GetAttribute (StackInfoAttributeName);

  //get the stack information of cfg
  long stack_size = attribute.getFrameSizeWithoutCaller ();
  long stack_maxoffset = attribute.getFrameSizeWithCaller ();

  TRACE(cout << "analyzeStack, stack_size = " << stack_size << " , stack_maxoffset = " << stack_maxoffset << endl);
  if (stack_maxoffset < offset)
    {
      Logger::addError ("stack analysis -> maxOffset < offset");
      //in case of error: stack_maxoffset computation needs to be checked
      //accesses to take into account:
      //addiu $v0,$sp,8
      //lw $a1,7($v0)
    }
  else
    {
      // fill general access address info
      AddressInfo base_info;
      base_info.setType (access);
      base_info.setSegment ("stack");

      //for each context
      const ContextList & contexts = (const ContextList &) cfg->GetAttribute (ContextListAttributeName);
      for (ContextList::const_iterator context = contexts.begin (); context != contexts.end (); ++context)
	{
	  long sp = attribute.getSP ((*context)->getStringId ());
	  TRACE(cout << "analyzeStack, context --sp = " <<  std::hex << sp << std::dec << endl);

	  long stack_maxoffset_caller;

	  //check that the stack size is equal to the caller stack size
	  //if the function is without stack frame (leaf of the context tree)
	  //and keep the stack_maxoffset of the caller in case !precision
	  if (stack_size == 0)
	    {
	      Context *context_caller = (*context)->getCallerContext ();
	      assert (context_caller != NULL);
	      Cfg *caller = context_caller->getCurrentFunction ();

	      assert (caller->HasAttribute (StackInfoAttributeName));
	      StackInfoAttribute caller_attribute = (StackInfoAttribute &) caller->GetAttribute (StackInfoAttributeName);

	      long sp_caller = caller_attribute.getSP (context_caller->getStringId ());

	      stack_maxoffset_caller = caller_attribute.getFrameSizeWithCaller ();

	      assert (sp == sp_caller);
	    }
	  //--

	  long addr, size;
	  base_info.setPrecision(precision);
	  if (precision)
	    {
	      addr = sp + offset;
	      size = sizeOfMemoryAccess;
	    }
	  else
	    {
	      addr = sp;
	      // TODO: check the MIPS ABI if the parameters of the caller can be accessed if not stack_size_caller can be used instead
	      if (stack_size == 0) size = stack_maxoffset_caller; else size = stack_maxoffset;
	    }

	  // Fill access address info
	  AddressInfo contextual_info = base_info;

	  ostringstream ossAdr, ossSize;
	  ossAdr << addr;
	  ossSize << size;
	  contextual_info.addAdrSize (ossAdr.str (), ossSize.str ());

	  // Attach the contextual address attribute to the instruction.
	  string attribute_name = AnalysisHelper::mkContextAttrName(AddressAttributeName, *context);
	  assert (! Instr->HasAttribute (attribute_name));
	  AddressAttribute contextual_address;
	  contextual_address.addInfo (contextual_info);
	  Instr->SetAttribute (attribute_name, contextual_address);
	  TRACE(cout << "attribute_name = " << attribute_name << endl; contextual_info.print());
	}
    }
}




// COPY OF MIPS !!!
// set the AddressAttribute for a gp or lui access; 
// local to setLoadStoreAddressAttribute
void ARMAddressAnalysis::analyzeReg (Instruction * Instr, long addr, string access, int sizeOfMemoryAccess, bool precision)
{
  string var_name;
  unsigned long start_addr = 0;
  int size = 0;
  string section_name;

  symbol_table.getInfo (addr, &var_name, &start_addr, &size, &section_name);

  if (start_addr == 0 || size == 0)
    {
      Logger::addError ("reg analysis -> access to unknown area");
    }
  else
    {
      mkAddressInfoAttribute(Instr, access, precision, section_name, var_name,  ( precision ? addr: start_addr) , ( precision ? sizeOfMemoryAccess :size ) );
    }
}


// Setting the address attribute for load and Store instructions (Memory instructions)
// @return true when it is applied, false otherwise.
bool ARMAddressAnalysis::setLoadStoreAddressAttribute(Cfg * vCfg, Instruction* vinstr, RegState *state)
{
  string reg, codeinstr, access, asm_code;
  bool WriteBack;
  vector < string > regList;
  long loffset;

  asm_code = vinstr->GetCode ();

  bool isLoad = Arch::isLoad (asm_code);
  bool isStore = Arch::isStore (asm_code);

  if (!isLoad && ! isStore) return false;
  // LOAD = { pop, ldr* simple, ldm* --load multiple }
  // STORE= { push, str* store simple, sdm* --store multiple }

  // get the size of the access according to the op code
  int sizeOfMemoryAccess = Arch::getSizeOfMemoryAccess (asm_code);
  
  if (isLoad) access = "read"; else access = "write"; // access type
  bool b = false;
  if ( Arch::getMultipleLoadStoreARMInfos(asm_code, codeinstr, reg, regList, &WriteBack))
    if (reg.find ("sp") != EOS)
      {
	// ldm sp, { registers} ; or  stm sp  {registers} 
	// ldm sp, {r0, r1}  <=> ldr r0 [sp];  ldr r1 [sp, 4]
	// stm sp, {r0, r1}  <=> str r0 [sp];  ldr r1 [sp, 4]
	// with !, sp = sp + 4
	// More general for STRXY LDRXY with XY= IA IB DF DB
 	cout << " ====================== TODO TODO ARMAddressAnalysis::setLoadStoreAddressAttribute " << endl;
	TRACE( cout << "Registres :" << endl; for (int i=0; i< regList.size(); i++) { cout << ' ' << regList[i]; } );
	cout << asm_code << "   ######################## TODO TODO " << endl;
	if (isLoad) loffset = Arch::getNumberOfLoads(asm_code); else loffset = Arch::getNumberOfStores(asm_code);
	loffset = loffset * sizeOfMemoryAccess;
	analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, true);
	b = true;
      }
  
  if (!b)
    {
      //access using $sp register
      if (asm_code.find ("sp") != EOS)
	{
	  loffset = GetOffsetValue(asm_code);
	  TRACE( cout << "ARMAddressAnalysis::setLoadStoreAddressAttribute (sp) instr = " <<  asm_code << ", loffset= "  
		 << loffset << " , sizeOfMemoryAccess= " << sizeOfMemoryAccess << endl);
	  analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, true);
	}
      else
	/* access using $sp register
	if (asm_code.find("push") != EOS)
	  {
	    loffset = Arch::getNumberOfStores(asm_code) * 4;
	    analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, true);
	  }
	else
	  //access using $sp register
	  if (asm_code.find("pop") != EOS)
	    {
	      loffset = Arch::getNumberOfLoads(asm_code) * 4;
	      analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, true);
	    }
	    else */
	    {
	      vector < string > access_pattern;
	      state->accessAnalysis (vinstr, access_pattern);
	
	      TRACE( cout << "  === state->accessAnalysis returns, code = " << access_pattern[0] << endl);
	      TRACE( cout << "  === state->accessAnalysis returns, value = " << access_pattern[1] << endl);
	      TRACE( cout << "  === state->accessAnalysis returns, precision = " << access_pattern[2] << endl);
	      TRACE( if (access_pattern.size() == 4) cout << "  === state->accessAnalysis returns,  4th operand = " << access_pattern[3] << endl);
	      TRACE( if (access_pattern.size() == 5) cout << "  === state->accessAnalysis returns,  5th operand = " << access_pattern[4] << endl);
	    
	      string p0 = access_pattern[0];
	      bool prec =  access_pattern[2] == "1";
	      // access using $reg different from $sp
	      // Example ldr R, [pc, #val] et MEM[pc + val] is an immediate value ( load a immediate value)
	      if (p0 == "immWord")  
		{
		  string s_addr = access_pattern[3];
		  long addr = Utl::string2long(s_addr);;
		  TRACE( cout << "  === state->accessAnalysis returns, address of the IMMEDIATE VALUE = " << s_addr << endl);
		  mkAddressInfoAttribute(vinstr, access, prec, ".data", "", addr, sizeOfMemoryAccess);
		}
	      else
		if (p0 == "lui")
		  {
		    long addr = Utl::string2long(access_pattern[1]);
		    analyzeReg (vinstr, addr, access, sizeOfMemoryAccess, prec);
		  }
		else
		  if (p0 == "gp")
		    {
		      long addr = Utl::string2long(access_pattern[1]);
		      analyzeReg (vinstr, addr, access, sizeOfMemoryAccess, prec);
		    }
		  else if (p0 == "sp")
		    {
		      loffset = Utl::string2long(access_pattern[1]);
		      TRACE(cout << " Stack pointer ----*****------" << asm_code << endl;);
		      analyzeStack (vCfg, vinstr, loffset, access, sizeOfMemoryAccess, prec);
		    }
		  else //- unknown- pointer: all addresses can be accessed (stub)
		    {
		      TRACE(cout << " POINTER ----*****------" << asm_code  << endl;);
		      setPointerAccessInfo(vinstr, access);
		    }
	    }
    }

  return true;
}


void ARMAddressAnalysis::intraBlockDataAnalysis (Cfg * vCfg, Node * aNode)
{
  AddressAnalysis::intraBlockDataAnalysis (vCfg, aNode,  new ARMRegState());
}



//-----------Public -----------------------------------------------------------------

ARMAddressAnalysis::ARMAddressAnalysis (Program * p, int sp):AddressAnalysis (p,sp)
{ 
}

bool
ARMAddressAnalysis::PerformAnalysis ()
{
  return AddressAnalysis::PerformAnalysis ();
}

// Checks if all required attributes are in the CFG
// Returns true if successful, false otherwise
bool
ARMAddressAnalysis::CheckInputAttributes ()
{
  return AddressAnalysis::CheckInputAttributes ();
}

//nothig to remove
void
ARMAddressAnalysis::RemovePrivateAttributes ()
{
  AddressAnalysis::RemovePrivateAttributes ();
}


