/*
 * spear-parse-check.cc - spear format files parsing test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <fstream>
#include "spear-format.h"

struct ExpectedSpear
{
  SpearVarMap varids;
  std::vector<guint> var_sizes;
  SpearConstraintVector constraints;
  SpearPredicateVector predicates;
};

static const std::string spear_operator_names[SPEAROP_LAST+1] =
{
  "&", // SPEAROP_AND
  "|", // SPEAROP_OR
  "^", // SPEAROP_XOR
  "~", // SPEAROP_NOT
  "=>", // SPEARPRED_IMPL
  "=", // SPEARPRED_EQ
  "/=", // SPEARPRED_NE
  "ule", // SPEARPRED_ULE
  "uge", // SPEARPRED_UGE
  "ult", // SPEARPRED_ULT
  "ugt", // SPEARPRED_UGT
  "sle", // SPEARPRED_SLE
  "sge", // SPEARPRED_SGE
  "slt", // SPEARPRED_SLT
  "sgt", // SPEARPRED_SGT
  "ite", // SPEAROP_ITE
  "+", // SPEAROP_ADD
  "-", // SPEAROP_SUB
  "*", // SPEAROP_MUL
  "/u", // SPEAROP_UDIV
  "/s", // SPEAROP_SDIV
  "%u", // SPEAROP_UMOD
  "%s", // SPEAROP_SMOD
  "<<", // SPEAROP_SL
  ">>a", // SPEAROP_ASR
  ">>l", // SPEAROP_LSR
  "trun", // SPEAROP_TRUN
  "sext", // SPEAROP_SEXT
  "zext", // SPEAROP_ZEXT
  "conc", // SPEAROP_CONC
  "extr", // SPEAROP_EXTR
};

static bool
get_operand (std::istream& is, const ExpectedSpear& spear, SpearOperand& arg)
{
  char c;
  is >> c;
  if (c == 'c')
  {
    is >> arg.value >> arg.bits;
    arg.constant = true;
  }
  else if (c == 'v')
  {
    std::string name;
    is >> name;
    SpearVarMap::const_iterator found = spear.varids.find (name);
    if (found == spear.varids.end ())
      return false;
    arg.varid = found->second;
    arg.bits = spear.var_sizes[arg.varid];
    arg.constant = false;
  }
  else
    arg.bits = 0;
  return true;
}

static bool
get_expected_from_file (std::istream& is, ExpectedSpear& spear)
{
  /* definition */
  for (guint i = 0;; i++)
  {
    guint bits;
    std::string name;
    is >> name;
    if (name != ":") /* is not end of defs */
    {
      is >> bits;
      if (is.fail ())
	return false;
      spear.varids[name] = i;
      spear.var_sizes.push_back (bits);
    }
    else
      break;
  }
  /* constraints */
  while (1)
  {
    char c;
    std::string opname;
    is >> c;
    if (is.eof ())
      break;
    if (c == 'c')
    { /* constraint */
      SpearConstraint constraint;
      if (!get_operand (is, spear, constraint.result))
        return false;
      is >> opname;
      const std::string* found = std::find (spear_operator_names,
          spear_operator_names + SPEAROP_LAST+1, opname);
      if (found == spear_operator_names + SPEAROP_LAST+1)
        return false;
      constraint.op = SpearOp (found - spear_operator_names);
      
      if (!get_operand (is, spear, constraint.args[0]))
        return false;
      if (!get_operand (is, spear, constraint.args[1]))
        return false;
      if (!get_operand (is, spear, constraint.args[2]))
        return false;
      if (is.fail ())
        return false;
      spear.constraints.push_back (constraint);
    }
    else if (c == 'p')
    {
      SpearPredicate predicate;
      
      is >> opname;
      const std::string* found = std::find (spear_operator_names,
          spear_operator_names + SPEAROP_LAST+1, opname);
      if (found == spear_operator_names + SPEAROP_LAST+1)
        return false;
      predicate.pred = SpearOp (found - spear_operator_names);
      
      if (!get_operand (is, spear, predicate.args[0]))
        return false;
      if (!get_operand (is, spear, predicate.args[1]))
        return false;
      if (!get_operand (is, spear, predicate.args[2]))
        return false;
      if (is.fail ())
        return false;
      spear.predicates.push_back (predicate);
    }
  }
  return true;
}

