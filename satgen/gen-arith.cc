/*
 * gen-arith.cc - generate arithmetic operation to CNF
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <set>
#include "gen-arith.h"

using namespace SatUtils;

void
cnf_gen_equal (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b, DefType def)
{
  if (def == DEF_POSITIVE || def == DEF_FULL)
  {
    for (guint i = 0; i < a.size (); i++)
        cnf.add_clause (-var, (b & (1ULL<<i)) ? a[i] : -a[i]);
  }
  if (def == DEF_NEGATIVE || def == DEF_FULL)
  {
    LiteralVector clause;
    clause.push_back (var);
    for (guint i = 0; i < a.size (); i++)
      clause.push_back ((b & (1ULL<<i)) ? -a[i] : a[i]);
    cnf.add_clause (clause);
  }
}

void
cnf_gen_equal (CNF& cnf, gint32 var, const LiteralVector& a, const LiteralVector& b,
      DefType def)
{
  assert (a.size () == b.size ());

  if (def == DEF_POSITIVE || def == DEF_FULL)
  {
    for (guint i = 0; i < a.size (); i++)
    {
      cnf.add_clause (-var, a[i], -b[i]);
      cnf.add_clause (-var, -a[i], b[i]);
    }
  }
  if (def == DEF_NEGATIVE || def == DEF_FULL)
  {
    /* startvars is result of a[i] xor b[i] */
    gint32 startvar = cnf.next_var ();
    cnf.add_vars (a.size ());
    {
      LiteralVector clause;
      clause.push_back (var);
      add_literals_from_range (clause, startvar, startvar + a.size ());
      cnf.add_clause (clause);
    }
    /* adding clause of results */
    for (guint i = 0; i < a.size (); i++)
    {
      cnf.add_clause (-startvar-i, a[i], b[i]);
      cnf.add_clause (-startvar-i, -a[i], -b[i]);
    }
  }
}

void
cnf_gen_and (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
  {
    cnf.add_clause (-out[i], a[i]);
    cnf.add_clause (-out[i], b[i]);
    cnf.add_clause (out[i], -a[i], -b[i]);
  }
}

void
cnf_gen_and (CNF& cnf, const LiteralVector& out, const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (b & (1ULL<<i))
    {
      cnf.add_clause (-out[i], a[i]);
      cnf.add_clause (out[i], -a[i]);
    }
    else
      cnf.add_clause (-out[i]);
}


void
cnf_gen_and (CNF& cnf, guint64 out, const LiteralVector& a, const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
    if (out & (1ULL<<i))
    {
      cnf.add_clause (a[i]);
      cnf.add_clause (b[i]);
    }
    else
      cnf.add_clause (-a[i], -b[i]);
}

void
cnf_gen_and (CNF& cnf, guint64 out, const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (out & (1ULL<<i))
    {
      if (b & (1ULL<<i)) /* 1 = a^1, a=1 */
        cnf.add_clause (a[i]);
      else /* 1 = a^0 (unsatisfiable) */
        cnf.add_clause (0);
    }
    else
    {
      if (b & (1ULL<<i)) /* 0 = a^1, a=0 */
        cnf.add_clause (-a[i]);
    }
}

void
cnf_gen_or (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
  {
    cnf.add_clause (-out[i], a[i], b[i]);
    cnf.add_clause (out[i], -a[i]);
    cnf.add_clause (out[i], -b[i]);
  }
}

void
cnf_gen_or (CNF& cnf, const LiteralVector& out, const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (b & (1ULL<<i))
      cnf.add_clause (out[i]);
    else
    {
      cnf.add_clause (-out[i], a[i]);
      cnf.add_clause (out[i], -a[i]);
    }
}

void
cnf_gen_or (CNF& cnf, guint64 out, const LiteralVector& a,
    const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
    if (out & (1ULL<<i))
      cnf.add_clause (a[i], b[i]);
    else
    {
      cnf.add_clause (-a[i]);
      cnf.add_clause (-b[i]);
    }
}

void
cnf_gen_or (CNF& cnf, guint64 out, const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (out & (1ULL<<i))
    {
      if ((b & (1ULL<<i)) == 0) /* 1 = a v 0, a=1 */
        cnf.add_clause (a[i]);
    }
    else
    {
      if (b & (1ULL<<i)) /* 0 = a v 1, unsatisfiable */
        cnf.add_clause (0);
      else /* 0 = a v 0, a=0 */
        cnf.add_clause (-a[i]);
    }
}


void
cnf_gen_xor (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
  {
    cnf.add_clause (-out[i], a[i], b[i]);
    cnf.add_clause (-out[i], -a[i], -b[i]);
    cnf.add_clause (out[i], a[i], -b[i]);
    cnf.add_clause (out[i], -a[i], b[i]);
  }
}

void
cnf_gen_xor (CNF& cnf, const LiteralVector& out, const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (b & (1ULL<<i))
    {
      cnf.add_clause (-out[i], -a[i]);
      cnf.add_clause (out[i], a[i]);
    }
    else
    {
      cnf.add_clause (-out[i], a[i]);
      cnf.add_clause (out[i], -a[i]);
    }
}

void
cnf_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    const LiteralVector& a, const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
  { /* out[i] = (-cond^b[i]) v (cond^a[i]) */
    cnf.add_clause (-out[i], -cond, a[i]);
    cnf.add_clause (-out[i], cond, b[i]);
    cnf.add_clause (out[i], -cond, -a[i]);
    cnf.add_clause (out[i], cond, -b[i]);
  }
}

void
cnf_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (b & (1ULL<<i))
    { /* out[i] = -cond v (cond^a[i]) -> out[i] = -cond v a[i] */
      cnf.add_clause (-out[i], -cond, a[i]);
      cnf.add_clause (out[i], cond);
      cnf.add_clause (out[i], -a[i]);
    }
    else
    { /* out[i] = (cond^a[i]) */
      cnf.add_clause (-out[i], cond);
      cnf.add_clause (-out[i], a[i]);
      cnf.add_clause (out[i], -cond, -a[i]);
    }
}

void
cnf_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    guint64 a, guint64 b)
{
  for (guint i = 0; i < out.size (); i++)
    if (a & (1ULL<<i))
    {
      if (b & (1ULL<<i))
        cnf.add_clause (out[i]);
      else
      {
        cnf.add_clause (-out[i], cond);
        cnf.add_clause (out[i], -cond);
      }
    }
    else
    {
      if (b & (1ULL<<i))
      {
        cnf.add_clause (-out[i], -cond);
        cnf.add_clause (out[i], cond);
      }
      else
        cnf.add_clause (-out[i]);
    }
}

void
cnf_gen_ite (CNF& cnf, guint64 out, gint32 cond, const LiteralVector& a,
    const LiteralVector& b)
{
  assert (a.size () == b.size ());

  for (guint i = 0; i < a.size (); i++)
    if (out & (1ULL<<i))
    { /* 1 = (-cond^b[i]) v (cond^a[i]) */
      cnf.add_clause (-cond, a[i]);
      cnf.add_clause (cond, b[i]);
    }
    else
    { /* 0 = (-cond^b[i]) v (cond^a[i]) */
      cnf.add_clause (-cond, -a[i]);
      cnf.add_clause (cond, -b[i]);
    }
}

void
cnf_gen_ite (CNF& cnf, guint64 out, gint32 cond, const LiteralVector& a, guint64 b)
{
  for (guint i = 0; i < a.size (); i++)
    if (out & (1ULL<<i))
    {
      if (b & (1ULL<<i))
        /* 1 = -cond v (cond^a[i]) -> out[i] = -cond v a[i] */
	cnf.add_clause (-cond, a[i]);
      else
      { /* 1 = (cond^a[i]) */
	cnf.add_clause (cond);
	cnf.add_clause (a[i]);
      }
    }
    else
    {
      if (b & (1ULL<<i))
      { /* 0 = -cond v (cond^a[i]) -> out[i] = -cond v a[i] */
	cnf.add_clause (cond);
	cnf.add_clause (-a[i]);
      }
      else
      /* 0 = (cond^a[i]) */
	cnf.add_clause (-cond, -a[i]);
    }
}

