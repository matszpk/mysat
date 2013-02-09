/*
 * spear-force-test.cc - spear force test
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include "spear-format.h"

static std::string
gen_input_string (const std::string& op, bool resv, bool av, bool bv,
    guint resbits_n, guint64 res, guint abits_n, guint64 a, guint bbits_n, guint64 b)
{
  std::ostringstream os;
  os << "v 1.0\ne 0\n";
  os << "d";
  if (resv)
    os << " res:i" << resbits_n;
  if (av)
    os << " a:i" << abits_n;
  if (bv)
    os << " b:i" << bbits_n;
  os << '\n';
  /* constraint */
  os << "c";
  if (resv)
    os << " res";
  else
    os << ' ' << res << ":i" << resbits_n;
  os << ' ' << op;
  if (av)
    os << " a";
  else
    os << ' ' << a << ":i" << abits_n;
  if (bv)
    os << " b";
  else
    os << ' ' << b << ":i" << bbits_n;
  os << '\n';
  
  if (resv)
    os << "p = res " << res << ":i" << resbits_n << '\n';
  if (av)
    os << "p = a " << a << ":i" << abits_n << '\n';
  if (bv)
    os << "p = b " << b << ":i" << bbits_n << '\n';
  
  return os.str ();
}

void
do_tests (const std::string& solver, guint bits_n)
{
  /* /s */
  guint combs_n = 1ULL<<bits_n;
  for (guint64 res = 0; res < combs_n; res++)
    for (guint64 a = 0; a < combs_n; a++)
      for (guint64 b = 0; b < combs_n; b++)
        for (guint vcomb = 0; vcomb < 8; vcomb++)
	{
	  CNF cnf;
	  SatGenModule* module = SpearFormatModule::create ();
	  std::string outmap_string;
	  bool resv = ((vcomb & 1) != 0) ? true : false;
	  bool av = ((vcomb & 2) != 0) ? true : false;
	  bool bv = ((vcomb & 4) != 0) ? true : false;
	  std::string input = gen_input_string ("/s", resv, av, bv,
	       bits_n, res, bits_n, a, bits_n, b);
	  std::cout << "Test " << ((resv) ? 'v':'i') << ':' << res <<
	     ' ' << "/s" << ' ' <<
	     ((av) ? 'v':'i') << ':' << a << ' ' <<
	     ((bv) ? 'v':'i') << ':' << b << ' ';
	  
	  try
	  {
	    module->parse_input (input);
	    module->generate (cnf, outmap_string, false);
	    cnf.save_to_file ("test.cnf");
	  }
	  catch (Glib::Exception& ex)
	  {
	    delete module;
	    std::ofstream file ("input.sf");
	    file << input;
	    
	    std::cerr << ex.what () << std::endl;
	    return;
	  }
	  //std::cout << input;
	  
	  bool sat = true;
	  guint64 signs = ~((1ULL<<bits_n)-1);
	  guint64 signmask = 1ULL<<(bits_n-1);
	  gint64 s_a = (a&signmask) ? (a|signs) : a;
	  gint64 s_b = (b&signmask) ? (b|signs) : b;
	  gint64 s_res = (res&signmask) ? (res|signs) : res;
	  if (s_b != 0)
	  {
	    gint64 expected = s_a / s_b;
	    if (expected != s_res)
	      sat = false;
	  }
	  else
	    sat = false;
	  
	  std::cout << ((sat) ? "sat" : "unsat") << std::endl;
	  
	  std::string command = solver;
	  command += " test.cnf";
	  //command = "false";
	  std::string stdoutput;
	  std::string stderror;
	  int exit_status = 0;
	  Glib::spawn_command_line_sync (command,
	     &stdoutput, &stderror, &exit_status);
	  //exit_status &= 255;
	  if (((exit_status == 2560) && !sat) ||
	      ((exit_status == 5120) && sat) ||
	      (exit_status != 2560 && exit_status != 5120))
	  {
	    delete module;
	    std::ofstream file ("input.sf");
	    file << input;
	    
	    std::cerr << "Test is failed: exit:" << exit_status << std::endl;
	    return;
	  }
	  std::cout << "exit:" << exit_status << std::endl;
	  delete module;
	}
}

void
do_tests_2 (const std::string& solver, guint bits_n)
{
  /* /s */
  guint combs_n = 1ULL<<bits_n;
  for (guint64 res = 0; res < combs_n; res++)
    for (guint64 a = 0; a < combs_n; a++)
      for (guint64 b = 0; b < combs_n; b++)
        for (guint vcomb = 0; vcomb < 8; vcomb++)
	{
	  CNF cnf;
	  SatGenModule* module = SpearFormatModule::create ();
	  std::string outmap_string;
	  bool resv = ((vcomb & 1) != 0) ? true : false;
	  bool av = ((vcomb & 2) != 0) ? true : false;
	  bool bv = ((vcomb & 4) != 0) ? true : false;
	  std::string input = gen_input_string ("/u", resv, av, bv,
	       bits_n, res, bits_n, a, bits_n, b);
	  std::cout << "Test " << ((resv) ? 'v':'i') << ':' << res <<
	     ' ' << "/u" << ' ' <<
	     ((av) ? 'v':'i') << ':' << a << ' ' <<
	     ((bv) ? 'v':'i') << ':' << b << ' ';
	  
	  try
	  {
	    module->parse_input (input);
	    module->generate (cnf, outmap_string, false);
	    cnf.save_to_file ("test.cnf");
	  }
	  catch (Glib::Exception& ex)
	  {
	    delete module;
	    std::ofstream file ("input.sf");
	    file << input;
	    
	    std::cerr << ex.what () << std::endl;
	    return;
	  }
	  //std::cout << input;
	  
	  bool sat = true;
	  if (b != 0)
	  {
	    guint64 expected = a / b;
	    if (expected != res)
	      sat = false;
	  }
	  else
	    sat = false;
	  
	  std::cout << ((sat) ? "sat" : "unsat") << std::endl;
	  
	  std::string command = solver;
	  command += " test.cnf";
	  //command = "false";
	  std::string stdoutput;
	  std::string stderror;
	  int exit_status = 0;
	  Glib::spawn_command_line_sync (command,
	     &stdoutput, &stderror, &exit_status);
	  //exit_status &= 255;
	  if (((exit_status == 2560) && !sat) ||
	      ((exit_status == 5120) && sat) ||
	      (exit_status != 2560 && exit_status != 5120))
	  {
	    delete module;
	    std::ofstream file ("input.sf");
	    file << input;
	    
	    std::cerr << "Test is failed: exit:" << exit_status << std::endl;
	    return;
	  }
	  std::cout << "exit:" << exit_status << std::endl;
	  delete module;
	}
}

int
main (int argc, char** argv)
{
  Glib::init ();
  
  guint bits_n;
  
  if (argc < 3)
  {
    std::cout << "spear-force-test BITS SATSOLVER" << std::endl;
    return 0;
  }
  
  {
    std::istringstream is (argv[1]);
    is >> bits_n;
    if (is.fail ())
    {
      std::cerr << "Cant parse BITS" << std::endl;
      return 1;
    }
  }
  
  try
  {
    do_tests (argv[2], bits_n);
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return 1;
  }
  
  return 0;
}
