/*
 * set-cover.cc - set cover module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <cctype>
#include <map>
#include "gen-arith.h"
#include "set-cover.h"

using namespace SatUtils;

SetCoverModule::SetCoverModule ()
    : cover_size (3)
{
  add_param ("cover_size", "Cover size", cover_size);
}

SetCoverModule::~SetCoverModule ()
{
}

SatGenModule*
SetCoverModule::create ()
{
  return static_cast<SatGenModule*>(new SetCoverModule ());
}

void
SetCoverModule::post_process_params ()
{
  if (cover_size == 0)
    throw ModuleError (ModuleError::BAD_VALUE,
          "Cover size must be greater than 0.");
}

void
SetCoverModule::parse_input (const std::string& input)
{
  std::map<std::string, guint32> elems;

  sets_with_elem_table.clear ();
  set_names.clear ();
  
  sets_n = 0;
  elems_n = 0;
  std::string::const_iterator input_it = input.begin ();
  
  guint lineno = 1;
  
  while (1)
  {
    /* skip spaces */
    while (input_it != input.end () && isspace (*input_it))
    {
      if (*input_it == '\n') lineno++;
      input_it++;
    }
    if (input_it == input.end ())
      break; /* if end */
    
    std::string setname;
    if (isalnum (*input_it) || *input_it == '_')
    {
      while (input_it != input.end () && (isalnum (*input_it) || *input_it == '_'))
        setname.push_back (*input_it++);
    }
    else
      throw InputSyntaxError (lineno, "Name of set must be specified");
    
    guint32 current_set = set_names.size ();
    set_names.push_back (setname);
    
    /* skip spaces */
    while (input_it != input.end () && isspace (*input_it))
    {
      if (*input_it == '\n') lineno++;
      input_it++;
    }
    
    if (input_it != input.end () && *input_it == '=')
      input_it++;
    else
      throw InputSyntaxError (lineno, "Expected '=' character");
    
    /* skip spaces */
    while (input_it != input.end () && isspace (*input_it))
    {
      if (*input_it == '\n') lineno++;
      input_it++;
    }
    
    if (input_it != input.end () && *input_it == '{')
      input_it++;
    else
      throw InputSyntaxError (lineno, "Expected '{' character");
    
    /* parsing elements of set */
    while (1)
    {
      /* skip spaces */
      while (input_it != input.end () && isspace (*input_it))
      {
        if (*input_it == '\n') lineno++;
	input_it++;
      }
      
      if (input_it != input.end ())
      {
        std::string elemname;
        if (isalnum (*input_it) || *input_it == '_')
        {
          while (input_it != input.end () && (isalnum (*input_it) ||
                *input_it == '_'))
            elemname.push_back (*input_it++);
        }
        else if (*input_it == '}')
        {
          ++input_it;
          break;
        }
        else
          throw InputSyntaxError (lineno, "Expected '}' or element name");
        
        /* check if found elem name */
        guint32 current;
        if (elems.find (elemname) == elems.end ())
        {
          sets_with_elem_table.resize (elems_n+1);
          current = elems_n;
          elems[elemname] = elems_n++;
        }
        else
          current = elems[elemname];
        
	/* add if current set haven't current element */
	if (sets_with_elem_table[current].size () == 0 ||
	      *(--sets_with_elem_table[current].end ()) != current_set)
	  sets_with_elem_table[current].push_back (current_set);
      }
      else
        throw InputSyntaxError (lineno, "Expected '}' or element name");
    }
  }
  
  sets_n = set_names.size ();
}

void
SetCoverModule::generate (CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  guint cover_size_bits_n = 0;
  for (guint v = 1; v <= cover_size; v <<= 1, cover_size_bits_n++);
  
  gint32 set_vars = cnf.next_var ();
  cnf.add_vars (sets_n);
  
  if (with_outmap)
  {
    outmap_string.clear ();
    for (guint i = 0; i < sets_n; i++)
    {
      std::ostringstream os;
      os << "{ {|" << set_names[i] << "\n}: " << set_vars + i << '}';
      outmap_string += os.str ();
    }
  }
  
  for (SetsWithElemConstIter elem = sets_with_elem_table.begin ();
       elem != sets_with_elem_table.end (); ++elem)
  {
    LiteralVector clause (elem->size ());
    for (guint i = 0; i < elem->size (); i++)
      clause[i] = (*elem)[i] + set_vars;
    cnf.add_clause (clause);
  }
  
  LiteralVector out = cnf.add_vars_with_literals (cover_size_bits_n);
  LiteralMatrix dadda_matrix (cover_size_bits_n);
  /* prepare dadda adder matrix */
  add_literals_from_range (dadda_matrix[0], set_vars, set_vars + sets_n);
  /* with dadda with carry where carry must be zero */
  cnf_gen_dadda_with_carry (cnf, out, dadda_matrix, true, 0, DEF_NEGATIVE);
  /* must be equal with cover size */
  cnf_gen_equal (cnf, 0, out, cover_size, DEF_POSITIVE);
}
