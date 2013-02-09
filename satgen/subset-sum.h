/*
 * subset-sum.h - subset sum module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_SUBSET_SUM_H__
#define __SATGEN_SUBSET_SUM_H__

#include "module.h"

using namespace SatUtils;

class SubsetSumModule: public SatGenModule
{
private:
  gint64 result_value; /* result of subset sum */
  
  std::vector<gint64> numbers;
  
  SubsetSumModule ();
  void post_process_params ();
public:
  ~SubsetSumModule ();
  
  static SatGenModule* create ();
  
  void parse_input (const std::string& input);
  void generate (CNF& cnf, std::string& outmap_string, bool with_outmap) const;
};

#endif /* __SATGEN_SUBSET_SUM_H__ */
