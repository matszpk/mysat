/*
 * modules-table.cc - modules-table
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <glibmm.h>
#include "modules-table.h"
#include "dpll.h"
#include "cdcl.h"

const MySatModuleInfo modules_info_table[] =
{
  {
    "dpll",
    "Simple DPLL algorithm",
    DPLLModule::create
  },
  {
    "cdcl",
    "Conflict Driven Clause Learning algorithm",
    CDCLModule::create
  }
};
