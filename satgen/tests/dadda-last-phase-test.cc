/*
 * dadda-last-phase-test.cc - Dadda last phase test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <sstream>
#include <vector>
#include "module.h"
#include "gen-arith.h"

using namespace SatUtils;

static void
gen_dadda_last_phase_test (const std::vector<guint>& colsizes)
{
  CNF cnf;
  
  {
    LiteralVector out1 = cnf.add_vars_with_literals (colsizes.size ());
    LiteralMatrix matrix (colsizes.size ());
    for (guint i = 0; i < colsizes.size (); i++)
      matrix[i] = cnf.add_vars_with_literals (colsizes[i]);
    
    cnf_gen_dadda_last_phase (cnf, out1, matrix);
    cnf.save_to_file ("dadda-last-phase-test-1.cnf");
    
    gint zerovar = 0;
    LiteralVector out2 = cnf.add_vars_with_literals (colsizes.size ());
    LiteralVector a (colsizes.size ());
    LiteralVector b (colsizes.size ());
    
    for (guint i = 0; i < colsizes.size (); i++)
    {
      if (colsizes[i] < 2 && zerovar == 0)
      {
        zerovar = cnf.add_var ();
        cnf.add_clause (-zerovar);
      }
      
      if (colsizes[i] >= 1)
        a[i] = matrix[i][0];
      else
        a[i] = zerovar;
      
      if (colsizes[i] == 2)
        b[i] = matrix[i][1];
      else
        b[i] = zerovar;
    }
    
    cnf_gen_add (cnf, out2, a, b, 0, false);
    
    /* if not equal out1 and out2 */
    cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
    
    cnf.save_to_file ("dadda-last-phase-test-2.cnf");
  }
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 2)
  {
    std::cout << "dadda-last-phase-test COLSIZE [COLSIZE [COLSIZE]...]" << std::endl;
    return 0;
  }
  
  std::vector<guint> colsizes;
  for (int i = 1; i < argc; i++)
  {
    guint v;
    std::istringstream is (argv[i]);
    is >> v;
    if (is.fail ())
    {
      std::cerr << "Cant parse column size" << std::endl;
      return 1;
    }
    if (v > 2)
    {
      std::cerr << "Must be <3." << std::endl;
      return 1;
    }
    colsizes.push_back (v);
  }
  
  gen_dadda_last_phase_test (colsizes);
  
  return 0;
}