void cnf_gen_map(CNF& cnf, const LiteralVector& out, guint map_size, const guint64* map,
    const LiteralVector& index)
{
  guint out_size = out.size();

  for (guint i = 0; i < out_size; i++)
  {
    guint ones = 0;
    for (guint j = 0; j < map_size; j++)
      if ((map[j] & (1ULL<<i)) != 0)
	ones++;

    if (ones < map_size/2)
    {
      LiteralVector clause;
      clause.push_back(-out[i]);

      for (guint j = 0; j < map_size; j++)
        if ((map[j] & (1ULL<<i)) != 0)
        {
          gint32 index_equal = cnf.add_var();
          cnf_gen_equal(cnf, index_equal, index, j, DEF_FULL);
          clause.push_back(index_equal);

          cnf.add_clause(out[i], -index_equal);
        }

      cnf.add_clause(clause);
    }
    else
    {
      LiteralVector clause;
      clause.push_back(out[i]);

      for (guint j = 0; j < map_size; j++)
        if ((map[j] & (1ULL<<i)) == 0)
        {
          gint32 index_equal = cnf.add_var();
          cnf_gen_equal(cnf, index_equal, index, j, DEF_FULL);
          clause.push_back(index_equal);

          cnf.add_clause(-out[i], -index_equal);
        }

      cnf.add_clause(clause);
    }
  }

  if (map_size < (1ULL<<index.size()))
    cnf_gen_lesseq(cnf, 0, index, map_size-1, DEF_POSITIVE);
}

/* carries generator */

void
cnf_gen_carry_int (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b,
      LiteralVector& out_carries, guint64& out_carries_one,
      gint32 carry0, bool carry0_one, DefType def, bool negate_sign)
{
  bool positive = (def == DEF_POSITIVE || def == DEF_FULL);
  bool negative = (def == DEF_NEGATIVE || def == DEF_FULL);

  gint32 prev_carry = carry0;
  gint32 carry;

  bool is_prev_carry_one = carry0_one;
  bool is_carry_one = false;
  bool result_is_constant = false;

  out_carries_one = 0;

  for (guint i = 0; i < a.size (); i++)
  {
    /* a and b single literal */
    bool blit;
    gint32 alit;
    if (i == a.size ()-1 && negate_sign)
    { /* negate sign and last literal */
      alit = -a[i];
      blit = ((b & (1ULL<<i))) ? false : true;
    }
    else
    {
      alit = a[i];
      blit = ((b & (1ULL<<i))) ? true : false;
    }

    if (prev_carry != 0)
    {
      if (i == a.size ()-1) /* if last: use output as var */
	carry = var;
      else
	carry = cnf.add_var ();

      /* use formuale:
       c[i+1] = (c[i]v(a[i]==b[i]))^(a[i]vb[i]) */
      //if (b & (1ULL<<i)) /* b[i] = 0 */
      if (blit)
      { /* c[i+1] = (c[i]v(a[i]==1))^(a[i]v1) <-> c[i+1] = c[i]va[i] */
	if (positive)
	  cnf.add_clause (-carry, prev_carry, alit);
	if (negative)
	{ /* c[i+1] != (-c[i]v(a[i]==1))^(-a[i]v0) <->
	     c[i+1] != (-c[i]va[i])^-a[i] <-> c[i+1] != -c[i]^-a[i]  */
	  cnf.add_clause (carry, -prev_carry);
	  cnf.add_clause (carry, -alit);
	}
      }
      else /* if b[i]=0 */
      { /* c[i+1] = (c[i]v(a[i]==0))^(a[i]v0) <->
           c[i+1] = (c[i]v-a[i])^a[i] <-> c[i+1] = c[i]^a[i] */
        if (positive)
        {
          cnf.add_clause (-carry, prev_carry);
          cnf.add_clause (-carry, alit);
        }
        if (negative)
          /* c[i+1] != (-c[i]v(a[i]==0))^(-a[i]v1) <-> c[i+1] != (-c[i]v-a[i]) */
          cnf.add_clause (carry, -prev_carry, -alit);
      }
    }
    else if (is_prev_carry_one)
    { /* if c[i] = 1 */
      if (blit)
      { /* b[i] = 1, c[i+1] = 1 */
        if (i == a.size ()-1)
          result_is_constant = true;
        carry = 0;
        is_carry_one = true;
      }
      else /* b[i] = 0 */
      { /* c[i+1] = a[i] */
        if (i == a.size ()-1) /* if last: use output as var */
        { /* last carry is var: */
	  carry = var;
	  /* define var <=> a[i] */
	  if (positive)
	    cnf.add_clause (-carry, alit);
	  if (negative)
	    cnf.add_clause (carry, -alit);
	}
	else /* only copy to carry (do not add clauses) */
	  carry = alit;
      }
    }
    else /* prev carry is zero */
    { /* c[i] = 0 */
      if (blit)
      { /* b[i] = 1 */
        if (i == a.size ()-1)
        { /* last carry is var */
          carry = var;
          /* define var <=> a[i] */
	  if (positive)
	    cnf.add_clause (-carry, alit);
	  if (negative)
	    cnf.add_clause (carry, -alit);
        }
        else /* only copy to carry (do not add clauses) */
          carry = alit;
      }
      else /* b[i] == 0 */
      { /* c[i+1] = 0 */
        /* if var is zero then formulae is false (empty clause),
           otherwise var must be zero */
        if (i == a.size ()-1)
          //cnf.add_clause (-var);
          result_is_constant = true;
        carry = 0;
        is_carry_one = false;
      }
    }

    out_carries.push_back (carry);
    if (carry == 0)
      out_carries_one |= (is_carry_one) ? (1ULL << i) : 0;

    prev_carry = carry;
    is_prev_carry_one = is_carry_one;
  }

  /* remove is always true */
  if (result_is_constant)
  {
    if (is_carry_one)
    {
      if (positive && var != 0) /* var must be true or skip this clause */
        cnf.add_clause (var);
      if (negative && !positive) /* var must be true or empty clause */
        cnf.add_clause (var);
    }
    else
    {
      if (positive) /* var must be false or empty clause */
        cnf.add_clause (-var);
      if (negative && !positive && var != 0)
        /* var must be false or skip this clause */
        cnf.add_clause (-var);
    }
  }
}

void
cnf_gen_carry (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b,
      gint32 carry0, bool carry0_one, DefType def, bool negate_sign)
{
  guint64 temp_carries_vals;

  LiteralVector temp_carries;
  cnf_gen_carry_int (cnf, var, a, b, temp_carries, temp_carries_vals,
      carry0, carry0_one, def, negate_sign);
}

void
cnf_gen_carry_int (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, LiteralVector& out_carries,
      const LiteralVector& equivs, gint32 carry0, bool carry0_one, DefType def,
      bool negate_sign)
{
  assert (a.size () == b.size ());

  bool positive = (def == DEF_POSITIVE || def == DEF_FULL);
  bool negative = (def == DEF_NEGATIVE || def == DEF_FULL);

  gint32 prev_carry = 0;
  gint32 carry;

  gint32 alit, blit;

  if (a.size () != 0)
  {
    if (a.size () == 1 && negate_sign)
    { /* negate sign */
      alit = -a[0];
      blit = -b[0];
    }
    else
    {
      alit = a[0];
      blit = b[0];
    }

    if (a.size () == 1)
      carry = var;
    else
      carry = cnf.add_var ();

    if (carry0 != 0) // if carry0 defined as variable
    { /* use formuale:
	c[i+1] = (c[i]v(a[i]==b[i]))^(a[i]vb[i]) */

      gint32 temp;
      if (equivs[0] == 0)
      {
	temp = cnf.add_var ();
	cnf.add_clause (-temp, alit, -blit);
	cnf.add_clause (-temp, -alit, blit);
      }
      else
	temp = equivs[0];
      if (positive)
      {
	cnf.add_clause (-carry, carry0, temp);
	cnf.add_clause (-carry, alit, blit);
      }
      if (negative)
      {
	cnf.add_clause (carry, -carry0, temp);
	cnf.add_clause (carry, -alit, -blit);
      }
    }
    else if (carry0_one)
    { /* c[i+1] = (1v(a[i]==b[i]))^(a[i]vb[i]) <-> c[i+1] = a[i]vb[i] */
      if (positive)
	cnf.add_clause (-carry, alit, blit);
      if (negative)
      {
	cnf.add_clause (carry, -alit);
	cnf.add_clause (carry, -blit);
      }
    }
    else /* if zero */
    { /* c[i+1] = -((0v(a[i]==b[i]))^(a[i]vb[i])) <->
	c[i+1] != (1v(a[i]==b[i]))^(-a[i]v-b[i]) <->
	c[i+1] = a[i]^b[i] */
      if (positive)
      {
	cnf.add_clause (-carry, alit);
	cnf.add_clause (-carry, blit);
      }
      if (negative)
	cnf.add_clause (carry, -alit, -blit);
    }
    prev_carry = carry;
    out_carries.push_back (carry);

    for (guint i = 1; i < a.size (); i++)
    {
      gint32 temp;

      if (i == a.size ()-1 && negate_sign)
      {
	alit = -a[i];
	blit = -b[i];
      }
      else
      {
	alit = a[i];
	blit = b[i];
      }

      if (i == a.size ()-1) /* if last: use output as var */
	carry = var;
      else
	carry = cnf.add_var ();
      /* use formuale:
	c[i+1] = (c[i]v(a[i]==b[i]))^(a[i]vb[i]) */
      if (equivs[i] == 0)
      {
	temp = cnf.add_var ();
	cnf.add_clause (-temp, alit, -blit);
	cnf.add_clause (-temp, -alit, blit);
      }
      else /* use from xors */
	temp = equivs[i];
      if (positive)
      {
	cnf.add_clause (-carry, prev_carry, temp);
	cnf.add_clause (-carry, alit, blit);
      }
      if (negative)
      {
	cnf.add_clause (carry, -prev_carry, temp);
	cnf.add_clause (carry, -alit, -blit);
      }
      prev_carry = carry;
      out_carries.push_back (carry);
    }
  }
}


