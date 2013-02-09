/*
 * module.h - mysat module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_MODULE_H__
#define __MYSAT_MODULE_H__

#include <glibmm.h>
#include <vector>
#include <satutils.h>
#include "mysat-types.h"

using namespace SatUtils;

class MySatModule: public Module
{
protected:
  MySatModule ();

  sigc::signal<void, const std::string&> signal_progress_def;

public:
  virtual ~MySatModule ();

  virtual void fetch_problem (const CNF& cnf) = 0;
  virtual Result solve (std::vector<bool>& model) = 0;

  sigc::signal<void, const std::string&>& signal_progress ()
  { return signal_progress_def; }
};

#endif /* __MYSAT_MODULE_H__ */
