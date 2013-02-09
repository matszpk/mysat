/*
 * set-cover-test.cc - ....
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <vector>
#include "set-cover.h"

struct SubSets
{
  guint sets_n;
  guint elems_n;
  const char** set_names;
  const guint32* sets_with_elem_tbl;
};

bool
compare_subsets (const SetCoverModule* module, const SubSets& subsets)
{
  if (subsets.sets_n != module->get_sets_n () ||
      subsets.elems_n != module->get_elems_n ())
    return false;
  const std::vector<std::string>& set_names = module->get_set_names ();
  for (guint i = 0; i < subsets.sets_n; i++)
    if (subsets.set_names[i] != set_names[i])
      return false;
  const SetsWithElemVector& elems = module->get_sets_with_elem_table ();
  if (elems.size () != subsets.elems_n)
    return false;
  guint tblindex = 0;
  for (guint i = 0; i < subsets.elems_n; i++)
  {
    guint sets_n_with_elem = subsets.sets_with_elem_tbl[tblindex++];
    if (sets_n_with_elem != elems[i].size ())
      return false;
    for (guint j = 0; j < sets_n_with_elem; j++)
      if (subsets.sets_with_elem_tbl[tblindex+j] != elems[i][j])
        return false;
    tblindex += sets_n_with_elem;
  }
  return true;
}

bool
set_parse_test ()
{
  static const char* text_input1 =
  "set1 = { aa bb c d }\n"
  "set2 = { bb d e g }\n"
  "set3 = { e hi k }\n"
  "set4 = { aa d k }\n"
  "set5 = { bb hi g k }\n";
  
  static const char* set_names1[] =
  { "set1", "set2", "set3", "set4", "set5" };
  
  static const guint32 sets_with_elem_tbl1[] =
  { /* size, sets with current element */
    2, 0, 3,  /* aa */
    3, 0, 1, 4, /* bb */
    1, 0, /* c */
    3, 0, 1, 3, /* d */
    2, 1, 2, /* e */
    2, 1, 4, /* g */
    2, 2, 4, /* hi */
    3, 2, 3, 4 /* k */
  };
  
  static const char* text_input2 =
  "p = { dde b x a }\n"
  "d = { x a c }\n"
  "x = { dde c ff }\n"
  "r = { ff a b }\n";
  
  static const char* set_names2[] =
  { "p", "d", "x", "r" };
  
  static const guint32 sets_with_elem_tbl2[] =
  { /* size, sets with current element */
    2, 0, 2, /* dde */
    2, 0, 3, /* b */
    2, 0, 1, /* x */
    3, 0, 1, 3, /* a */
    2, 1, 2, /* c */
    2, 2, 3, /* ff */
  };
  
  static const char* text_input3 =
  "p = { dde b x a }\n"
  "d = { x a c x }\n"
  "x = { dde c ff }\n"
  "r = { ff a b ff }\n";
  
  static const char* text_inputs_tbl[] =
  {
    text_input1, text_input2, text_input3
  };
  
  static const SubSets subsets_tbl[3] =
  {
    { 5, 8, set_names1, sets_with_elem_tbl1 },
    { 4, 6, set_names2, sets_with_elem_tbl2 },
    { 4, 6, set_names2, sets_with_elem_tbl2 },
  };
  
  static const guint subsets_tbl_size = sizeof (subsets_tbl)/sizeof (SubSets);
  
  try
  {
    SetCoverModule* module = static_cast<SetCoverModule*>
        (SetCoverModule::create ());
    
    for (guint i = 0; i < subsets_tbl_size; i++)
    {
      std::cout << "Parsing " << i << " subsets" << std::endl;
      module->parse_input (text_inputs_tbl[i]);
      if (!compare_subsets (module, subsets_tbl[i]))
      {
	delete module;
	return false;
      }
    }
    
    delete module;
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return false;
  }
  
  return true;
}



int
main (int argc, char** argv)
{
  if (!set_parse_test ())
  {
    std::cerr << "Set parsing test is FAILED!" << std::endl;
    return 1;
  }
  return 0;
}
