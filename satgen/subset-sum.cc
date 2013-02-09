/*
 * subset-sum.cc - subset sum module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <sstream>
#include "gen-arith.h"
#include "subset-sum.h"

using namespace SatUtils;

SubsetSumModule::SubsetSumModule ()
    : result_value (0)
{
  add_param ("sum", "Required result of subset sum", result_value);
}

SubsetSumModule::~SubsetSumModule ()
{
}

SatGenModule*
SubsetSumModule::create ()
{
  return static_cast<SatGenModule*>(new SubsetSumModule ());
}

void
SubsetSumModule::post_process_params ()
{
}

void
SubsetSumModule::parse_input (const std::string& input)
{
  numbers.clear ();
  std::istringstream is (input);
  
  while (!is.eof ())
  {
    gint64 value;
    is >> value;
    if (is.eof ())
      break;
    if (is.fail ())
      throw InputSyntaxError ("Bad number");
    numbers.push_back (value);
  }
}

namespace
{

/* generate matrix for dadda adder from vector of numbers.
   return: start of variables of switches */

static gint32
generate_numbers_def (CNF& cnf, LiteralMatrix& matrix,
    const std::vector<gint64>& nums, gint32 zerovar)
{
  /* generate pairs */
  gint32 switches_start = cnf.next_var ();
  cnf.add_vars (nums.size ());
  guint nums_n = nums.size ();
  guint outbits_n = matrix.size ();
  
  /* join two numbers into one variable dependent of two switchers */
  for (guint i = 0; i+1 < nums_n; i += 2)
  {
    /* current switch variable number */
    gint32 curswitch = switches_start + i;
    /* values of possible cases */
    gint64 first = nums[i];
    gint64 second = nums[i+1];
    gint64 sum = nums[i] + nums[i+1];
    
    /* only eights combinations, because: switch1=0, switch2=0 then
       switch1*n1+switch2*n2 is always zero */
    bool combs[8];
    gint32 combvars[8];
    std::fill (combs, combs+8, false); /* zeroing */
    /* determine type of switching */
    for (guint j = 0; j < outbits_n; j++)
    {
      guint index = ((first & (1ULL<<j)) ? 1 : 0) |
          ((second & (1ULL<<j)) ? 2 : 0) |
          ((sum & (1ULL<<j)) ? 4 : 0);
      combs[index] = true;
    }
    
    if (combs[1])
    { /* if in only first number bit is 1 */
      combvars[1] = cnf.add_var ();
      cnf.add_clause (-combvars[1], curswitch);
      cnf.add_clause (-combvars[1], -(curswitch+1));
      cnf.add_clause (combvars[1], -curswitch, (curswitch+1));
    }
    if (combs[2])
    { /* if in only second number bit is 1 */
      combvars[2] = cnf.add_var ();
      cnf.add_clause (-combvars[2], -curswitch);
      cnf.add_clause (-combvars[2], (curswitch+1));
      cnf.add_clause (combvars[2], curswitch, -(curswitch+1));
    }
    if (combs[3])
    { /* if in first or second (exclusively) numbers bit is 1 */
      combvars[3] = cnf.add_var ();
      cnf.add_clause (-combvars[3], curswitch, curswitch+1);
      cnf.add_clause (-combvars[3], -curswitch, -(curswitch+1));
      cnf.add_clause (combvars[3], curswitch, -(curswitch+1));
      cnf.add_clause (combvars[3], -curswitch, (curswitch+1));
    }
    if (combs[4])
    { /* if in sum of first and second numbers bit must be 1 */
      combvars[4] = cnf.add_var ();
      cnf.add_clause (-combvars[4], curswitch);
      cnf.add_clause (-combvars[4], (curswitch+1));
      cnf.add_clause (combvars[4], -curswitch, -(curswitch+1));
    }
    /* combs[5]: in first number or sum bit must be 1 */
    combvars[5] = curswitch;
    /* combs[6]: in second number or sum bit must be 1 */
    combvars[6] = curswitch+1;
    if (combs[7])
    { /* if in first or second or sum numbers bit is 1 */
      combvars[7] = cnf.add_var ();
      cnf.add_clause (-combvars[7], curswitch, (curswitch+1));
      cnf.add_clause (combvars[7], -curswitch);
      cnf.add_clause (combvars[7], -(curswitch+1));
    }
    
    /* now push to matrix this combination */
    for (guint j = 0; j < outbits_n; j++)
    {
      guint index = ((first & (1ULL<<j)) ? 1 : 0) |
          ((second & (1ULL<<j)) ? 2 : 0) |
          ((sum & (1ULL<<j)) ? 4 : 0);
      if (index != 0)
        matrix[j].push_back (combvars[index]);
    }
  }
  
  if (nums_n & 1) /* if odd number of values */
  {
    gint32 curswitch = switches_start + nums_n-1;
    /* push to matrix switch n-1 for last value */
    gint64 last = nums[nums_n-1];
    for (guint j = 0; j < outbits_n; j++)
      if (last & (1ULL<<j))
        matrix[j].push_back (curswitch);
  }
  return switches_start;
}

};

