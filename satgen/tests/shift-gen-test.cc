/*
 * shift-gen-test.cc - shift gen test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "module.h"
#include "gen-arith.h"

static void
gen_shift_test (guint abits_n, guint shiftbits_n, guint outbits_n, bool lower,
      const std::string& cnffilename)
{
  CNF cnf;
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector shift = cnf.add_vars_with_literals (shiftbits_n);
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  
  /*{
    std::ofstream file ("outmap");
    file << "a = { x : ";
    for (guint i = 0; i < abits_n; i++)
      file << a[i] << ' ';
    file << "}\n";
    file << "shift = { u : ";
    for (guint i = 0; i < shiftbits_n; i++)
      file << shift[i] << ' ';
    file << "}\n";
    file << "out1 = { x : ";
    for (guint i = 0; i < outbits_n; i++)
      file << out1[i] << ' ';
    file << "}\n";
    file << "out2 = { x : ";
    for (guint i = 0; i < outbits_n; i++)
      file << out2[i] << ' ';
    file << "}\n";
  }*/
  
  /* generate shift left by multiply */
  if (!lower)
  {
    guint maxshift = std::min (outbits_n, 1U << shiftbits_n);
    LiteralVector b = cnf.add_vars_with_literals (maxshift);
    
    for (guint i = 0; i < maxshift; i++)
      cnf_gen_equal (cnf, b[i], shift, i, DEF_FULL);
    
    cnf_gen_uint_mul (cnf, out1, a, b);
  }
  else
  {
    guint maxshift = std::min (outbits_n, 1U << shiftbits_n);
    LiteralMatrix matrix (outbits_n);
    for (guint i = 0; i < maxshift; i++)
    {
      gint32 pred = cnf.add_var ();
      cnf_gen_equal (cnf, pred, shift, i, DEF_FULL);
      /* add a[0] from 0 to i-1 column */
      for (guint j = 0; j < i; j++)
      {
        gint32 product = cnf.add_var ();
        cnf.add_clause (-product, a[0]);
        cnf.add_clause (-product, pred);
        cnf.add_clause (product, -a[0], -pred);
        matrix[j].push_back (product);
      }
      guint jmax = std::min (outbits_n-i, abits_n);
      for (guint j = 0; j < jmax; j++)
      {
        gint32 product = cnf.add_var ();
	cnf.add_clause (-product, pred);
	cnf.add_clause (-product, a[j]);
	cnf.add_clause (product, -pred, -a[j]);
	matrix[i+j].push_back (product);
      }
    }
    if (outbits_n < (1U << shiftbits_n))
    { /* add special case if outbits_n < maximum shift */
      gint32 pred = cnf.add_var ();
      /* pred is shift>=outbits_n */
      cnf_gen_less (cnf, -pred, shift, outbits_n, DEF_FULL);
      for (guint j = 0; j < outbits_n; j++)
      {
        gint32 product = cnf.add_var ();
        cnf.add_clause (-product, a[0]);
        cnf.add_clause (-product, pred);
        cnf.add_clause (product, -a[0], -pred);
        //cnf.add_clause (-product);
        matrix[j].push_back (product);
      }
    }
    
    cnf_gen_dadda (cnf, out1, matrix);
  }
  
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
    std::cout << "shift-gen-test ABITS SHIFTBITS OUTBITS LOWER CNFFILE" << std::endl;
    return 0;
  }
  
  guint abits_n, shiftbits_n, outbits_n;
  bool lower;
  
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
  
  gen_shift_test (abits_n, shiftbits_n, outbits_n, lower, argv[5]);
  
  return 0;
}
