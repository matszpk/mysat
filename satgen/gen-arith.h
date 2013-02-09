/*
 * gen-arith.h - generate arithmetic operation to CNF
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_GEN_ARITH_H__
#define __SATGEN_GEN_ARITH_H__

#include <glibmm.h>
#include <satutils.h>

using namespace SatUtils;

enum DefType
{
  /* definition with right implication: result => function */
  DEF_POSITIVE = 0,
  /* definition with left implication: result <= function,
     use -result for generate implication: result => not function */
  DEF_NEGATIVE,
  /* definition with full implication: result <=> function,
     use -result for generate implication: result <=> not function */
  DEF_FULL,
};

void cnf_gen_equal (CNF& cnf, gint32 var, const LiteralVector& a,
      guint64 b, DefType def = DEF_FULL);

void cnf_gen_equal (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, DefType def = DEF_FULL);

void cnf_gen_and (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b);

void cnf_gen_and (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b);

void cnf_gen_and (CNF& cnf, guint64 out, const LiteralVector& a,
    const LiteralVector& b);

void cnf_gen_and (CNF& cnf, guint64 out, const LiteralVector& a, guint64 b);

void cnf_gen_or (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b);

void cnf_gen_or (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b);

void cnf_gen_or (CNF& cnf, guint64 out, const LiteralVector& a,
    const LiteralVector& b);

void cnf_gen_or (CNF& cnf, guint64 out, const LiteralVector& a, guint64 b);

void cnf_gen_xor (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b);

void cnf_gen_xor (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b);

void cnf_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    const LiteralVector& a, const LiteralVector& b);

void cnf_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    const LiteralVector& a, guint64 b);

void cnf_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    guint64 a, guint64 b);

void cnf_gen_ite (CNF& cnf, guint64 out, gint32 cond, const LiteralVector& a,
    const LiteralVector& b);

void cnf_gen_ite (CNF& cnf, guint64 out, gint32 cond,
    const LiteralVector& a, guint64 b);

/* mappings */
void cnf_gen_map(CNF& cnf, const LiteralVector& out, guint map_size, const guint64* map,
    const LiteralVector& index);

/* special arguments:
   out_carries - output carries for all bits,
   out_carries_one - values of output carries if not variables,
   carry0 - input carry variable,
   carry0_one - input carry value,
   negate_sign - negate last literal in inputs (useful for compare signed values) */
void cnf_gen_carry_int (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b,
      LiteralVector& out_carries, guint64& out_carries_one,
      gint32 carry0 = 0, bool carry0_one = 0, DefType def = DEF_FULL,
      bool negate_sign = false);

/* special arguments:
   carry0 - input carry variable,
   carry0_one - input carry value,
   negate_sign - negate last literal in inputs (useful for compare signed values) */
void cnf_gen_carry (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b,
      gint32 carry0 = 0, bool carry0_one = 0, DefType def = DEF_FULL,
      bool negate_sign = false);

/* special arguments:
   carry0 - input carry variable,
   carry0_one - input carry value,
   equivss - bitwise A<=>B result variables vector (if zero do create own),
   negate_sign - negate last literal in inputs (useful for compare signed values) */
void cnf_gen_carry_int (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, LiteralVector& out_carries, const LiteralVector& equivs,
      gint32 carry0 = 0, bool carry0_one = 0, DefType def = DEF_FULL,
      bool negate_sign = false);

/* special arguments:
   carry0 - input carry variable,
   carry0_one - input carry value,
   negate_sign - negate last literal in inputs (useful for compare signed values) */
void cnf_gen_carry (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, gint32 carry0 = 0, bool carry0_one = 0,
      DefType def = DEF_FULL, bool negate_sign = false);

void cnf_gen_less (CNF& cnf, gint32 var, const LiteralVector& a,
      guint64 b, DefType def = DEF_FULL, bool negate_sign = false);

void cnf_gen_less (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, DefType def = DEF_FULL, bool negate_sign = false);

void cnf_gen_lesseq (CNF& cnf, gint32 var, const LiteralVector& a,
      guint64 b, DefType def = DEF_FULL, bool negate_sign = false);

