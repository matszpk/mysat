/*
 * dadda-gen-test.cc - dadda gen test
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
gen_dadda_test (guint vars_n, guint inbits_n, guint outbits_n)
{
  CNF cnf;
  LiteralVector out1 = cnf.add_vars_with_literals (outbits_n);
  LiteralVector out2 = cnf.add_vars_with_literals (outbits_n);
  gint32 input_start = cnf.next_var ();
  cnf.add_vars (vars_n*inbits_n);
  
  {
    gint32 outputs;
    LiteralMatrix temp_matrix (vars_n);
    
    gint32 zerovar = 0;
    if (inbits_n < outbits_n)
    {
      zerovar = cnf.add_var ();
      cnf.add_clause (-zerovar);
    }
    
    /* initialize inputs */
    for (guint i = 0; i < vars_n; i++)
    {
      LiteralVector a = get_literals_from_range (input_start + i*inbits_n,
            input_start + (i+1)*inbits_n);
      if (inbits_n < outbits_n)
        for (guint j = inbits_n; j < outbits_n; j++)
          a.push_back (zerovar);
      temp_matrix[i] = a;
    }
    
    for (guint k = vars_n; k > 2; k >>= 1)
    {
      outputs = cnf.next_var ();
      cnf.add_vars ((k>>1)*outbits_n);
      guint j = 0;
      for (guint i = 0; i < k; i += 2, j++)
        if (i+1 < k)
        {
          LiteralVector c = get_literals_from_range (outputs + j*outbits_n,
              outputs + (j+1)*outbits_n);
          cnf_gen_add (cnf, c, temp_matrix[i], temp_matrix[i+1], 0, false);
          temp_matrix[j] = c;
        }
        else
          temp_matrix[j] = temp_matrix[i];
      
      if (k & 1)
        k++; /* if odd numbers */
    }
    
    /* last phase */
    cnf_gen_add (cnf, out1, temp_matrix[0], temp_matrix[1], 0, false);
  }
  
  LiteralMatrix matrix (inbits_n);
  for (guint i = 0; i < vars_n; i++)
  {
    LiteralVector v = get_literals_from_range (input_start + i*inbits_n,
        input_start + (i+1)*inbits_n);
    add_to_dadda_matrix (matrix, v);
  }
  
  cnf_gen_dadda (cnf, out2, matrix);
  
  /* negation of out1==out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
  
  cnf.save_to_file ("dadda-test.cnf");
}

/* main function */

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 4)
  {
    std::cout << "dadda-gen-test VARS INBITS OUTBITS" << std::endl;
    return 0;
  }
  
  guint vars_n;
  guint inbits_n, outbits_n;
  {
    std::istringstream is (argv[1]);
    is >> vars_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse VARS number" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[2]);
    is >> inbits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse INBITS number" << std::endl;
      return 1;
    }
  }
  {
    std::istringstream is (argv[3]);
    is >> outbits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse OUTBITS number" << std::endl;
      return 1;
    }
  }
  
  gen_dadda_test (vars_n, inbits_n, outbits_n);
  
  return 0;
}
