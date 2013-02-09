/*
 * ite-gen-test.cc - generate ite gen testing formulae
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <sstream>
#include <vector>
#include "module.h"
#include "gen-arith.h"


static void
my_gen_ite (CNF& cnf, const LiteralVector& out, gint32 cond,
    const LiteralVector& a, const LiteralVector& b)
{
  for (guint i = 0; i < a.size (); i++)
  {
    gint32 t1 = cnf.add_var ();
    gint32 t2 = cnf.add_var ();
    cnf.add_clause (-out[i], t1, t2);
    cnf.add_clause (-t1, a[i]);
    cnf.add_clause (-t1, cond);
    cnf.add_clause (-t2, b[i]);
    cnf.add_clause (-t2, -cond);
    cnf.add_clause (out[i], -a[i], -cond);
    cnf.add_clause (out[i], -b[i], cond);
  }
}

static void
gen_ite_test (guint abits_n, guint bbits_n, guint outbits_n,
      const std::string& cnffilename)
{
  CNF cnf;
  
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector b = cnf.add_vars_with_literals (bbits_n);
  gint32 cond = cnf.add_var ();
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  
  cnf_gen_ite (cnf, out1, cond, a, b);
  my_gen_ite (cnf, out2, cond, a, b);
  
  /* out1 != out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
  
  cnf.save_to_file (cnffilename);
}

static void
gen_ite_imm_test (guint abits_n, guint64 bval, guint outbits_n,
      const std::string& cnffilename)
{
  CNF cnf;
  
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector b = cnf.add_vars_with_literals (abits_n);
  gint32 cond = cnf.add_var ();
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  
  cnf_gen_equal (cnf, 0, b, bval, DEF_POSITIVE);
  
  cnf_gen_ite (cnf, out1, cond, a, bval);
  my_gen_ite (cnf, out2, cond, a, b);
  
  /* out1 != out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
  
  cnf.save_to_file (cnffilename);
}

static void
gen_ite_2imms_test (guint64 aval, guint64 bval, guint outbits_n,
      const std::string& cnffilename)
{
  CNF cnf;
  
  LiteralVector a = cnf.add_vars_with_literals (outbits_n);
  LiteralVector b = cnf.add_vars_with_literals (outbits_n);
  gint32 cond = cnf.add_var ();
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  
  cnf_gen_equal (cnf, 0, a, aval, DEF_POSITIVE);
  cnf_gen_equal (cnf, 0, b, bval, DEF_POSITIVE);
  
  cnf_gen_ite (cnf, out1, cond, aval, bval);
  my_gen_ite (cnf, out2, cond, a, b);
  
  /* out1 != out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
  
  cnf.save_to_file (cnffilename);
}


int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 4)
  {
    std::cout << "ite-gen-test A B OUTBITS IMMMODE CNFFILE" << std::endl;
    return 0;
  }
  
  guint abits_n, bbits_n;
  guint64 aval;
  guint64 bval;
  guint outbits_n;
  guint imms_n;
  
  {
    std::istringstream is (argv[4]);
    is >> imms_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse immmode" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[1]);
    if (imms_n >= 2)
      is >> aval;
    else
      is >> abits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse A" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[2]);
    if (imms_n >= 1)
      is >> bval;
    else
      is >> bbits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse B" << std::endl;
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
  
  if (imms_n == 0)
    gen_ite_test (abits_n, bbits_n, outbits_n, argv[5]);
  else if (imms_n == 1)
    gen_ite_imm_test (abits_n, bval, outbits_n, argv[5]);
  else
    gen_ite_2imms_test (bval, bval, outbits_n, argv[5]);
   
  return 0;
}
