
#include "DAAInstruction.h"
#include "Utl.h"

///-------------------------------
/// Generic functions of 
//  abstract class DAAInstruction
///-------------------------------
// getOperands
// -----------
// Operands are in the order they appear in the asm text file (destination first)
// The instruction mnemonic is not part of the returned vector,
// that contains operands only
vector < string > DAAInstruction::getOperands(const string & instructionAsm)
{
  vector < string > instr = Arch::splitInstruction(instructionAsm);
  vector < string > result(instr.begin() + 1, instr.end());	//+1 to not put the mnemonic
  return result;
}

DAAInstruction::~DAAInstruction()
{
}


string DAAInstruction::eval(string operand1, string codop, string operand2)
{
  long val;
  if (! Utl::eval(operand1, codop, operand2, &val)) return (operand1 + codop + operand2);
  return Utl::int2cstring(val);
}

void DAAInstruction::localop(regTable & regs, regPrecisionTable & precision, string vop)
{
  string operand1 = regs[num_register1];
  string operand2 = regs[num_register2];

  if (operand1 == "*" || operand2 == "*")
    {
      regs[num_register0] = "*";
      assert(!precision[num_register1] || !precision[num_register2]);
    }
  else
    {
      regs[num_register0] = eval(operand1, vop, operand2);
      bool b = precision[num_register1] && precision[num_register2];
      // if (!b) cout << " assertion not satisfied " << endl;
      // assert(b);
    }

  precision[num_register0] = precision[num_register1] && precision[num_register2];
}

// Consider that all information on the first operand (num_register0) is lost
void DAAInstruction::killop1(regTable & regs, regPrecisionTable & precision)
{
  regs[num_register0] = "*";
  precision[num_register0] = false;
}

// Consider that all information on the second operand is lost
void DAAInstruction::killop2(regTable & regs, regPrecisionTable & precision)
{
  regs[num_register1] = "*";
  precision[num_register1] = false;
}

/**
   The result (ie regs[num_register0]) is set to:
   - "unknown" if the operands are both unknown.
   - "+" if the operands are both known.
   - "unknown" otherwise when bAugmentPrecision is false.
   - the value of the known operand when bAugmentPrecision is true.
   (Damien 's note: this particular situation is to augment the precision of the analysis when accessing arrays (to detect
   that the array is accessed, even if the precise address in the array is not known)
*/
void DAAInstruction::add(regTable & regs, regPrecisionTable & precision, bool bAugmentPrecision)
{

  if (bAugmentPrecision)
    {
      string operand1 = regs[num_register1];
      string operand2 = regs[num_register2];
      if (operand1 == "*" && operand2 == "*")
	{
	  regs[num_register0] = "*";
	  assert(!precision[num_register1]);
	  assert(!precision[num_register2]);
	}
      else if (operand1 == "*")
	{
	  regs[num_register0] = operand2;	// for induction variable
	  assert(!precision[num_register1]);
	}
      else if (operand2 == "*")
	{
	  regs[num_register0] = operand1;	// for induction variable
	  assert(!precision[num_register2]);
	}
      else
	{
	  regs[num_register0] = eval(operand1, " + ", operand2);
	  assert(precision[num_register1]);
	  assert(precision[num_register2]);
	}

      precision[num_register0] = precision[num_register1] && precision[num_register2];
    }
  else
    localop(regs, precision, " + ");
}

void DAAInstruction::minus(regTable & regs, regPrecisionTable & precision)
{
  localop(regs, precision, "- ");
}

void DAAInstruction::mult(regTable & regs, regPrecisionTable & precision)
{
  localop(regs, precision, " x ");
}

// Keep all information we add on source register on destination register    
void DAAInstruction::move(regTable & regs, regPrecisionTable & precision)
{
  regs[num_register0] = regs[num_register1];
  precision[num_register0] = precision[num_register1];
}

// Loading a constant in a register.
void DAAInstruction::loadConstant(regTable & regs, regPrecisionTable & precision, string vcste)
{
  regs[num_register0] = vcste;
  precision[num_register0] = true;
}

void DAAInstruction::arithmetic_shift_right(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::logical_shift_left(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::logical_shift_right(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::rotate_right(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_and(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_or(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_eor(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

void DAAInstruction::op_bic(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}

/** It provides the value of the contents of a register shifted right one bit. 
    The old carry flag is shifted into bit[31]. 
    If the S suffix is present, the old bit[0] is placed in the carry flag.
*/
void DAAInstruction::rotate_right_extended(regTable & regs, regPrecisionTable & precision)
{
  killop1(regs, precision);	// NYI
}
