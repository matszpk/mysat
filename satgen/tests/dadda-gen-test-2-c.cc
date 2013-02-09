/*
 * dadda-gen-test-2-c.cc -
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "module.h"
#include "gen-arith.h"

using namespace SatUtils;

bool
parse_input_file (const std::string& filename, std::vector<guint>& colsizes)
{
  std::ifstream file (filename.c_str ());
  if (!file)
  {
    std::cerr << "Cant open file" << std::endl;
    return false;
  }
  
  while (1)
  {
    guint size;
    file >> size;
    if (file.eof ())
      break;
    if (file.fail ())
    {
      if (file.badbit)
        std::cerr << "I/O error" << std::endl;
      else
        std::cerr << "Parse error" << std::endl;
      return false;
    }
    
    colsizes.push_back (size);
  }
  
  return true;
}

void
gen_dadda_test (const std::string& cnffilename, const std::vector<guint>& col_sizes)
{
  CNF cnf;
  
  gint32 zerovar = 0;
  LiteralVector out1 = cnf.add_vars_with_literals (col_sizes.size ());
  LiteralVector out2 = cnf.add_vars_with_literals (col_sizes.size ());
  gint32 carry1 = cnf.add_var ();
  gint32 carry2 = cnf.add_var ();
  LiteralMatrix add_matrix;
  LiteralMatrix dadda_matrix (col_sizes.size ());
  
  guint maxcolsize = *(std::max_element (col_sizes.begin (), col_sizes.end ()));
  maxcolsize = std::max (maxcolsize, 2U);
  
  { /* always add zerovar */
    zerovar = cnf.add_var ();
    cnf.add_clause (-zerovar);
  }
  add_matrix.resize (maxcolsize);
  
  for (LiteralMatrixIter it = add_matrix.begin (); it != add_matrix.end (); ++it)
  {
    it->resize (col_sizes.size ());
    std::fill (it->begin (), it->end (), zerovar);
  }
  
  for (guint i = 0; i < col_sizes.size (); i++)
  {
    LiteralVector input = cnf.add_vars_with_literals (col_sizes[i]);
    dadda_matrix[i] = input;
    for (guint j = 0; j < col_sizes[i]; j++)
      add_matrix[j][i] = input[j];
  }
  
  /* add extra bits to normal adding output */
  guint outbits_n = col_sizes.size ();
  guint add_outbits_n = 0;
  
  for (guint v = 1; v < maxcolsize; v <<= 1, add_outbits_n++);
  
  add_outbits_n = (add_outbits_n) ? add_outbits_n : 1;
  add_outbits_n += outbits_n;
  
  gint32 extra_out = cnf.next_var ();
  cnf.add_vars (add_outbits_n-outbits_n);
  add_literals_from_range (out1, extra_out, extra_out + add_outbits_n-outbits_n);
  
  /* resize adding matrix */
  for (guint i = 0; i < maxcolsize; i++)
    add_matrix[i].insert (add_matrix[i].end (), add_outbits_n-outbits_n, zerovar);
  
  {
    gint32 outputs;
    for (guint k = maxcolsize; k > 2; k >>= 1)
    {
      outputs = cnf.next_var ();
      cnf.add_vars ((k>>1)*add_outbits_n);
      guint j = 0;
      for (guint i = 0; i < k; i += 2, j++)
        if (i+1 < k)
        {
          LiteralVector c = get_literals_from_range (outputs + j*add_outbits_n,
              outputs + (j+1)*add_outbits_n);
          cnf_gen_add (cnf, c, add_matrix[i], add_matrix[i+1], 0, false);
          add_matrix[j] = c;
        }
        else
          add_matrix[j] = add_matrix[i];
      
      if (k & 1)
        k++;
    }
    
    cnf_gen_add (cnf, out1, add_matrix[0], add_matrix[1], 0, false);
    
    /* define carry by checking extra != 0 */
    LiteralVector extras = get_literals_from_range (extra_out,
        extra_out + add_outbits_n-outbits_n);
    cnf_gen_equal (cnf, -carry1, extras, 0, DEF_FULL);
  }
  
  cnf_gen_dadda_with_carry (cnf, out2, dadda_matrix, true, carry2, DEF_FULL);
  
  /* negation of {out1,carry1}=={out2,carry2} */
  {
    LiteralVector tmpout1 (out1.begin (), out1.begin () + outbits_n);
    LiteralVector tmpout2 (out2.begin (), out2.begin () + outbits_n);
    tmpout1.push_back (carry1);
    tmpout2.push_back (carry2);
    cnf_gen_equal (cnf, 0, tmpout1, tmpout2, DEF_NEGATIVE);
  }
  cnf.save_to_file (cnffilename);
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (argc < 3)
  {
    std::cout << "dadda-gen-test-2 INPUTFILE CNFFILE" << std::endl;
    return 0;
  }
  
  std::vector<guint> column_sizes;
  
  if (!parse_input_file (argv[1], column_sizes))
    return 1;
  
  gen_dadda_test (argv[2], column_sizes);
  
  return 0;
}
