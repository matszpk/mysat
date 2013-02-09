/*
 * satmap.cc - mapping sat results to values and outputs values
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glibmm.h>
#include <satutils.h>

static bool
parse_dimacs_output (std::istream& file, std::vector<bool>& instance, bool& sat)
{
  while (!file.eof ())
  {
    std::string line;
    std::getline (file, line);
    if (line.size () == 0 || line[0] == 'c') /* if empty line */
      continue;
    
    std::istringstream is (line);
    char c = is.get ();
    if (c == 's')
    {
      std::string str;
      is >> str;
      if (str == "SATISFIABLE")
        sat = true;
      else if (str == "UNSATISFIABLE")
        sat = false;
    }
    else if (c == 'v')
    {
      sat = true;
      while (!is.eof ())
      {
        gint32 value, varnum;
        is >> value;
        if (is.fail ())
        {
          std::cerr << "Bad literal in instance line" << std::endl;
          return false;
        }
        if (value != 0)
        {
	  varnum = std::abs (value);
	  if (instance.size () < guint (varnum))
	    instance.resize (varnum);
	  instance[varnum-1] = (value > 0) ? true : false;
        }
      }
    }
    else
    {
      std::cerr << "Bad format of output" << std::endl;
      return false;
    }
  }
  return true;
}

bool
print_output_based_on_outmap (std::istream& file,
      const std::vector<bool>& instance, bool sat)
{
  if (!sat)
  {
    std::cout << "Formulae is unsatisfiable!" << std::endl;
    return true;
  }
  if (instance.size () == 0)
  {
    std::cout << "Formulae is satisfiable." << std::endl;
    return true;
  }
  
  while (1)
  {
    char c;
    c = file.get ();
    if (file.eof ())
      break;
    if (c == '\\')
    {
      c = file.get ();
      if (file.eof ())
        break;
      std::cout << c;
      continue;
    }
    else if (c == '{')
    { /* conversion */
      std::vector<std::string> choices;
      char conversion;
      file >> conversion;
      if (conversion != 'u' && conversion != 's' &&
          conversion != 'x' && conversion != 'o' && conversion != '{')
      {
        std::cerr << "Bad conversion type" << std::endl;
        return false;
      }
      
      if (conversion == '{')
      {
        bool closed = false;
        guint choice = 0;
        choices.resize (1);
        while (1)
        {
          char c;
          c = file.get ();
          if (file.eof ())
            break;
          if (c == '\\')
          {
            c = file.get ();
            if (file.eof ())
              break;
            choices[choice].push_back (c);
            continue;
          }
          else if (c == '}')
          {
            closed = true;
            break;
          }
          else if (c == '|')
          {
            choice++;
            choices.resize (choices.size () + 1);
          }
          else
            choices[choice].push_back (c);
        }
        if (!closed)
        {
          std::cerr << "Braces is not closed" << std::endl;
          return false;
        }
      }
      
      file >> c;
      if (c != ':')
      {
        std::cerr << "Colon is required after conversion type" << std::endl;
        return false;
      }
      
      guint64 value = 0;
      guint bit;
      for (bit = 0; bit < 65; bit++)
      {
        file >> c;
        if (file.eof ())
        {
          std::cerr << "Not closed brace at end file" << std::endl;
          return false;
        }
        if (c == '}') /* if end of conversion */
          break;
        file.unget ();
        
        gint32 literal;
        file >> literal;
        if (file.fail ())
        {
          std::cerr << "Bad literal in conversion" << std::endl;
          return false;
        }
        if (literal == 0)
        {
          std::cerr << "Bad literal must be nonzero" << std::endl;
          return false;
        }
        gint32 valnum = std::abs (literal);
        if (guint32 (valnum) <= instance.size ())
          value |= (instance[valnum-1] == (literal > 0)) ? (1ULL << bit) : 0;
      }
      
      if (bit == 65)
      {
        std::cerr <<
          "Only 64 literals is possible to use in conversion" << std::endl;
        return false;
      }
      
      if (bit != 0)
      {
	if (conversion == 's')
	{ /* expand sign */
	  if (value & (1ULL << (bit-1)))
	    value |= (G_MAXUINT64 ^ ((1ULL << bit)-1));
	}
	
	switch (conversion)
	{
	  case 'u':
	    std::cout << std::dec << guint64 (value);
	    break;
	  case 's':
	    std::cout << std::dec << gint64 (value);
	    break;
	  case 'x':
	    std::cout << std::hex << guint64 (value);
	    break;
	  case 'o':
	    std::cout << std::oct << guint64 (value);
	    break;
	  case '{':
	    if (value < choices.size ())
	      std::cout << choices[value];
	    else
	      std::cout << "{!!!OUT OF RANGE!!!:" << value << '}';
	    break;
	  default:
	    break;
	}
      }
    }
    else
      std::cout << c;
  }
  std::cout.flush ();
  return true;
}

/* main function */

int
main (int argc, char** argv)
{
  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "C");

  Glib::init ();
  
  if (argc < 2)
  {
    std::cout << "satmap OUTMAP [DIMACSOUTPUT]" << std::endl;
    return 0;
  }
  
  bool satisfiable;
  std::vector<bool> instance;
  
  try
  {
    /* parse dimacs output */
    if (argc >= 3)
    {
      std::ifstream file (argv[2], std::ios::in);
      if (!file)
      {
	std::cerr << "Open error: " << argv[2];
	return 1;
      }
      file.exceptions (std::ios_base::badbit);
      if (!parse_dimacs_output (file, instance, satisfiable))
	return 1;
    }
    else
    {
      std::cin.exceptions (std::ios_base::badbit);
      if (!parse_dimacs_output (std::cin, instance, satisfiable))
	return 1;
    }
    
    {
      std::ifstream file (argv[1], std::ios::in);
      if (!file)
      {
	std::cerr << "Open error: " << argv[1];
	return 1;
      }
      file.exceptions (std::ios_base::badbit);
      if (!print_output_based_on_outmap (file, instance, satisfiable))
        return 1;
    }
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return 1;
  }
  return 0;
}