void
cnf_gen_carry (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, gint32 carry0, bool carry0_one, DefType def,
      bool negate_sign)
{
  LiteralVector temp_carries;
  LiteralVector dummy_xors (a.size ());
  std::fill (dummy_xors.begin (), dummy_xors.end (), 0);
  cnf_gen_carry_int (cnf, var, a, b, temp_carries, dummy_xors,
      carry0, carry0_one, def, negate_sign);
}

void
cnf_gen_less (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b,
      DefType def, bool negate_sign)
{
  LiteralVector reva = a;
  for (LiteralIter it = reva.begin (); it != reva.end (); ++it)
    *it = -*it;
  cnf_gen_carry (cnf, var, reva, b, 0, false, def, negate_sign);
}

void
cnf_gen_less (CNF& cnf, gint32 var, const LiteralVector& a,
      const LiteralVector& b, DefType def, bool negate_sign)
{
  LiteralVector reva = a;
  for (LiteralIter it = reva.begin (); it != reva.end (); ++it)
    *it = -*it;
  cnf_gen_carry (cnf, var, reva, b, 0, false, def, negate_sign);
}

void
cnf_gen_lesseq (CNF& cnf, gint32 var, const LiteralVector& a, guint64 b,
      DefType def, bool negate_sign)
{
  LiteralVector reva = a;
  for (LiteralIter it = reva.begin (); it != reva.end (); ++it)
    *it = -*it;
  cnf_gen_carry (cnf, var, reva, b, 0, true, def, negate_sign);
}

void
cnf_gen_lesseq (CNF& cnf, gint32 var, const LiteralVector& a,
     const LiteralVector& b, DefType def, bool negate_sign)
{ /* (v<=(not a)<(not b)) <=> (v=>(a<b)) */
  LiteralVector reva = a;
  for (LiteralIter it = reva.begin (); it != reva.end (); ++it)
    *it = -*it;
  cnf_gen_carry (cnf, var, reva, b, 0, true, def, negate_sign);
}

/*
 * ADDER
 */

void
cnf_gen_add (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    guint64 b, gint32 carry0, bool carry0_value)
{
  assert (a.size () <= out.size ());
  LiteralVector carries;
  guint64 carries_vals;
  gint32 last_carry;

  /* add one carry if output is bigger than input */
  if (out.size () > a.size ())
  {
    last_carry = out[a.size ()];
    cnf_gen_carry_int (cnf, last_carry, a, b, carries, carries_vals,
          carry0, carry0_value, DEF_FULL);
  }
  else
  { /* if do not save last carry */
    if (a.size () > 1)
    {
      LiteralVector tmpa (a.begin (), --a.end ());
      last_carry = cnf.add_var ();
      cnf_gen_carry_int (cnf, last_carry, tmpa, b, carries, carries_vals,
	    carry0, carry0_value, DEF_FULL);
      carries.push_back (last_carry);
    }
  }
  /* first bit */
  /* S = (-C v -XOR)^(C v XOR) <=> S = (-C v EQUIV)^(C v -EQUIV) */
  if (b & (1ULL))
  { /* b[i] = 1 */
    if (carry0 != 0)
    { /* out[i] = 1 xor a[i] xor c[i-1] <=> out[i] = (a[i] <=> c[i-1]) */
      cnf.add_clause (-out[0], a[0], -carry0);
      cnf.add_clause (-out[0], -a[0], carry0);
      cnf.add_clause (out[0], -a[0], -carry0);
      cnf.add_clause (out[0], a[0], carry0);
    }
    else if (carry0_value)
    { /* if c[i-1]=1 */
      /* out[i] = 1 xor 1 xor a[i] <=> out[i] = a[i] */
      cnf.add_clause (-out[0], a[0]);
      cnf.add_clause (out[0], -a[0]);
    }
    else
    { /* if c[i-1]=0 */
      /* out[i] = 1 xor 0 xor a[i] <=> out[i] = -a[i] */
      cnf.add_clause (-out[0], -a[0]);
      cnf.add_clause (out[0], a[0]);
    }
  }
  else
  { /* b[i] = 0 */
    if (carry0 != 0)
    { /* out[i] = 1 xor a[i] xor c[i-1] <=> out[i] = (a[i] xor c[i-1]) */
      cnf.add_clause (-out[0], -a[0], -carry0);
      cnf.add_clause (-out[0], a[0], carry0);
      cnf.add_clause (out[0], a[0], -carry0);
      cnf.add_clause (out[0], -a[0], carry0);
    }
    else if (carry0_value)
    { /* if c[i-1]=1 */
      /* out[i] = 0 xor 1 xor a[i] <=> out[i] = -a[i] */
      cnf.add_clause (-out[0], -a[0]);
      cnf.add_clause (out[0], a[0]);
    }
    else
    { /* if c[i-1]=0 */
      /* out[i] = 0 xor 0 xor a[i] <=> out[i] = a[i] */
      cnf.add_clause (-out[0], a[0]);
      cnf.add_clause (out[0], -a[0]);
    }
  }

  for (guint i = 1; i < a.size (); i++)
  { /* S = (-C v -XOR)^(C v XOR) <=> S = (-C v EQUIV)^(C v -EQUIV) */
    if (b & (1ULL<<i))
    { /* b[i] = 1 */
      if (carries[i-1] != 0)
      { /* out[i] = 1 xor a[i] xor c[i-1] <=> out[i] = (a[i] <=> c[i-1]) */
        cnf.add_clause (-out[i], a[i], -carries[i-1]);
        cnf.add_clause (-out[i], -a[i], carries[i-1]);
        cnf.add_clause (out[i], -a[i], -carries[i-1]);
        cnf.add_clause (out[i], a[i], carries[i-1]);
      }
      else if (carries_vals & (1ULL<<(i-1)))
      { /* if c[i-1]=1 */
        /* out[i] = 1 xor 1 xor a[i] <=> out[i] = a[i] */
        cnf.add_clause (-out[i], a[i]);
        cnf.add_clause (out[i], -a[i]);
      }
      else
      { /* if c[i-1]=0 */
        /* out[i] = 1 xor 0 xor a[i] <=> out[i] = -a[i] */
        cnf.add_clause (-out[i], -a[i]);
        cnf.add_clause (out[i], a[i]);
      }
    }
    else
    { /* b[i] = 0 */
      if (carries[i-1] != 0)
      { /* out[i] = 1 xor a[i] xor c[i-1] <=> out[i] = (a[i] xor c[i-1]) */
        cnf.add_clause (-out[i], -a[i], -carries[i-1]);
        cnf.add_clause (-out[i], a[i], carries[i-1]);
        cnf.add_clause (out[i], a[i], -carries[i-1]);
        cnf.add_clause (out[i], -a[i], carries[i-1]);
      }
      else if (carries_vals & (1ULL<<(i-1)))
      { /* if c[i-1]=1 */
        /* out[i] = 0 xor 1 xor a[i] <=> out[i] = -a[i] */
        cnf.add_clause (-out[i], -a[i]);
        cnf.add_clause (out[i], a[i]);
      }
      else
      { /* if c[i-1]=0 */
        /* out[i] = 0 xor 0 xor a[i] <=> out[i] = a[i] */
        cnf.add_clause (-out[i], a[i]);
        cnf.add_clause (out[i], -a[i]);
      }
    }
  }
}

