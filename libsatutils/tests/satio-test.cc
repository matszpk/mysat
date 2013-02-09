/*
 * satio-test.cc - testing SAT Input/Output
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <cstdio>
#include <glibmm.h>
#include <satutils.h>

using namespace SatUtils;

static bool
cnf_io_test ()
{
  static gint32 cnf_data[] =
  {
    3, 1, 2, -4,
    3, 4, 3, -1,
    2, 2, 5,
    4, 5, -4, 1, -2,
  };
  
  std::cout << "Saving CNF" << std::endl;
  CNF cnf (5, 4);
  cnf.get_formulae ().assign (cnf_data, cnf_data + 16);
  if (!cnf.check_consistency ())
    return false;
  
  cnf.save_to_file ("test.cnf");
  
  std::cout << "Loading CNF" << std::endl;
  CNF cnf2 ("test.cnf");
  if (cnf != cnf2)
    return false;
  
  remove ("test.cnf");
  
  std::cout << "Generating CNF with using add_clause" << std::endl;
  cnf2.clear ();
  cnf2.add_vars (5);
  cnf2.add_clause (1, 2, -4);
  cnf2.add_clause (4, 3, -1);
  cnf2.add_clause (2, 5);
  cnf2.add_clause (5, -4, 1, -2);
  
  if (cnf != cnf2)
    return false;
  
  std::cout << "Generating CNF with using add_clause 2" << std::endl;
  cnf2.clear ();
  cnf2.add_vars (5);
  cnf2.add_clause (3, cnf_data + 1);
  cnf2.add_clause (3, cnf_data + 1+3+1);
  cnf2.add_clause (2, cnf_data + 1+3+1+3+1);
  cnf2.add_clause (4, cnf_data + 1+3+1+3+1+2+1);
  if (cnf != cnf2)
    return false;
  
  std::cout << "Generating CNF with using add_clause 3" << std::endl;
  cnf2.clear ();
  cnf2.add_vars (5);
  cnf2.add_clause (LiteralVector (cnf_data + 1, cnf_data + 1+3));
  cnf2.add_clause (LiteralVector (cnf_data + 1+3+1, cnf_data + 1+3+1+3));
  cnf2.add_clause (LiteralVector (cnf_data + 1+3+1+3+1, cnf_data + 1+3+1+3+1+2));
  cnf2.add_clause (LiteralVector (cnf_data + 1+3+1+3+1+2+1,
          cnf_data + 1+3+1+3+1+2+1+4));
  if (cnf != cnf2)
    return false;
  
  std::cout << "Loading and saving more complicated CNF" << std::endl;
  cnf.load_from_file (TEST_DIR "/input.cnf");
  cnf.save_to_file ("output.cnf");
  cnf2.load_from_file ("output.cnf");
  if (cnf != cnf2)
    return false;
  
  remove ("output.cnf");
  return true;
}

static bool
cnf_eval_test ()
{
  static const bool input_values1_1[] = { true, false, true };
  static const bool input_values1_2[] = { true, true, true };
  static const bool input_values1_3[] = { false, false, true };
  static const bool input_values1_4[] = { true, false, false };
  
  std::cout << "Testing evaluation of CNF with 3 variables" << std::endl;
  
  CNF cnf;
  cnf.add_vars (3);
  cnf.add_clause (3, 1, 2);
  cnf.add_clause (-3, 1);
  cnf.add_clause (-2, -1);
  cnf.add_clause (1, -2, 3);
  
  if (!cnf.evaluate (std::vector<bool>(input_values1_1, input_values1_1 + 3)))
    return false;
  if (cnf.evaluate (std::vector<bool>(input_values1_2, input_values1_2 + 3)))
    return false;
  if (cnf.evaluate (std::vector<bool>(input_values1_3, input_values1_3 + 3)))
    return false;
  if (!cnf.evaluate (std::vector<bool>(input_values1_4, input_values1_4 + 3)))
    return false;
  
  std::cout << "Testing evaluation of CNF with 5 variables" << std::endl;
  
  static const bool input_values2_1[] = { false, false, false, true, true };
  static const bool input_values2_2[] = { true, false, false, true, true };
  static const bool input_values2_3[] = { true, false, false, false, true };
  static const bool input_values2_4[] = { true, false, true, true, false};
  
  cnf.clear ();
  cnf.add_vars (5);
  cnf.add_clause (3, 5, -1, 2);
  cnf.add_clause (2, -5, 4);
  cnf.add_clause (-3, 1, 2, 4);
  cnf.add_clause (5, -3, -1);
  
  
  if (!cnf.evaluate (std::vector<bool>(input_values2_1, input_values2_1 + 5)))
    return false;
  if (!cnf.evaluate (std::vector<bool>(input_values2_2, input_values2_2 + 5)))
    return false;
  if (cnf.evaluate (std::vector<bool>(input_values2_3, input_values2_3 + 5)))
    return false;
  if (cnf.evaluate (std::vector<bool>(input_values2_4, input_values2_4 + 5)))
    return false;
  
  return true;
}

/* main function */

int
main (int argc, char** argv)
{
  Glib::init ();
  
  try
  {
    if (!cnf_io_test ())
    {
      std::cerr << "CNF IO failed" << std::endl;
      return 1;
    }
    if (!cnf_eval_test ())
    {
      std::cerr << "CNF evaluation is failed" << std::endl;
      return 1;
    }
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return 1;
  }
  
  return 0;
}
