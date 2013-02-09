/*
 * spear-format.h - Spear modular arithmetic format converter
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_SPEAR_FORMAT_H__
#define __SATGEN_SPEAR_FORMAT_H__

#include <string>
#include <map>
#include <vector>
#include "module.h"

using namespace SatUtils;

typedef std::map<std::string, guint32> SpearVarMap;

struct SpearOperand
{
  guint bits;
  bool constant;
  union {
    guint32 varid;
    guint64 value;
  };
};

enum SpearOp
{
  SPEAROP_FIRST = 0,
  SPEAROP_AND = 0,
  SPEAROP_OR,
  SPEAROP_XOR,
  SPEAROP_NOT,
  SPEARPRED_IMPL, /* implication */
  SPEARPRED_EQ,
  SPEARPRED_NE,
  SPEARPRED_ULE, /* unsigned less or equal */
  SPEARPRED_UGE, /* unsigned greater or equal */
  SPEARPRED_ULT, /* unsigned less */
  SPEARPRED_UGT, /* unsigned greater */
  SPEARPRED_SLE, /* signed less or equal */
  SPEARPRED_SGE, /* signed greater or equal */
  SPEARPRED_SLT, /* signed less */
  SPEARPRED_SGT, /* signed greater */
  SPEAROP_ITE, /* if then else */
  SPEAROP_ADD,
  SPEAROP_SUB,
  SPEAROP_MUL,
  SPEAROP_UDIV, /* unsigned division */
  SPEAROP_SDIV, /* signed division */
  SPEAROP_UMOD, /* unsigned division */
  SPEAROP_SMOD, /* signed division */
  SPEAROP_SL,  /* shift left */
  SPEAROP_ASR, /* arith shift right */
  SPEAROP_LSR, /* arith shift right */
  SPEAROP_TRUN, /* truncate */
  SPEAROP_SEXT, /* sign extend */
  SPEAROP_ZEXT, /* zero extend */
  SPEAROP_CONC, /* concatenate */
  SPEAROP_EXTR, /* extract */
  SPEAROP_LAST = SPEAROP_EXTR,
  SPEARPRED_FIRST = SPEARPRED_IMPL,
  SPEARPRED_LAST = SPEARPRED_SGT,
};

struct SpearConstraint
{
  SpearOp op;
  SpearOperand result;
  SpearOperand args[3];
};

typedef std::vector<SpearConstraint> SpearConstraintVector;
typedef SpearConstraintVector::iterator SpearConstraintIter;
typedef SpearConstraintVector::const_iterator SpearConstraintConstIter;

struct SpearPredicate
{
  SpearOp pred;
  SpearOperand args[3];
};

typedef std::vector<SpearPredicate> SpearPredicateVector;
typedef SpearPredicateVector::iterator SpearPredicateIter;
typedef SpearPredicateVector::const_iterator SpearPredicateConstIter;

class SpearFormatModule: public SatGenModule
{
private:
  bool simplify;
  guint lineno;
  SpearVarMap varids;
  std::vector<guint> var_sizes;
  
  SpearPredicateVector predicates;
  SpearConstraintVector constraints;
  
  SpearFormatModule ();
  void post_process_params ();
  
  std::string get_normal_line (std::string::const_iterator& start,
    std::string::const_iterator end);
  guint get_integer_type (std::string::const_iterator& start,
    std::string::const_iterator end);
  std::string get_var_name (std::string::const_iterator& start,
    std::string::const_iterator end);
  void get_operand (std::string::const_iterator& start,
    std::string::const_iterator end, SpearOperand& operand);
public:
  ~SpearFormatModule ();
  
  static SatGenModule* create ();
  
  void parse_input (const std::string& input);
  void generate (CNF& cnf, std::string& outmap_string, bool with_outmap) const;
  
  const SpearVarMap& get_varids () const
  { return varids; }
  const std::vector<guint>& get_var_sizes () const
  { return var_sizes; }
  const SpearPredicateVector& get_predicates () const
  { return predicates; }
  const SpearConstraintVector& get_constraints () const
  { return constraints; }
};

#endif /* __SATGEN_SPEAR_FORMAT_H__ */
