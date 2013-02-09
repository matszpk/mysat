/*
 * graph-coloring.h - graph coloring module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_GRAPH_COLORING_H__
#define __SATGEN_GRAPH_COLORING_H__

#include "module.h"
#include "parse-graph.h"

using namespace SatUtils;

class GraphColoringModule: public SatGenModule
{
public:
  
private:
  bool edge_coloring;
  guint colors_n;
  
  guint vertices_n;
  std::vector<std::string> vertex_names;
  EdgeVector edges;
  
  GraphColoringModule ();
  void post_process_params ();
public:
  ~GraphColoringModule ();
  
  static SatGenModule* create ();
  
  void parse_input (const std::string& input);
  void generate (CNF& cnf, std::string& outmap_string, bool with_outmap) const;
  
  guint get_colors_n () const
  { return colors_n; }
  guint get_vertices_n () const
  { return vertices_n; }
  const std::vector<std::string>& get_vertex_names () const
  { return vertex_names; }
  const EdgeVector& get_edges () const
  { return edges; }
};

#endif /* __SATGEN_GRAPH_COLORING_H__ */