struct SpearOpEqual
{
  bool operator() (const SpearOperand& op1, const SpearOperand& op2)
  {
    return ((op1.bits == 0 && op2.bits == 0) ||
         (op1.constant == op2.constant && op1.bits == op2.bits &&
          ((op1.constant && op1.value == op2.value) ||
           (!op1.constant && op1.varid == op2.varid))));
  }
};

struct ConstraintEqual
{
  bool operator() (const SpearConstraint& c1, const SpearConstraint& c2)
  {
    SpearOpEqual opeq;
    return (opeq (c1.result, c2.result) && opeq (c1.args[0], c2.args[0]) &&
            opeq (c1.args[1], c2.args[1]) && opeq (c1.args[2], c2.args[2]) &&
            c1.op == c2.op);
  }
};

struct PredicateEqual
{
  bool operator() (const SpearPredicate& p1, const SpearPredicate& p2)
  {
    SpearOpEqual opeq;
    return (opeq (p1.args[0], p2.args[0]) && opeq (p1.args[1], p2.args[1]) &&
          opeq (p1.args[2], p2.args[2]) && p1.pred == p2.pred);
  }
};

static bool
spear_parse_check (const std::string& exfilename,
    const std::string& sffilename)
{
  ExpectedSpear expected;
  std::ifstream file;
  file.exceptions (std::ios_base::badbit);
  
  file.open (exfilename.c_str (), std::ios::in);
  if (!file)
    return false;
  if (!get_expected_from_file (file, expected))
  {
    std::cerr << "Cant parse input expected spear file" << std::endl;
    return false;
  }
  file.close ();
  
  SpearFormatModule* module = static_cast<SpearFormatModule*>(
      SpearFormatModule::create ());
  
  try
  {
    std::string input;
    file.open (sffilename.c_str (), std::ios::in);
    if (!file)
      return false;
    while (1)
    {
      char c = file.get ();
      if (file.eof ())
	break;
      input.push_back (c);
    }
    file.close ();
    
    module->parse_input (input);
    
    /* check if equal */
    bool is_equal = true;
    
    if (expected.varids != module->get_varids ())
    {
      std::cerr << "Bad variables order or names" << std::endl;
      is_equal = false;
    }
    if (expected.var_sizes != module->get_var_sizes ())
    {
      std::cerr << "Bad variables sizes" << std::endl;
      is_equal = false;
    }
    
    if (expected.constraints.size () != module->get_constraints ().size () ||
        !std::equal (expected.constraints.begin (), expected.constraints.end (),
            module->get_constraints ().begin (), ConstraintEqual ()))
    {
      std::cerr << "Bad constraints" << std::endl;
      is_equal = false;
    }
    if (expected.predicates.size () != module->get_predicates ().size () ||
        !std::equal (expected.predicates.begin (), expected.predicates.end (),
            module->get_predicates ().begin (), PredicateEqual ()))
    {
      std::cerr << "Bad predicates" << std::endl;
      is_equal = false;
    }
    if (!is_equal)
    {
      delete module;
      std::cerr << "Parse returns bad results" << std::endl;
      return false;
    }
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    delete module;
    return false;
  }
  
  return true;
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 3)
  {
    std::cout << "spear-parse-check EXPECTED SPEARFILE" << std::endl;
    return 0;
  }
  if (!spear_parse_check (argv[1], argv[2]))
  {
    std::cerr << "FAILED" << std::endl;
    return 1;
  }
  else
    std::cout << "OK!" << std::endl;
  return 0;
}
