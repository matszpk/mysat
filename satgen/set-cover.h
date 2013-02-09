/*
 * set-cover.h - set cover module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_SET_COVER_H__
#define __SATGEN_SET_COVER_H__

#include <string>
#include <vector>
#include "module.h"

using namespace SatUtils;

typedef std::vector<guint32> SetsWithElem;
typedef std::vector<SetsWithElem> SetsWithElemVector;
typedef SetsWithElemVector::const_iterator SetsWithElemConstIter;
typedef SetsWithElemVector::iterator SetsWithElemIter;

class SetCoverModule: public SatGenModule
{
public:
  
private:
  guint cover_size;
  
  guint sets_n;
  guint elems_n;
  SetsWithElemVector sets_with_elem_table;
  std::vector<std::string> set_names;
  
  SetCoverModule ();
  void post_process_params ();
public:
  ~SetCoverModule ();
  
  static SatGenModule* create ();
  
  void parse_input (const std::string& input);
  void generate (CNF& cnf, std::string& outmap_string, bool with_outmap) const;
  
  guint get_sets_n () const
  { return sets_n; }
  guint get_elems_n () const
  { return elems_n; }
  const SetsWithElemVector& get_sets_with_elem_table () const
  { return sets_with_elem_table; }
  const std::vector<std::string>& get_set_names () const
  { return set_names; }
};

#endif /* __SATGEN_SET_COVER_H__ */
