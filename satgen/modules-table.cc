/*
 * modules-table.cc - satgen modules table
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include "modules-table.h"
#include "eternity.h"
#include "graph-coloring.h"
#include "subset-sum.h"
#include "npuzzle.h"
#include "vertex-cover.h"
#include "set-cover.h"
#include "sokoban.h"
#include "spear-format.h"

const SatGenModuleInfo modules_info_table[] =
{
  {
    "eternity",
    "Eternity Puzzle problem generator",
    EternityModule::create
  },
  {
    "graph-coloring",
    "Graph coloring problem generator",
    GraphColoringModule::create
  },
  {
    "subset-sum",
    "Subset sum problem generator",
    SubsetSumModule::create
  },
  {
    "npuzzle",
    "NPuzzle problem generator",
    NPuzzleModule::create
  },
  {
    "vertex-cover",
    "Vertex cover problem generator",
    VertexCoverModule::create
  },
  {
    "set-cover",
    "Set cover problem generator",
    SetCoverModule::create
  },
  {
      "sokoban",
      "Sokoban problem generator",
      SokobanModule::create
  },
  {
    "spear-format",
    "Spear Format Converter",
    SpearFormatModule::create
  }
};
