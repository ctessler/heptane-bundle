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

#include "Utl.h"
#include <sstream>
#include <math.h>


#define isHexaCode(c) ((c >='0' && c <='9') || ( c >= 'a' && c <='f'))
//singleton declaration
Utl * Utl;

Utl::Utl ()
{}

Utl::~Utl ()
{}


void Utl::rmSpaces(string &str)
{
  //  std::string whitespaces (" \t\f\v\n\r");
  std::string whitespaces (" ");

  std::size_t i0 = str.find_first_not_of(whitespaces);
  if (i0 != 0) str.erase(0, i0);
  std::size_t found = str.find_last_not_of(whitespaces);
  if (found!= EOS ) 
    str.erase(found+1); 
  else 
    str.clear(); // str is all whitespace
}


bool Utl::isHexNumber(const string& str)
{
  if(str.length()<=2) { return false; }
  if(str[0] != '0' || str[1] != 'x' ) return false;
    
  for (size_t i=2;i<str.length();i++)
    { 
      if( ! isHexaCode(str[i]) ) return false;
    }
  return true;
}

// from arch.cc
bool Utl::isAddr(const string& str)
{
  assert(str.length()>0);
  for (size_t i=0;i<str.length();i++)
    {
      if( ! isHexaCode(str[i]) ) return false;
    }
  return true;
}

bool Utl::isDecNumber(const string& str)
{
  size_t lg = str.length();
  assert(lg>0);
  for (size_t i=0;i< lg;i++)
    {
      if(str[i]<'0' || str[i]>'9')
        {
	  if(i==0 && str[i]=='-'){;}
	  else return false;
        }
    }
  return true;
}

int Utl::string2int(string vpattern)
{
  if (vpattern.length() == 0) return 0;
  assert (isDecNumber (vpattern));
  return atoi (vpattern.c_str ());
}

long Utl::string2long(string vpattern)
{
  if (vpattern.length() == 0) return (long)0;
  bool b = isDecNumber (vpattern);
  assert(b);
  return atol (vpattern.c_str ());
}
	  
int Utl::imax(int i, int j)
{
  if (i>j) return i;
  return j;
}

bool Utl::replace(string &line, char c1, char c2)
{
  size_t pos= line.find(c1);
  if (pos == EOS) return false;
  line[pos]=c2;
  return true;
}

bool Utl::replaceAll(string &line, char c1, char c2)
{
  bool b = false;
  while(replace(line, c1,c2)) b=true;
  return b;
}

int Utl::count(string &line, char sep)
{
  if (line.length () == 0) return 0;
  int n = 0;
  for (size_t i = 0; i < line.length (); i++) 
    if (line[i] == sep) n++;
  return n;
}


string Utl::complement(string v)
{
  std::stringstream ss; 
  ss << ~ string2int(v);
  return ss.str();
}


string Utl::int2cstring(long v)
{
  ostringstream ossOffset;
  ossOffset << v;
  return ossOffset.str();
}

bool Utl::eval(string &operand1, string &codop, string &operand2, long *v)
{
  long v1, v2;

  // cout << endl << "eval (" << operand1 << "," << ocodeop <<"," <<  operand2 << ")" << endl;
  if (! isDecNumber(operand1)) return false;
  if (! isDecNumber(operand2)) return false;
  v1 = atol (operand1.c_str ());
  v2 = atol (operand2.c_str ());
  if (codop.find("+") != EOS) *v = v1 + v2;
  else if (codop.find("-") != EOS) *v = v1 - v2;
  else if (codop.find("*") != EOS) *v = v1 * v2;
  else return false;
  return true;
}

bool Utl::evalexpr(string &exp, long *val)
{
  bool foundPlus, foundMinus, foundMult;
  size_t index;
  string op1, op2, codeop;
  
  // looking for ONE +|-|* in exp. otherwise return false.
  index= exp.find ("+");
  foundPlus= (index != EOS );

  index= exp.find ("-");
  foundMinus= (index != EOS );
  if (foundPlus && foundMinus) return false;

  index= exp.find ("-");
  foundMult= (index != EOS );
  if (foundPlus && foundMult) return false;
  if (! foundPlus && !foundMult && !foundMinus) return false;

  op1 = exp.substr(0, index);
  rmSpaces(op1);
  op2 = exp.substr(index+1);
  rmSpaces(op2);
  codeop= (foundPlus ? "+" : (foundMinus ? "-": "*"));
  return eval(op1, codeop, op2, val);
}


string Utl::extractStringValue(string &arg)
{
  size_t pos;
  string res;

  pos=arg.find("\"");
  if (pos == EOS ) return string("");
  res=arg.substr (pos+1);
  pos=res.find("\"");
  if (pos == EOS ) return string("");
  res=res.substr (0, pos);
  return res;
}

