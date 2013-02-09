/*
 * divmod-gen-bltmod.cc - generate test formulae for checking comparing b<mos in
 * int divmod
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <string>
#include "gen-arith.h"

void
gen_divmod_imm_test_cnf (guint abits_n, guint outbits_n,
    gint64 aval, gint64 bval, gint64 outval, const std::string& cnffilename,
    bool isdiv = false)
{
  CNF cnf;
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector out = cnf.add_vars_with_literals (outbits_n);
  cnf_gen_equal (cnf, 0, a, guint64 (aval), DEF_POSITIVE);
  cnf_gen_equal (cnf, 0, out, guint64 (outval), DEF_POSITIVE);
  
  cnf_gen_int_divmod (cnf, isdiv, out, a, bval);
  
  cnf.save_to_file (cnffilename);
}

void
gen_divmod_test_cnf (guint abits_n, guint bbits_n, guint outbits_n,
    gint64 aval, gint64 bval, gint64 outval, const std::string& cnffilename,
    bool isdiv = false)
{
  CNF cnf;
  LiteralVector a = cnf.add_vars_with_literals (abits_n);
  LiteralVector b = cnf.add_vars_with_literals (bbits_n);
  LiteralVector out = cnf.add_vars_with_literals (outbits_n);
  cnf_gen_equal (cnf, 0, a, guint64 (aval), DEF_POSITIVE);
  cnf_gen_equal (cnf, 0, b, guint64 (bval), DEF_POSITIVE);
  cnf_gen_equal (cnf, 0, out, guint64 (outval), DEF_POSITIVE);
  
  cnf_gen_int_divmod (cnf, isdiv, out, a, bval);
  
  cnf.save_to_file (cnffilename);
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  gen_divmod_imm_test_cnf (5, 3, -4, 5, -4, "bltmod1-sat.cnf"); /* mod=-4 */
  gen_divmod_imm_test_cnf (5, 3, -4, -5, -4, "bltmod2-sat.cnf"); /* mod=-4 */
  gen_divmod_imm_test_cnf (5, 3, 4, 5, 4, "bltmod3-unsat.cnf"); /* mod=-4 */
  gen_divmod_imm_test_cnf (5, 3, 4, -5, 4, "bltmod4-unsat.cnf"); /* mod=-4 */
  gen_divmod_imm_test_cnf (5, 3, -3, 5, -3, "bltmod5-sat.cnf"); /* mod=-3 */
  gen_divmod_imm_test_cnf (5, 3, -3, -5, -3, "bltmod6-sat.cnf"); /* mod=-3 */
  gen_divmod_imm_test_cnf (5, 3, 3, 5, 3, "bltmod5-2-sat.cnf"); /* mod=3 */
  gen_divmod_imm_test_cnf (5, 3, 3, -5, 3, "bltmod6-2-sat.cnf"); /* mod=3 */
  
  gen_divmod_test_cnf (5, 4, 3, -4, 5, -4, "bltmod7-sat.cnf"); /* mod=-4 */
  gen_divmod_test_cnf (5, 4, 3, -4, -5, -4, "bltmod8-sat.cnf"); /* mod=-4 */
  gen_divmod_test_cnf (5, 4, 3, 4, 5, 4, "bltmod9-unsat.cnf"); /* mod=-4 */
  gen_divmod_test_cnf (5, 4, 3, 4, -5, 4, "bltmod10-unsat.cnf"); /* mod=-4 */
  gen_divmod_test_cnf (5, 4, 3, -3, 5, -3, "bltmod11-sat.cnf"); /* mod=-3 */
  gen_divmod_test_cnf (5, 4, 3, -3, -5, -3, "bltmod12-sat.cnf"); /* mod=-3 */
  gen_divmod_test_cnf (5, 4, 3, 3, 5, 3, "bltmod11-2-sat.cnf"); /* mod=3 */
  gen_divmod_test_cnf (5, 4, 3, 3, -5, 3, "bltmod12-2-sat.cnf"); /* mod=3 */
  
  gen_divmod_imm_test_cnf (5, 1, -4, -1, 0, "bltmod13-sat.cnf"); /* mod=3 */
  gen_divmod_test_cnf (5, 3, 1, -4, -1, 0, "bltmod14-sat.cnf"); /* mod=3 */
  return 0;
}