void
cnf_gen_add (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b, gint32 carry0, bool carry0_value)
{
  assert (a.size () <= out.size () && a.size () == b.size ());
  LiteralVector equivs = cnf.add_vars_with_literals (a.size ());
  LiteralVector carries;

  for (guint i = 0; i < a.size (); i++)
  {
    cnf.add_clause (-equivs[i], a[i], -b[i]);
    cnf.add_clause (-equivs[i], -a[i], b[i]);
    cnf.add_clause (equivs[i], -a[i], -b[i]);
    cnf.add_clause (equivs[i], a[i], b[i]);
  }

  gint32 last_carry;
  /* add one carry if output is bigger than input */
  if (out.size () > a.size ())
  { /* if do save last carry */
    last_carry = out[a.size ()];
    cnf_gen_carry_int (cnf, last_carry, a, b, carries, equivs,
          carry0, carry0_value, DEF_FULL);
  }
  else
  { /* if do not save last carry */
    if (a.size () > 1)
    {
      LiteralVector tmpa (a.begin (), --a.end ());
      LiteralVector tmpb (b.begin (), --b.end ());
      last_carry = cnf.add_var ();
      cnf_gen_carry_int (cnf, last_carry, tmpa, tmpb, carries, equivs,
	    carry0, carry0_value, DEF_FULL);
      carries.push_back (last_carry);
    }
  }

  if (carry0 != 0)
  {
    cnf.add_clause (-out[0], carry0, -equivs[0]);
    cnf.add_clause (-out[0], -carry0, equivs[0]);
    cnf.add_clause (out[0], -carry0, -equivs[0]);
    cnf.add_clause (out[0], carry0, equivs[0]);
  }
  else if (carry0_value)
  {
    cnf.add_clause (-out[0], equivs[0]);
    cnf.add_clause (out[0], -equivs[0]);
  }
  else /* carry0 is zero */
  {
    cnf.add_clause (-out[0], -equivs[0]);
    cnf.add_clause (out[0], equivs[0]);
  }
  for (guint i = 1; i < a.size (); i++)
  { /* S = (-C v -XOR)^(C v XOR) <=> S = (-C v EQUIV)^(C v -EQUIV) */
    cnf.add_clause (-out[i], carries[i-1], -equivs[i]);
    cnf.add_clause (-out[i], -carries[i-1], equivs[i]);
    cnf.add_clause (out[i], -carries[i-1], -equivs[i]);
    cnf.add_clause (out[i], carries[i-1], equivs[i]);
  }
}

void
cnf_gen_sub (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    guint64 b, gint32 carry, bool carry_value)
{
  guint64 mask = (a.size () < 64) ? ((1ULL<<a.size ())-1) : G_MAXUINT64;
  cnf_gen_add (cnf, out, a, (b ^ mask),
      carry, carry_value);
}

void
cnf_gen_sub (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    const LiteralVector& b, gint32 carry, bool carry_value)
{
  LiteralVector notb = b;
  for (LiteralIter it = notb.begin (); it != notb.end (); ++it)
    *it = -*it;

  cnf_gen_add (cnf, out, a, notb, carry, carry_value);
}

void
cnf_gen_sub (CNF& cnf, const LiteralVector& out, guint64 a,
    const LiteralVector& b, gint32 carry, bool carry_value)
{
  LiteralVector notb = b;
  for (LiteralIter it = notb.begin (); it != notb.end (); ++it)
    *it = -*it;

  cnf_gen_add (cnf, out, notb, a, carry, carry_value);
}


/*
 * Dadda multi-adder
 */

void
add_to_dadda_matrix (LiteralMatrix& matrix, const LiteralVector& a)
{
  for (guint i = 0; i < matrix.size (); i++)
    if (a[i] != 0)
      matrix[i].push_back (a[i]);
}

void
cnf_gen_dadda_with_carry (CNF& cnf, const LiteralVector& out,
    const LiteralMatrix& matrix, bool with_carry, gint32 carryvar,
    DefType carrydef)
{
  bool carry_positive = (carrydef == DEF_POSITIVE || carrydef == DEF_FULL);
  bool carry_negative = (carrydef == DEF_NEGATIVE || carrydef == DEF_FULL);

  LiteralVector carries;
  if (with_carry && carry_positive)
    /* push back for definition carryvar => carry_conditions */
    carries.push_back (-carryvar);

  guint used_columns_n = std::min (out.size (), matrix.size ());
  guint maxcolumn_size = 0;
  for (guint i = 0; i < used_columns_n; i++)
    if (matrix[i].size () > maxcolumn_size)
      maxcolumn_size = matrix[i].size ();

  guint step_sizes_tbl[110];
  guint steps_n = 0;
  for (guint max_step_size = 2; max_step_size < maxcolumn_size; steps_n++)
  {
    step_sizes_tbl[steps_n] = max_step_size;
    max_step_size += (max_step_size >> 1);
  }

  /* prepare input data */
  LiteralMatrix curmatrix = matrix;
  curmatrix.resize (out.size ());

  //guint curcolumns_n = out.size ();
  /* main loop */
  for (guint step = steps_n; step > 0; step--)
  {
    guint new_column_size = step_sizes_tbl[step-1];
    /* matrix with extra cells */
    //LiteralMatrix extramatrix (curmatrix.size ());
    //LiteralMatrixIter extracol = extramatrix.begin ();
    LiteralVector extracol1;
    LiteralVector extracol2;
    LiteralVector* extracol = &extracol1;
    LiteralVector* nextextracol = &extracol2;

    for (LiteralMatrixIter col = curmatrix.begin ();
         col != curmatrix.end (); ++col)
    {
      nextextracol->clear ();
      if (col->size () + extracol->size () > new_column_size)
      {
        /* reduce only cells added in previous phase,
           cells added in this phase (extracol, nextextracol) is not reduced */
        guint cells_to_reduce = extracol->size () + col->size () - new_column_size;
        /* determine start position */
        guint src = col->size () - cells_to_reduce - ((cells_to_reduce+1)>>1);
        guint dest = src;
        guint end = col->size ();
        for (;src < end; src += 3, dest++)
        { /* srca,srcb,srcc - source bits from current column */
          gint32 srca = (*col)[src];
          gint32 srcb = (*col)[src+1];
          /* destr - destination for current column */
          gint32 destr = cnf.add_var ();
          /* destination for next column */
          gint32 destc = 0;
          if (col+1 != curmatrix.end ())
            destc = cnf.add_var ();

          if (src + 2 < end)
          { /* if full adder */
            gint32 srcc = (*col)[src+2];
            gint32 equiv = cnf.add_var ();
            /* equiv */
            cnf.add_clause (-equiv, srca, -srcb);
            cnf.add_clause (-equiv, -srca, srcb);
            cnf.add_clause (equiv, -srca, -srcb);
            cnf.add_clause (equiv, srca, srcb);
            if (destc != 0)
            { /* use formuale:
	       c[i+1] = (c[i]v(a[i]==b[i]))^(a[i]vb[i]) */
              cnf.add_clause (-destc, srca, srcb);
              cnf.add_clause (-destc, srcc, equiv);
              cnf.add_clause (destc, -srca, -srcb);
              cnf.add_clause (destc, -srcc, equiv);
            }
            else if (with_carry)
            {
              if (carry_positive)
              {
                gint32 tempvar = cnf.add_var ();
                cnf.add_clause (-tempvar, srca, srcb);
                cnf.add_clause (-tempvar, srcc, equiv);
                carries.push_back (tempvar);
              }
              if (carry_negative)
              {
                cnf.add_clause (carryvar, -srca, -srcb);
                cnf.add_clause (carryvar, -srcc, equiv);
              }
            }
            /* S = (-C v -XOR)^(C v XOR) <=> S = (-C v EQUIV)^(C v -EQUIV) */
            cnf.add_clause (-destr, -srcc, equiv);
            cnf.add_clause (-destr, srcc, -equiv);
            cnf.add_clause (destr, -srcc, -equiv);
            cnf.add_clause (destr, srcc, equiv);
          }
          else
          { /* if half adder */
            if (destc != 0)
            {
              cnf.add_clause (-destc, srca);
              cnf.add_clause (-destc, srcb);
              cnf.add_clause (destc, -srca, -srcb);
            }
            else if (with_carry)
            {
              if (carry_positive)
              {
                gint32 tempvar = cnf.add_var ();
                cnf.add_clause (-tempvar, srca);
                cnf.add_clause (-tempvar, srcb);
                carries.push_back (tempvar);
              }
              if (carry_negative)
                cnf.add_clause (carryvar, -srca, -srcb);
            }
            cnf.add_clause (-destr, srca, srcb);
            cnf.add_clause (-destr, -srca, -srcb);
            cnf.add_clause (destr, srca, -srcb);
            cnf.add_clause (destr, -srca, srcb);
          }
          (*col)[dest] = destr;
          if (destc != 0)
            nextextracol->push_back (destc);
        }
        /* resize to new column size */
        col->resize (dest);
      }

      /* insert from extracols */
      col->insert (col->end (), extracol->begin (), extracol->end ());
      /* swap pointers: extra to nextextracol, nextextracol to extracol */
      std::swap (extracol, nextextracol);
    }
  }

  /* must be with carry and last column must be one or more cells */
  if (with_carry && curmatrix[out.size ()-1].size () >= 1)
  { /* including carry from last phase */
    gint32 last_carry = cnf.add_var ();
    LiteralVector tempout = out;
    tempout.push_back (last_carry);
    curmatrix.resize (curmatrix.size ()+1);
    cnf_gen_dadda_last_phase (cnf, tempout, curmatrix);

    if (carry_positive)
      carries.push_back (last_carry);
    if (carry_negative)
      cnf.add_clause (carryvar, -last_carry);
  }
  else
    cnf_gen_dadda_last_phase (cnf, out, curmatrix);

  if (with_carry && carry_positive)
    /* add clause with positive definition of carries */
    cnf.add_clause (carries);
}

