/*
 * dadda-gen-test-2.cc -
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
  LiteralMatrix add_matrix;
  LiteralMatrix dadda_matrix (col_sizes.size ());
  
  guint mincolsize = *(std::min_element (col_sizes.begin (), col_sizes.end ()));
  guint maxcolsize = *(std::max_element (col_sizes.begin (), col_sizes.end ()));
  
  maxcolsize = std::max (maxcolsize, 2U);
  
  if (mincolsize != maxcolsize)
  {
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
  
  guint outbits_n = col_sizes.size ();
  
  {
    gint32 outputs;
    for (guint k = maxcolsize; k > 2; k >>= 1)
    {
      outputs = cnf.next_var ();
      cnf.add_vars ((k>>1)*outbits_n);
      guint j = 0;
      for (guint i = 0; i < k; i += 2, j++)
        if (i+1 < k)
        {
          LiteralVector c = get_literals_from_range (outputs + j*outbits_n,
              outputs + (j+1)*outbits_n);
          cnf_gen_add (cnf, c, add_matrix[i], add_matrix[i+1], 0, false);
          add_matrix[j] = c;
        }
        else
          add_matrix[j] = add_matrix[i];
      
      if (k & 1)
        k++;
    }
    
    cnf_gen_add (cnf, out1, add_matrix[0], add_matrix[1], 0, false);
  }
  
  cnf_gen_dadda (cnf, out2, dadda_matrix);
  
  /* negation of out1==out2 */
  cnf_gen_equal (cnf, 0, out1, out2, DEF_NEGATIVE);
  
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
