/*
 * shift-imm-gen-test.cc - shift imm gen test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "module.h"
#include "gen-arith.h"

void
gen_shift_test (guint64 avalue, guint shiftbits_n, guint outbits_n, bool lower,
      const std::string& cnffilename)
{
  CNF cnf;
  LiteralVector a = cnf.add_vars_with_literals (64);
  LiteralVector shift = cnf.add_vars_with_literals (shiftbits_n);
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  
  cnf_gen_equal (cnf, 0, a, avalue, DEF_POSITIVE);
  
  cnf_gen_shl (cnf, out1, avalue, shift, lower);
  cnf_gen_shl (cnf, out2, a, shift, lower);
  
  /* out1 != out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
  
  cnf.save_to_file (cnffilename);
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 6)
  {
    std::cout << "shift-gen-test AVALUE SHIFTBITS OUTBITS LOWER CNFFILE" << std::endl;
    return 0;
  }
  
  guint64 avalue;
  guint shiftbits_n, outbits_n;
  bool lower;
  
  {
    std::istringstream is (argv[1]);
    is >> avalue;
    if (is.fail ())
    {
      std::cerr << "Cant parse avalue" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[2]);
    is >> shiftbits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse shift_bits_n" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[3]);
    is >> outbits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse out_bits_n" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[4]);
    is >> lower;
    if (is.fail ())
    {
      std::cerr << "Cant parse lower" << std::endl;
      return 1;
    }
  }
  
  gen_shift_test (avalue, shiftbits_n, outbits_n, lower, argv[5]);
  
  return 0;
}