void
cnf_gen_dadda (CNF& cnf, const LiteralVector& out,
    const LiteralMatrix& matrix)
{
  cnf_gen_dadda_with_carry (cnf, out, matrix, false);
}


void
cnf_gen_dadda_last_phase (CNF& cnf, const LiteralVector& out,
    LiteralMatrix& curmatrix)
{
  gint32 carry = 0;
  bool is_carry_one = false;
  gint32 next_carry = 0;
  gint32 is_next_carry_one = 0;
  for (guint i = 0; i < curmatrix.size (); )
  {
    if (curmatrix[i].size () == 2)
    {
      LiteralVector tmpa, tmpb, tmpout;
      for (; i < curmatrix.size () && curmatrix[i].size () == 2; ++i)
      {
	tmpa.push_back (curmatrix[i][0]);
	tmpb.push_back (curmatrix[i][1]);
	tmpout.push_back (out[i]);
      }
      if (i < curmatrix.size ())
      { /* generate next carry only if not end */
        next_carry = cnf.add_var ();
        tmpout.push_back (next_carry);
      }
      cnf_gen_add (cnf, tmpout, tmpa, tmpb, carry, is_carry_one);
    }
    else if (curmatrix[i].size () == 1)
    {
      LiteralVector tmpa, tmpout;
      for (; i < curmatrix.size () && curmatrix[i].size () == 1; ++i)
      {
	tmpa.push_back (curmatrix[i][0]);
	tmpout.push_back (out[i]);
      }

      if (carry == 0 && is_carry_one == false)
      { /* if carry is constant */
	cnf_gen_add (cnf, tmpout, tmpa, 0, carry, is_carry_one);
	next_carry = 0;
	is_next_carry_one = false;
      }
      else
      {
        if (i < curmatrix.size ())
        { /* generate next carry only if not end */
	  next_carry = cnf.add_var ();
	  tmpout.push_back (next_carry);
	}
	cnf_gen_add (cnf, tmpout, tmpa, 0, carry, is_carry_one);
      }
    }
    else /* if zero argument */
    {
      if (carry != 0)
      {
	cnf.add_clause (-out[i], carry);
	cnf.add_clause (out[i], -carry);
      }
      else if (is_carry_one)
	cnf.add_clause (out[i]);
      else
	cnf.add_clause (-out[i]);
      i++;
      /* define as zero if next columns with zero size */
      for (; i < curmatrix.size () && curmatrix[i].size () == 0; ++i)
	cnf.add_clause (-out[i]);
      next_carry = 0;
      is_next_carry_one = false;
    }
    carry = next_carry;
    is_carry_one = is_next_carry_one;
  }
}

/* multiply routine */

void
cnf_gen_uint_mul_matrix(CNF& cnf, LiteralMatrix& matrix,
    const LiteralVector& a, guint64 b)
{
  /* value of two previous literals */
  guint64 toadd = 0;

  /* number of cumulated ones */
  guint cumulated_ones = 0;

  guint b_bits = 0; /* find number of bits for b value */
  for(guint64 v = b; v; v >>= 1, b_bits++);

  guint significands_size = std::min (matrix.size (), size_t(b_bits));

  for (guint i = 0; i <= significands_size; i++)
  {
    if (i < significands_size && (b & (1ULL<<i)) != 0ULL)
      cumulated_ones++;
    else
    {
      if (cumulated_ones >= 3)
      {
	guint jmax = std::min (matrix.size () - i, a.size ());
	guint jmaxmn = std::min (matrix.size () - i + cumulated_ones, a.size ());

	for (guint j = 0; j < jmax; j++)
	  matrix[i+j].push_back(a[j]);
	for (guint j = 0; j < jmaxmn; j++)
	  matrix[i+j-cumulated_ones].push_back(-a[j]);

	/* negated ones + carry */
	toadd += (1ULL<<(i-cumulated_ones));
	if (i + a.size () - cumulated_ones < 64) /* correctly handle shifts */
	{
	  /*guint64 cumulated_ones_val = ((1ULL<<cumulated_ones)-1ULL);
	  toadd += cumulated_ones_val << (i + a.size () - cumulated_ones);*/
	  toadd += ~((1ULL<<(i + a.size  () - cumulated_ones))-1ULL);
	}
      }
      else
      {
	if (cumulated_ones == 2)
	{
	  guint jmaxm2 = std::min (matrix.size () - i + 2, a.size ());
	  for (guint j = 0; j < jmaxm2; j++)
	    matrix[i+j-2].push_back(a[j]);
	}
	if (cumulated_ones >= 1)
	{
	  guint jmaxm1 = std::min (matrix.size () - i + 1, a.size ());
	  for (guint j = 0; j < jmaxm1; j++)
	    matrix[i+j-1].push_back(a[j]);
	}
      }
      cumulated_ones = 0;
    }
  }

  if (toadd)
  {
    gint32 zerovar = cnf.add_var ();
    cnf.add_clause (-zerovar);

    guint realoutsize = 0;
    if (a.size () == 1)
      realoutsize = b_bits;
    else if (b_bits == 1)
      realoutsize = a.size ();
    else
      realoutsize = a.size () + b_bits;

    realoutsize = std::min (guint (matrix.size ()), realoutsize);

    for (guint i = 0; i < realoutsize; i++)
      if (toadd & (1ULL<<i))
	matrix[i].push_back(-zerovar);
  }
}

void
cnf_gen_uint_mul_with_carry (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b,
    bool with_carry, gint32 carryvar, DefType carrydef)
{
  LiteralMatrix matrix (out.size ());

  guint significands_size = std::min (out.size (), a.size ());

  for (guint i = 0; i < significands_size; i++)
  {
    guint jmax = std::min (out.size ()-i, b.size ());
    for (guint j = 0; j < jmax; j++)
    {
      gint32 product = cnf.add_var ();
      cnf.add_clause (-product, a[i]);
      cnf.add_clause (-product, b[j]);
      cnf.add_clause (product, -a[i], -b[j]);
      matrix[i+j].push_back (product);
    }
  }

  cnf_gen_dadda_with_carry (cnf, out, matrix, with_carry,
      carryvar, carrydef);
}