void
SubsetSumModule::generate (CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  gint64 neg_sum = 0;
  gint64 pos_sum = 0;
  /* negnums - absolute values of negative numbers from set,
     posnums - absolute values of positive numbers from set */
  std::vector<gint64> negnums, posnums;
  
  for (std::vector<gint64>::const_iterator it = numbers.begin ();
       it != numbers.end (); ++it)
    if (*it >= 0)
    {
      posnums.push_back (*it);
      pos_sum += *it;
    }
    else
    {
      negnums.push_back (-*it);
      neg_sum += -*it;
    }
  
  std::sort (posnums.begin (), posnums.end ());
  std::sort (negnums.begin (), negnums.end ());
  
  /* adding required sum to neg/pos sum */
  if (result_value > 0) /* p1=p2+s */
    neg_sum += result_value;
  else if (result_value < 0) /* p1+(-s)=p2 */
    pos_sum -= result_value;
  
  guint outbits_n;
  guint pos_bits_n, neg_bits_n;
  
  {
    /* count numbers of bits required for storing results of sum */
    pos_bits_n = 0;
    for (gint64 v = 1; v <= pos_sum; v <<= 1, pos_bits_n++);
    neg_bits_n = 0;
    for (gint64 v = 1; v <= neg_sum; v <<= 1, neg_bits_n++);
    pos_bits_n = (pos_bits_n) ? pos_bits_n : 1;
    neg_bits_n = (neg_bits_n) ? neg_bits_n : 1;
    
    outbits_n = std::max (pos_bits_n, neg_bits_n);
  }
  
  gint32 zerovar = cnf.add_var ();
  cnf.add_clause (-zerovar);
  
  /* generate dadda */
  LiteralMatrix neg_matrix (neg_bits_n);
  LiteralMatrix pos_matrix (pos_bits_n);
  LiteralVector pos_out = cnf.add_vars_with_literals (pos_bits_n);
  LiteralVector neg_out = cnf.add_vars_with_literals (neg_bits_n);
  
  gint32 pos_switches = generate_numbers_def (cnf, pos_matrix, posnums, zerovar);
  gint32 neg_switches = generate_numbers_def (cnf, neg_matrix, negnums, zerovar);
  
  /* generate outmap */
  if (with_outmap)
  {
    outmap_string.clear ();
    for (guint i = 0; i < posnums.size (); i++)
    {
      std::ostringstream os;
      os << "{ {|" << posnums[i] << "\n} :" << pos_switches + i << " }";
      outmap_string += os.str ();
    }
    for (guint i = 0; i < negnums.size (); i++)
    {
      std::ostringstream os;
      os << "{ {|" << -negnums[i] << "\n} :" << neg_switches + i << " }";
      outmap_string += os.str ();
    }
  }
  
  if (result_value == 0)
  { /* add clause for nonempty subset */
    LiteralVector nonempty = get_literals_from_range (pos_switches,
          pos_switches + posnums.size ());
    add_literals_from_range (nonempty, neg_switches,
          neg_switches + negnums.size ());
  
    cnf.add_clause (nonempty);
  }
  
  cnf_gen_dadda (cnf, pos_out, pos_matrix);
  cnf_gen_dadda (cnf, neg_out, neg_matrix);
  
  /* now last phase is generate equality and add constant */
  {
    if (result_value != 0)
    { /* sum is not zero */
      LiteralVector new_out;
      if (result_value > 0)
      { /* p1=p2+s */
        new_out = cnf.add_vars_with_literals (neg_bits_n);
        cnf_gen_add (cnf, new_out, neg_out, result_value, 0, false);
        neg_out = new_out; /* replace neg_out by new output */
      }
      else if (result_value < 0)
      { /* p1+(-s)=p2, where s is negative */
        new_out = cnf.add_vars_with_literals (pos_bits_n);
        cnf_gen_add (cnf, new_out, pos_out, -result_value, 0, false);
        pos_out = new_out;
      }
    }
    
    if (pos_bits_n < neg_bits_n)
    {
      LiteralVector tmp_neg_out (neg_out.begin (), neg_out.begin () + pos_bits_n);
      cnf_gen_equal (cnf, 0, pos_out, tmp_neg_out, DEF_POSITIVE);
      tmp_neg_out.assign (neg_out.begin () + pos_bits_n, neg_out.end ());
      cnf_gen_equal (cnf, 0, tmp_neg_out, 0, DEF_POSITIVE);
    }
    else
    {
      LiteralVector tmp_pos_out (pos_out.begin (), pos_out.begin () + neg_bits_n);
      cnf_gen_equal (cnf, 0, tmp_pos_out, neg_out, DEF_POSITIVE);
      if (pos_bits_n != neg_bits_n)
      {
	tmp_pos_out.assign (pos_out.begin () + neg_bits_n, pos_out.end ());
	cnf_gen_equal (cnf, 0, tmp_pos_out, 0, DEF_POSITIVE);
      }
    }
  }
}
