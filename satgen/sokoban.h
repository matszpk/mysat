/*
 * sokoban.h
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_SOKOBAN_H__
#define __MYSAT_SOKOBAN_H__

#include "module.h"
#include "libsokobie.h"

using namespace SatUtils;
using namespace Sokobie;

class SokobanModule: public SatGenModule
{
private:
  guint32 moves_n; /* result of subset sum */
  guint32 levelno;
  Level level;

  SokobanModule ();
  void post_process_params();
public:
  ~SokobanModule();

  static SatGenModule* create();

  void parse_input(const std::string& input);
  void generate(CNF& cnf, std::string& outmap_string, bool with_outmap) const;
};



#endif /* SOKOBAN_H_ */