void
cnf_gen_uint_mul_with_carry (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b, bool with_carry,
    gint32 carryvar, DefType carrydef)
{
  LiteralMatrix matrix (out.size ());

//#if 0
  guint b_bits = 0; /* find number of bits for b value */
  for(guint64 v = b; v; v >>= 1, b_bits++);

  guint significands_size = std::min (out.size (), a.size ());
  for (guint i = 0; i < significands_size; i++)
  {
    guint jmax = std::min (guint (out.size ()-i), b_bits);
    for (guint j = 0; j < jmax; j++)
      if (b&(1ULL<<j))
        matrix[i+j].push_back (a[i]);
  }
//#endif
  //cnf_gen_uint_mul_matrix(cnf, matrix, a, b);

  cnf_gen_dadda_with_carry (cnf, out, matrix, with_carry,
      carryvar, carrydef);
}


void
cnf_gen_uint_mul (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b)
{
  LiteralMatrix matrix (out.size ());

  guint significands_size = std::min (out.size (), a.size ());

  for (guint i = 0; i < significands_size; i++)
  {
    guint jmax = std::min (out.size ()-i, b.size ());
    for (guint j = 0; j < jmax; j++)
    {
      gint32 product = cnf.add_var ();
      cnf.add_clause (-product, a[i]);
      cnf.add_clause (-product, b[j]);
      cnf.add_clause (product, -a[i], -b[j]);
      matrix[i+j].push_back (product);
    }
  }

  cnf_gen_dadda (cnf, out, matrix);
}

void
cnf_gen_uint_mul (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, guint64 b)
{
  LiteralMatrix matrix (out.size ());

  cnf_gen_uint_mul_matrix(cnf, matrix, a, b);

  cnf_gen_dadda (cnf, out, matrix);
}

/* value shifting */

void
cnf_gen_shl_int (CNF& cnf, const LiteralVector& out, const LiteralVector& a,
    guint64 aval, const LiteralVector& shift, bool lower)
{
  gint shift_bit = shift.size ()-1;

  LiteralVector temp (out.size ());
  std::vector<bool> tempval (out.size ());
  guint significands_size = std::min (out.size (), a.size ());

  std::copy (a.begin (), a.begin () + significands_size, temp.begin ());
  std::fill (temp.begin () + significands_size, temp.end (), 0);

  std::fill (tempval.begin (), tempval.end (), false);
  for (guint i = 0; i < out.size () && i < 64; i++)
    tempval[i] = (temp[i] == 0 && (aval & (1ULL << i))) ? true : false;

  gint shift_bit_outofrange = shift.size ();
  guint outofrange = 0;

  if (out.size () <= (1U<<shift_bit))
  {
    if (shift_bit > 0 && out.size () <= (1U<<(shift_bit-1)))
    {
      outofrange = cnf.add_var ();
      LiteralVector clause;
      clause.push_back (-outofrange);
      /* generate clause outofrange => shift[n] v .... v shift[x],
	for shift bits with value greater than number of bits of A */
      for (;out.size () <= (1U<<shift_bit) && shift_bit >= 0; shift_bit--)
	clause.push_back (shift[shift_bit]);
      cnf.add_clause (clause);
      /* generate clause outofrange <= shift[n] v .... v shift[x] */
      shift_bit = shift.size () - 1;
      for (; out.size () <= (1U<<shift_bit) && shift_bit >= 0; shift_bit--)
	cnf.add_clause (outofrange, -shift[shift_bit]);
      shift_bit_outofrange = shift_bit+1;
      shift_bit++; /* first bit out of range */
    }
    else
    {
      outofrange = shift[shift_bit];
      shift_bit_outofrange = shift_bit;
    }
  }

  bool lowerval = (lower) ? ((aval&1) == 1) : false;
  gint32 lowervar = (lower) ? a[0] : 0;

  for (; shift_bit >= 0; shift_bit--)
  {
    gint shiftpos = 1<<shift_bit;
    gint32 s = (shift_bit_outofrange > shift_bit) ? shift[shift_bit] : outofrange;
    /* s - current shift bit,
       a0 - right A bit (lower significand),
       a1 - current A bit */
    for (gint i = out.size ()-1; i >= 0; i--)
    {
      bool a0v = (i-shiftpos >= 0) ? tempval[i-shiftpos]: lowerval;
      bool a1v = tempval[i];
      gint32 a0 = (i-shiftpos >= 0) ? temp[i-shiftpos] : lowervar;
      gint32 a1 = temp[i];
      if (a1 != 0)
      {
        if (a1 == a0)
        {
          if (shift_bit != 0)
            temp[i] = a1;
          else
          {
            cnf.add_clause (-out[i], a1);
            cnf.add_clause (out[i], -a1);
          }
        }
        else
        {
	  if (shift_bit != 0)
	    temp[i] = cnf.add_var ();
	  else
	    temp[i] = out[i];

	  if (a0 != 0)
	  { /* definition out[i] = (a1^-s)v(a0^s) */
	    cnf.add_clause (-temp[i], -s, a0);
	    cnf.add_clause (-temp[i], s, a1);
	    cnf.add_clause (temp[i], -s, -a0);
	    cnf.add_clause (temp[i], s, -a1);
	  }
	  else
	  {
	    if (a0v) /* a0 = 1 */
	    { /* out[i] = (a1^-s)v(1^s) -> out[i] = (a1^-s) v s
		-> out[i] = a1 v s */
	      cnf.add_clause (-temp[i], a1, s);
	      cnf.add_clause (temp[i], -a1);
	      cnf.add_clause (temp[i], -s);
	    }
	    else
	    { /* out[i] = (a1^-s)v(0^s) ->  out[i] = a1^-s */
	      cnf.add_clause (-temp[i], a1);
	      cnf.add_clause (-temp[i], -s);
	      cnf.add_clause (temp[i], -a1, s);
	    }
	  }
        }
      }
      else
      {
        if (a0 != 0)
        {
          if (shift_bit != 0)
	    temp[i] = cnf.add_var ();
	  else
	    temp[i] = out[i];

          if (a1v)
          { /* out[i] = (1^-s)v(a0^s) -> out[i] = -s v (a0^s) ->
               out[i] = -s v a0 */
            cnf.add_clause (-temp[i], a0, -s);
            cnf.add_clause (temp[i], -a0);
            cnf.add_clause (temp[i], s);
          }
          else
          { /* out[i] = (0^-s)v(a0^s) ->  out[i] = a0^s */
	    cnf.add_clause (-temp[i], a0);
	    cnf.add_clause (-temp[i], s);
	    cnf.add_clause (temp[i], -a0, -s);
          }
        }
        else
        { /* if zero */
          if (a0v)
          {
            if (a1v)
            { /* out[i] = 1 */
              if (shift_bit != 0)
              {
                temp[i] = 0;
                tempval[i] = true;
              }
              else
                cnf.add_clause (out[i]);
            }
            else
            { /* out[i] = (0^-s)v(1^s) -> out[i] = s */
              if (shift_bit != 0)
                temp[i] = s;
              else
              {
                cnf.add_clause (-out[i], s);
                cnf.add_clause (out[i], -s);
              }
            }
          }
          else
          {
            if (a1v)
            { /* out[i] = (1^-s)v(0^s) -> out[i] = -s */
              if (shift_bit != 0)
                temp[i] = -s;
              else
              {
                cnf.add_clause (-out[i], -s);
                cnf.add_clause (out[i], s);
              }
            }
            else
            {
	      if (shift_bit != 0)
	      {
		temp[i] = 0;
		tempval[i] = false;
              }
	      else
		cnf.add_clause (-out[i]);
            }
          }
        }
      }
    }
  }
}


void
cnf_gen_shl (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& shift, bool lower)
{
  cnf_gen_shl_int (cnf, out, a, 0, shift, lower);
}

void
cnf_gen_shl (CNF& cnf, const LiteralVector& out,
    guint64 a, const LiteralVector& shift, bool lower)
{
  LiteralVector empty (64);
  std::fill (empty.begin (), empty.end (), 0);
  cnf_gen_shl_int (cnf, out, empty, a, shift, lower);
}

/* right shift */

