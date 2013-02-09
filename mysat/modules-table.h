/*
 * modules-table.h - mysat modules table
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_MODULES_TABLE_H__
#define __MYSAT_MODULES_TABLE_H__

#include "module.h"

typedef MySatModule* (*MySatModuleCreateCallback) ();

struct MySatModuleInfo
{
  std::string name;
  std::string description;
  MySatModuleCreateCallback create;
};

static const guint modules_info_table_size = 2;

extern const MySatModuleInfo modules_info_table[];


#endif /* __SATGEN_MODULES_TABLE_H__ */