void cnf_gen_lesseq (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, DefType def = DEF_FULL, bool negate_sign = false);

void cnf_gen_add (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    guint64 b, gint32 carry = 0, bool carry_value = false);

void cnf_gen_add (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b, gint32 carry = 0, bool carry_value = false);

void cnf_gen_sub (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    guint64 b, gint32 carry = 0, bool carry_value = true);

void cnf_gen_sub (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b, gint32 carry = 0, bool carry_value = true);

void cnf_gen_sub (CNF& cnf, const LiteralVector& out, guint64 a,
    const LiteralVector& b, gint32 carry = 0, bool carry_value = true);

typedef std::vector<LiteralVector> LiteralMatrix;
typedef LiteralMatrix::iterator LiteralMatrixIter;
typedef LiteralMatrix::const_iterator LiteralMatrixConstIter;

void add_to_dadda_matrix (LiteralMatrix& matrix, const LiteralVector& a);

void cnf_gen_dadda_last_phase (CNF& cnf, const LiteralVector& out,
    LiteralMatrix& curmatrix);

/* generate Dadda multi-adder:
 * matrix form:
 * result bits: 0       1         2         ...  n
 *      matrix: m[0][0]  m[1][0]  m[0][2]        m[0][n-1]
 *              ..................................
 *              m[p][n] .........................m[p][n-1]
 *
 * with_last_phase - if true adds two columns from last phase */
 /* cnf_get_uint_dadda_with_carry:
  * special arguments:
  * with_carry - define carry of sum,
  * carryvar - variable for carry (1 - if carry, 0 - no carry),
  * carrydef - carry definition type */
void cnf_gen_dadda_with_carry (CNF& cnf, const LiteralVector& out,
    const LiteralMatrix& matrix, bool with_carry = false, gint32 carryvar = 0,
    DefType carrydef = DEF_POSITIVE);

void cnf_gen_dadda (CNF& cnf, const LiteralVector& out,
    const LiteralMatrix& matrix);


/* generate multiplication dadda matrix */
void cnf_gen_uint_mul_matrix(CNF& cnf, LiteralMatrix& matrix,
    const LiteralVector& a, guint64 b);

/* generate multiplication:
   a and b vector may be with various size
   (a.size () != b.size () is permitted) */

void cnf_gen_uint_mul_with_carry (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b, bool with_carry = false,
    gint32 carryvar = 0, DefType carrydef = DEF_POSITIVE);

void cnf_gen_uint_mul_with_carry (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b,
    bool with_carry = false, gint32 carryvar = 0,
    DefType carrydef = DEF_POSITIVE);

void cnf_gen_uint_mul (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b);

void cnf_gen_uint_mul (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b);

/* shifts */

void cnf_gen_shl_int (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    guint64 aval, const LiteralVector& shift, bool lower = false);

/* special argument: lower - if true is use a[0] for a[-x], otherwise use 0,
   example: (a[n]...a[0]) << 5 -> out=(a[n-5]...a[0],a[0],a[0],a[0],a[0],a[0]) */
void cnf_gen_shl (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& shift, bool lower = false);

void cnf_gen_shl (CNF& cnf, const LiteralVector& out,
    guint64 a, const LiteralVector& shift, bool lower = false);

/* special argument: sign - if true is use a[n] for a[n+x], otherwise use 0,
   example: (a[n]...a[0]) >> 5 -> out=(a[n],a[n],a[n],a[n],a[n]...a[5]) */
void cnf_gen_shr (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& shift, bool sign = false);

/* abits_n: number of bits of A value */
void cnf_gen_shr (CNF& cnf, const LiteralVector& out,
    guint64 a, guint abits_n, const LiteralVector& shift, bool sign = false);

/* divsions and modulo */

void cnf_gen_uint_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b);

void cnf_gen_uint_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, guint64 b);

void cnf_gen_int_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b);

void cnf_gen_int_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, gint64 b);

#endif /* __SATGEN_GEN_ARITH_H__ */