void
cnf_gen_shr (CNF& cnf, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& shift, bool sign)
{
  assert (out.size () <= a.size ());

  LiteralVector newout (out.rbegin (), out.rend ());
  LiteralVector newa (a.rbegin (), a.rend ());

  cnf_gen_shl (cnf, newout, newa, shift, sign);
}

void
cnf_gen_shr (CNF& cnf, const LiteralVector& out,
    guint64 a, guint abits_n, const LiteralVector& shift, bool sign)
{
  assert (out.size () <= abits_n);

  LiteralVector newout (out.rbegin (), out.rend ());
  guint64 newa = 0;
  for (guint i = 0; i < abits_n; i++)
    newa |= (a & (1ULL << i)) ? (1ULL << (abits_n-i-1)) : 0;

  cnf_gen_shl (cnf, newout, newa, shift, sign);
}

/* division and remainder */

void
cnf_gen_uint_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b)
{
  LiteralVector outmod;
  LiteralVector outdiv;

  if (isdiv)
  {
    outmod = cnf.add_vars_with_literals (b.size ());
    outdiv = out;
  }
  else
  {
    outdiv = cnf.add_vars_with_literals (a.size ());
    outmod = out;
  }

  assert (outmod.size () <= b.size ());
  assert (outdiv.size () <= a.size ());
  assert (a.size () >= b.size ());

  /* define by formulae: (div*b+mod=a) AND (mod<b) AND (div*b+mod <= MAX) */
  LiteralMatrix matrix (a.size ());

  for (guint i = 0; i < outdiv.size (); i++)
  {
    guint jmax = std::min (a.size ()-i, b.size ());
    for (guint j = 0; j < jmax; j++)
    {
      gint32 product = cnf.add_var ();
      cnf.add_clause (-product, outdiv[i]);
      cnf.add_clause (-product, b[j]);
      cnf.add_clause (product, -outdiv[i], -b[j]);
      matrix[i+j].push_back (product);
    }
    /* higher rest must be zero */
    for (guint j = jmax; j < b.size (); j++)
      /* -(outdiv[i]^b[j]) */
      cnf.add_clause (-outdiv[i], -b[j]);
  }
  /* div*b+mod=a */
  for (guint i = 0; i < outmod.size (); i++)
    matrix[i].push_back (outmod[i]);

  /* all carries must be zero */
  cnf_gen_dadda_with_carry (cnf, a, matrix, true, 0, DEF_NEGATIVE);
  /* outmod<b */
  if (outmod.size () == b.size ())
    cnf_gen_less (cnf, 0, outmod, b, DEF_POSITIVE);
  else
  {
    LiteralVector tmpb (b.begin (), b.begin () + outmod.size ());
    LiteralVector clause;
    gint32 isless = cnf.add_var ();
    cnf_gen_less (cnf, 0, outmod, tmpb, DEF_POSITIVE);
    clause.push_back (isless);
    clause.insert (clause.end (), b.begin () + outmod.size (), b.end ());
    cnf.add_clause (clause);
  }
}

void
cnf_gen_uint_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, guint64 b)
{
  LiteralVector outmod;
  LiteralVector outdiv;

  guint b_bits = 0; /* find number of bits for b value */
  for(guint64 v = b; v; v >>= 1, b_bits++);

  assert (a.size () >= b_bits);

  if (isdiv)
  {
    outmod = cnf.add_vars_with_literals (b_bits);
    outdiv = out;

    assert (outmod.size () <= b_bits);
    assert (outdiv.size () <= a.size ());

    for (guint i = 0; i < outdiv.size (); i++)
    {
      guint jmax = std::min (guint (a.size ()-i), b_bits);
      for (guint j = jmax; j < b_bits; j++)
        /* -(outdiv[i]^b[j]) */
	if (b & (1ULL<<j))
	  cnf.add_clause (-outdiv[i]);
    }
  }
  else
  {
    outdiv.assign (a.size (), 1);
    outmod = out;

    assert (outdiv.size () <= a.size ());
    assert (outmod.size () <= b_bits);

    for (guint i = 0; i < outdiv.size (); i++)
    {
      guint jmax = std::min (guint (a.size ()-i), b_bits);
      for (guint j = jmax; j < b_bits; j++)
        /* -(outdiv[i]^b[j]) */
	if (b & (1ULL<<j))
	  outdiv[i] = 0; /* outdiv[i] must be zero */
    }
    for (LiteralIter it = outdiv.begin (); it != outdiv.end (); ++it)
      if (*it != 0) /* if not zero then add variable */
        *it = cnf.add_var ();
  }

  /* define by formulae: (div*b+mod=a) AND (mod<b) AND (div*b+mod <= MAX) */
  LiteralMatrix matrix (a.size ());

  for (guint i = 0; i < outdiv.size (); i++)
  {
    guint jmax = std::min (guint (a.size ()-i), b_bits);
    for (guint j = 0; j < jmax; j++)
      if (b & (1ULL<<j) && outdiv[i])
        /* if b[i] is true and if outdiv is not zero */
        matrix[i+j].push_back (outdiv[i]);
  }
  /* div*b+mod=a */
  for (guint i = 0; i < outmod.size (); i++)
    matrix[i].push_back (outmod[i]);

  /* all carries must be zero */
  cnf_gen_dadda_with_carry (cnf, a, matrix, true, 0, DEF_NEGATIVE);
  /* outmod<b (only if bbits == outmod.size ()) */
  if (outmod.size () == b_bits)
    cnf_gen_less (cnf, 0, outmod, b, DEF_POSITIVE);
}

