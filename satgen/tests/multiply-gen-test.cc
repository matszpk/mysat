/*
 * multiply-gen-test.cc - multiply gen test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <sstream>
#include <vector>
#include "module.h"
#include "gen-arith.h"

static void
gen_multiply_test (guint abits_n, guint64 bvalue, guint outbits_n,
    const std::string& cnffilename)
{
  CNF cnf;
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector b = cnf.add_vars_with_literals (64);
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out3 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out4 = cnf.add_vars_with_literals (outbits_n);
  gint32 carry1 = cnf.add_var ();
  gint32 carry2 = cnf.add_var ();
  
  cnf_gen_equal (cnf, 0, b, bvalue, DEF_POSITIVE);
  cnf_gen_uint_mul (cnf, out1, a, b);
  cnf_gen_uint_mul (cnf, out2, a, bvalue);
  
  cnf_gen_uint_mul_with_carry (cnf, out3, a, b, true, carry1, DEF_FULL);
  cnf_gen_uint_mul_with_carry (cnf, out4, a, bvalue, true, carry2, DEF_FULL);
  
  {
    /* fullout1 = {out1,out3,carry1}, fullout2 .... */
    LiteralVector fullout1 (out1.begin (), out1.end ());
    LiteralVector fullout2 (out2.begin (), out2.end ());
    fullout1.insert (fullout1.end (), out3.begin (), out3.end ());
    fullout2.insert (fullout2.end (), out4.begin (), out4.end ());
    /* add carry to output vectors */
    fullout1.push_back (carry1);
    fullout2.push_back (carry2);
    /* out3 != out4 */
    cnf_gen_equal (cnf, 0, fullout1, fullout2, DEF_NEGATIVE);
  }
  
  cnf.save_to_file (cnffilename);
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 5)
  {
    std::cout << "multiply-gen-test ABITS BVALUE OUTBITS CNFFILE" << std::endl;
    return 0;
  }
  
  guint abits_n, outbits_n;
  guint64 bvalue;
  
  {
    std::istringstream is (argv[1]);
    is >> abits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse a_bits_n" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[2]);
    is >> bvalue;
    if (is.fail ())
    {
      std::cerr << "Cant parse bvalue" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[3]);
    is >> outbits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse outbits_n" << std::endl;
      return 1;
    }
  }
  
  gen_multiply_test (abits_n, bvalue, outbits_n, argv[4]);
  
  return 0;
}
