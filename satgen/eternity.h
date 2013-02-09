/*
 * eternity.h
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_ETERNITY_H__
#define __SATGEN_ETERNITY_H__

#include "module.h"
#include "libsokobie.h"

using namespace SatUtils;
using namespace Sokobie;

class EternityModule: public SatGenModule
{
private:
  guint64 board_size;
  std::vector<guchar> board_data;

  EternityModule ();
  void post_process_params();
public:
  ~EternityModule();

  static SatGenModule* create();

  void parse_input(const std::string& input);
  void generate(CNF& cnf, std::string& outmap_string, bool with_outmap) const;
};

#endif /* ETERNITY_H_ */