void
cnf_gen_int_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, const LiteralVector& b)
{
  LiteralVector outmod; /* positive modulo */
  LiteralVector outdiv;

  if (isdiv)
  {
    outmod = cnf.add_vars_with_literals (b.size ());
    outdiv = out;
    assert (outdiv.size () <= a.size ());
  }
  else
  {
    outdiv = cnf.add_vars_with_literals (a.size ()+1);
    outmod = out;
  }

  assert (outmod.size () <= b.size ());
  assert (a.size () >= b.size ());

  /* abs(value) of outdiv, a, b */
  LiteralVector negdiv = cnf.add_vars_with_literals (outdiv.size ());
  LiteralVector negmod = cnf.add_vars_with_literals (outmod.size ());
  LiteralVector nega = cnf.add_vars_with_literals (a.size ());
  LiteralVector negb = cnf.add_vars_with_literals (b.size ());

  cnf_gen_sub (cnf, negdiv, 0, outdiv); /* -div */
  cnf_gen_sub (cnf, negmod, 0, outmod); /* -a */
  cnf_gen_sub (cnf, nega, 0, a); /* -a */
  cnf_gen_sub (cnf, negb, 0, b); /* -b */

  LiteralVector abs_div = cnf.add_vars_with_literals (outdiv.size ());
  LiteralVector abs_mod = cnf.add_vars_with_literals (outmod.size ());
  LiteralVector abs_a = cnf.add_vars_with_literals (a.size ());
  LiteralVector abs_b = cnf.add_vars_with_literals (b.size ());

  gint32 divsign = outdiv[outdiv.size () - 1];
  gint32 asign = a[a.size () - 1];
  gint32 bsign = b[b.size () - 1];

  if (!isdiv) /* prevention for cases -4%-1 for 4bit signed output */
  {
    gint32 beforedivsign = outdiv[outdiv.size () - 2];
    /* sign[0]=1 -> sign[-1]=1 */
    cnf.add_clause (beforedivsign, -divsign);
  }

  /* negate if value is negative */
  cnf_gen_ite (cnf, abs_div, divsign, negdiv, outdiv);
  cnf_gen_ite (cnf, abs_mod, asign, negmod, outmod);
  cnf_gen_ite (cnf, abs_a, asign, nega, a);
  cnf_gen_ite (cnf, abs_b, bsign, negb, b);

  gint32 blesseqa = cnf.add_var ();

  /* sign(div) = (sign(a) xor sign(b)) ^ (abs(b)>=abs(a)) */
  cnf.add_clause (-divsign, asign, bsign);
  cnf.add_clause (-divsign, -asign, -bsign);
  cnf.add_clause (-divsign, blesseqa);
  cnf.add_clause (divsign, asign, -bsign, -blesseqa);
  cnf.add_clause (divsign, -asign, bsign, -blesseqa);

  /* adds clause: abs(a)>=abs(b) */
  if (a.size () == b.size ())
    cnf_gen_lesseq (cnf, blesseqa, abs_b, abs_a, DEF_FULL);
  else
  { /* TEST IT */
    LiteralVector a0 (abs_a.begin (), abs_a.begin () + abs_b.size ());
    LiteralVector a1 (abs_a.begin () + abs_b.size (), abs_a.end ());
    LiteralVector clause;
    gint32 islesseq = cnf.add_var ();
    gint32 iszero = cnf.add_var ();
    cnf_gen_less (cnf, islesseq, abs_b, a0, DEF_FULL);
    cnf_gen_equal (cnf, iszero, a1, 0, DEF_FULL);
    cnf.add_clause (-blesseqa, islesseq, iszero);
    cnf.add_clause (blesseqa, -islesseq);
    cnf.add_clause (blesseqa, -iszero);
  }

  /* define by formulae: (div*b+mod=a) AND (mod<b) AND (div*b+mod <= MAX) */
  LiteralMatrix matrix (abs_a.size ());

  for (guint i = 0; i < abs_div.size (); i++)
  {
    guint jmax = std::min (abs_a.size ()-i, abs_b.size ());
    for (guint j = 0; j < jmax; j++)
    {
      gint32 product = cnf.add_var ();
      cnf.add_clause (-product, abs_div[i]);
      cnf.add_clause (-product, abs_b[j]);
      cnf.add_clause (product, -abs_div[i], -abs_b[j]);
      matrix[i+j].push_back (product);
    }
    /* higher rest must be zero */
    for (guint j = jmax; j < b.size (); j++)
      /* -(outdiv[i]^b[j]) */
      cnf.add_clause (-abs_div[i], -abs_b[j]);
  }
  /* div*b+mod=a */
  for (guint i = 0; i < outmod.size (); i++)
    matrix[i].push_back (abs_mod[i]);

  /* all carries must be zero */
  cnf_gen_dadda_with_carry (cnf, abs_a, matrix, true, 0, DEF_NEGATIVE);
  /* outmod<b */
  if (outmod.size () == b.size ())
    cnf_gen_less (cnf, 0, abs_mod, abs_b, DEF_POSITIVE);
  else
  {
    LiteralVector tmpb (abs_b.begin (), abs_b.begin () + abs_mod.size ());
    LiteralVector clause;
    gint32 isless = cnf.add_var ();
    cnf_gen_less (cnf, 0, abs_mod, tmpb, DEF_POSITIVE);
    clause.push_back (isless);
    clause.insert (clause.end (), abs_b.begin () + abs_mod.size (),
        abs_b.end ());
    cnf.add_clause (clause);
    /* sign of outmod must be same as sign of A, (expect if absmod is zero)
       in this case absmod always in good range */
    gint32 absmod_is_zero = cnf.add_var ();
    cnf_gen_equal (cnf, absmod_is_zero, abs_mod, 0, DEF_FULL);
    /*cnf.add_clause (outmod[outmod.size ()-1], -asign);
    cnf.add_clause (-outmod[outmod.size ()-1], asign);*/
    cnf.add_clause (outmod[outmod.size ()-1], -asign);
    cnf.add_clause (-outmod[outmod.size ()-1], asign);
    cnf.add_clause (-outmod[outmod.size ()-1], -absmod_is_zero);
  }
}

void
cnf_gen_int_divmod (CNF& cnf, bool isdiv, const LiteralVector& out,
    const LiteralVector& a, gint64 b)
{
  LiteralVector outmod; /* positive modulo */
  LiteralVector outdiv;

  gint64 abs_b = std::abs (b);
  guint b_bits = 0;

  if (b >= 0)
    for(guint64 v = b; v; v >>= 1, b_bits++);
  else
    //for(guint64 v = abs_b; v; v >>= 1, b_bits++);
    for (gint64 v = b; v != -1LL; v >>= 1, b_bits++);

  b_bits++;

  if (isdiv)
  {
    outmod = cnf.add_vars_with_literals (b_bits);
    outdiv = out;
    assert (outdiv.size () <= a.size ());
  }
  else
  {
    outdiv = cnf.add_vars_with_literals (a.size ()+1);
    outmod = out;
  }

  assert (outmod.size () <= b_bits);
  assert (a.size () >= b_bits);

  /* abs(value) of outdiv, a, b */
  LiteralVector negdiv = cnf.add_vars_with_literals (outdiv.size ());
  LiteralVector negmod = cnf.add_vars_with_literals (outmod.size ());
  LiteralVector nega = cnf.add_vars_with_literals (a.size ());

  cnf_gen_sub (cnf, negdiv, 0, outdiv); /* -div */
  cnf_gen_sub (cnf, negmod, 0, outmod); /* -a */
  cnf_gen_sub (cnf, nega, 0, a); /* -a */

  LiteralVector abs_div = cnf.add_vars_with_literals (outdiv.size ());
  LiteralVector abs_mod = cnf.add_vars_with_literals (outmod.size ());
  LiteralVector abs_a = cnf.add_vars_with_literals (a.size ());


  gint32 divsign = outdiv[outdiv.size () - 1];
  gint32 asign = a[a.size () - 1];

  if (!isdiv) /* prevention for cases -4%-1 for 4bit signed output */
  {
    gint32 beforedivsign = outdiv[outdiv.size () - 2];
    /* sign[0]=1 -> sign[-1]=1 */
    cnf.add_clause (beforedivsign, -divsign);
  }

  /* negate if value is negative */
  cnf_gen_ite (cnf, abs_div, divsign, negdiv, outdiv);
  cnf_gen_ite (cnf, abs_mod, asign, negmod, outmod);
  cnf_gen_ite (cnf, abs_a, asign, nega, a);

  gint32 blesseqa = cnf.add_var ();

  /* sign(div) = (sign(a) xor sign(b)) ^ (abs(b)>=abs(a)) */
  if (b >= 0)
  {
    cnf.add_clause (-divsign, asign);
    cnf.add_clause (-divsign, blesseqa);
    cnf.add_clause (divsign, -asign, -blesseqa);
  }
  else
  {
    cnf.add_clause (-divsign, -asign);
    cnf.add_clause (-divsign, blesseqa);
    cnf.add_clause (divsign, asign, -blesseqa);
  }

  /* adds clause: !(abs(a)<abs(b)) -> abs(a)>=abs(b) */
  cnf_gen_less (cnf, -blesseqa, abs_a, abs_b, DEF_FULL);

  /* define by formulae: (div*b+mod=a) AND (mod<b) AND (div*b+mod <= MAX) */
  LiteralMatrix matrix (abs_a.size ());

  for (guint i = 0; i < abs_div.size (); i++)
  {
    guint jmax = std::min (guint (a.size ()-i), b_bits);
    for (guint j = 0; j < jmax; j++)
      if (abs_b & (1ULL<<j) && abs_div[i])
        /* if b[i] is true and if outdiv is not zero */
        matrix[i+j].push_back (abs_div[i]);

    /* higher rest must be zero */
    for (guint j = jmax; j < b_bits; j++)
      /* -(outdiv[i]^b[j]) */
      if (abs_b & (1ULL<<j))
        cnf.add_clause (-abs_div[i]);
  }
  /* div*b+mod=a */
  for (guint i = 0; i < outmod.size (); i++)
    matrix[i].push_back (abs_mod[i]);

  /* all carries must be zero */
  cnf_gen_dadda_with_carry (cnf, abs_a, matrix, true, 0, DEF_NEGATIVE);
  if (outmod.size () == b_bits)
    cnf_gen_less (cnf, 0, abs_mod, abs_b, DEF_POSITIVE);
  else if (outmod.size () < b_bits)
  {
    /* sign of outmod must be same as sign of A (expect if absmod is zero),
       in this case absmod always in good range */
    gint32 absmod_is_zero = cnf.add_var ();
    cnf_gen_equal (cnf, absmod_is_zero, abs_mod, 0, DEF_FULL);
    /*cnf.add_clause (outmod[outmod.size ()-1], -asign);
    cnf.add_clause (-outmod[outmod.size ()-1], asign);*/
    cnf.add_clause (outmod[outmod.size ()-1], -asign, absmod_is_zero);
    cnf.add_clause (-outmod[outmod.size ()-1], asign);
    cnf.add_clause (-outmod[outmod.size ()-1], -absmod_is_zero);
  }
}
