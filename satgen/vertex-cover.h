/*
 * vertex-cover.h - vertex cover module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_VERTEX_COVER_H__
#define __SATGEN_VERTEX_COVER_H__

#include "module.h"
#include "parse-graph.h"

using namespace SatUtils;

class VertexCoverModule: public SatGenModule
{
public:
  
private:
  guint cover_size;
  
  guint vertices_n;
  std::vector<std::string> vertex_names;
  EdgeVector edges;
  
  VertexCoverModule ();
  void post_process_params ();
public:
  ~VertexCoverModule ();
  
  static SatGenModule* create ();
  
  void parse_input (const std::string& input);
  void generate (CNF& cnf, std::string& outmap_string, bool with_outmap) const;
};

#endif /* __SATGEN_VERTEX_COVER_H__ */
