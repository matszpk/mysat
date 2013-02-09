/*
 * modules-table.h - satgen modules table
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_MODULES_TABLE_H__
#define __SATGEN_MODULES_TABLE_H__

#include "module.h"

typedef SatGenModule* (*SatGenModuleCreateCallback) ();

struct SatGenModuleInfo
{
  std::string name;
  std::string description;
  SatGenModuleCreateCallback create;
};

static const guint modules_info_table_size = 8;

extern const SatGenModuleInfo modules_info_table[];


#endif /* __SATGEN_MODULES_TABLE_H__ */
