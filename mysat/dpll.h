/*
 * dpll.h - simple DPLL algorithm
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_DPLL_H__
#define __MYSAT_DPLL_H__

#include <glibmm.h>
#include "module.h"

class DPLLModule: public MySatModule
{
private:
  CNF cnf;

  DPLLModule ();
  void post_process_params ();

  guint32 choose_variable(const AssignVector& assigns, guint32 curlit);
public:
  static MySatModule* create ();

  ~DPLLModule ();

  void fetch_problem (const CNF& cnf);
  Result solve (std::vector<bool>& model);
};

#endif /* __MYSAT_DPLL_H__ */
