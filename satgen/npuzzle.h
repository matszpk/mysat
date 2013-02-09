/*
 * npuzzle.h - NPuzzle problem generator module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_NPUZZLE_H__
#define __SATGEN_NPUZZLE_H__

#include "module.h"

using namespace SatUtils;

class NPuzzleModule: public SatGenModule
{
private:
  bool no_obsolete;
  guint moves_n;
  guint rows_n, cols_n;
  std::vector<guint16> elems;
  
  NPuzzleModule ();
  void post_process_params ();
public:
  ~NPuzzleModule ();
  
  static SatGenModule* create ();
  
  void parse_input (const std::string& input);
  void generate (CNF& cnf, std::string& outmap_string, bool with_outmap) const;
};

#endif /* __SATGEN_NPUZZLE_H__ */
