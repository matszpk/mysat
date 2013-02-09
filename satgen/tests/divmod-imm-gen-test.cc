/*
 * divmod-imm-gen-test.cc - generate division modulo testing formulae
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
gen_divmod_imm_test (guint abits_n, guint64 bvalue, guint outbits_n, bool issigned,
    bool isdiv, const std::string& cnffilename)
{
  CNF cnf;

  gint64 bsignvalue = bvalue;
  guint bbits_n = 0;
  if (!issigned || bsignvalue >= 0)
  {
    for(guint64 v = bvalue; v; v >>= 1, bbits_n++);
    if (issigned)
      bbits_n++;
  }
  else
  {
    for (gint64 v = bvalue; (v != -1L); v >>= 1, bbits_n++);
    bbits_n++;
  }
  
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector bvar = cnf.add_vars_with_literals (bbits_n);
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
#if 1
  gint32 zerovar = 0;
  if (!issigned)
  {
    zerovar = cnf.add_var ();
    cnf.add_clause (-zerovar);
  }
  
  if (issigned)
    cnf_gen_int_divmod (cnf, isdiv, out1, a, bsignvalue);
  else
    cnf_gen_uint_divmod (cnf, isdiv, out1, a, bvalue);
  
  cnf_gen_equal (cnf, 0, bvar, bvalue, DEF_POSITIVE);
  
  /*{
    std::ofstream file ("outmap");
    file << "a = { u : ";
    for (guint i = 0; i < abits_n; i++)
      file << a[i] << ' ';
    file << "}\n";
    file << "out1 = { u : ";
    for (guint i = 0; i < outbits_n; i++)
      file << out1[i] << ' ';
    file << "}\n";
    file << "out2 = { u : ";
    for (guint i = 0; i < outbits_n; i++)
      file << out2[i] << ' ';
    file << "}\n";
  }*/
  
  {
    guint maxbits_n;
    if (isdiv)
      maxbits_n = std::max (abits_n, outbits_n + bbits_n + 1);
    else /* if modulo mode: output have size of A */
      maxbits_n = abits_n + bbits_n + 1;
    
    LiteralVector temp = cnf.add_vars_with_literals (maxbits_n);
    LiteralVector tmpmod;
    LiteralVector tmpout;
    if (isdiv)
    {
      tmpmod = cnf.add_vars_with_literals (bbits_n);
      tmpout = out2;
    }
    else
    { /* is modulo */
      tmpmod = out2;
      tmpout = cnf.add_vars_with_literals (abits_n);
    }
    LiteralVector tempa = a;
    LiteralVector tempb = bvar;
    
    /* zero extend MOD and B to MAXBITS */
    if (!issigned)
    {
      tempb.insert (tempb.end (), maxbits_n-bbits_n, zerovar);
      tmpmod.insert (tmpmod.end (), maxbits_n-tmpmod.size (), zerovar);
      tempa.insert (tempa.end (), maxbits_n-abits_n, zerovar);
    }
    else
    {
      gint32 asign = a[a.size ()-1];
      gint32 bsign = bvar[bvar.size ()-1];
      tempb.insert (tempb.end (), maxbits_n-bbits_n, bsign);
      tmpmod.insert (tmpmod.end (), maxbits_n-tmpmod.size (), asign);
      tempa.insert (tempa.end (), maxbits_n-abits_n, asign);
    }
    
    /* temp = out2*b, where out2*b */
    if (issigned)
      cnf_gen_uint_mul (cnf, temp, tmpout, tempb);
    else
      cnf_gen_uint_mul (cnf, temp, tmpout, bvar);
    /* tempa = out2*b+m, without carry */
    cnf_gen_add (cnf, tempa, temp, tmpmod);
    /* mod < b (b is zero extended) */
    if (!issigned)
      cnf_gen_less (cnf, 0, tmpmod, tempb, DEF_POSITIVE);
    else
    {
      LiteralVector negb = cnf.add_vars_with_literals (tempb.size ());
      LiteralVector negmod = cnf.add_vars_with_literals (tmpmod.size ());
      cnf_gen_sub (cnf, negb, 0, tempb); /* -b */
      cnf_gen_sub (cnf, negmod, 0, tmpmod); /* -mod */
      LiteralVector finalb = cnf.add_vars_with_literals (tempb.size ());
      LiteralVector finalmod = cnf.add_vars_with_literals (tmpmod.size ());
      
      gint32 bsign = bvar[bvar.size () - 1];
      gint32 asign = a[a.size () - 1];
      cnf_gen_ite (cnf, finalb, bsign, negb, tempb);
      cnf_gen_ite (cnf, finalmod, asign, negmod, tmpmod);
      
      cnf_gen_less (cnf, 0, finalmod, finalb, DEF_POSITIVE);
      
      /* modsign must be same as asign */
      gint32 mod_iszero = cnf.add_var ();
      cnf_gen_equal (cnf, mod_iszero, finalmod, 0, DEF_FULL);
      gint32 modsign = (isdiv) ? tmpmod[bbits_n-1] : tmpmod[out2.size ()-1];
      /*cnf.add_clause (modsign, -asign);
      cnf.add_clause (-modsign, asign);*/
      cnf.add_clause (modsign, -asign, mod_iszero);
      cnf.add_clause (-modsign, asign);
      cnf.add_clause (-modsign, -mod_iszero);
    }
  }
  
  /* out1 != out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
#else
  /*{
    std::ofstream file ("outmap");
    file << "a = { u : ";
    for (guint i = 0; i < abits_n; i++)
      file << a[i] << ' ';
    file << "}\n";
    file << "out1 = { u : ";
    for (guint i = 0; i < outbits_n; i++)
      file << out1[i] << ' ';
    file << "}\n";
  }

  cnf_gen_equal (cnf, 0, a, 60, DEF_POSITIVE);
  cnf_gen_int_divmod (cnf, false, out1, a, bsignvalue);*/
#endif
  cnf.save_to_file (cnffilename);
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 7)
  {
    std::cout << "divmod-imm-gen-test ABITS BVALUE OUTBITS SIGNED DIV CNFFILE"
        << std::endl;
    return 0;
  }
  
  guint abits_n, outbits_n;
  gint64 bsignvalue;
  guint64 bvalue;
  bool issigned, isdiv;
  
  {
    std::istringstream is (argv[4]);
    is >> issigned;
    if (is.fail ())
    {
      std::cerr << "Cant parse is_signed" << std::endl;
      return 1;
    }
  }
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
    if (issigned)
    {
      is >> bsignvalue;
      bvalue = bsignvalue;
    }
    else
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
      std::cerr << "Cant parse out_bits_n" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[5]);
    is >> isdiv;
    if (is.fail ())
    {
      std::cerr << "Cant parse is_div" << std::endl;
      return 1;
    }
  }
  
  gen_divmod_imm_test (abits_n, bvalue, outbits_n, issigned, isdiv, argv[6]);
  
  return 0;
}
