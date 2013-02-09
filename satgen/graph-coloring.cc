/*
 * graph-coloring.cc - graph coloring module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <cctype>
#include <map>
#include <sstream>
#include "gen-arith.h"
#include "graph-coloring.h"

using namespace SatUtils;

GraphColoringModule::GraphColoringModule ()
    : edge_coloring (false), colors_n (2)
{
  add_param ("edges", "Coloring edges instead vertices", edge_coloring);
  add_param ("colors", "Number of colors", colors_n);
}

GraphColoringModule::~GraphColoringModule ()
{
}

SatGenModule*
GraphColoringModule::create ()
{
  return static_cast<SatGenModule*>(new GraphColoringModule ());
}

void
GraphColoringModule::post_process_params ()
{
  if (colors_n == 0)
    throw ModuleError (ModuleError::BAD_VALUE,
          "Number of colors must be greater than 0.");
}

void
GraphColoringModule::parse_input (const std::string& input)
{
  parse_graph (input, vertex_names, edges);
  vertices_n = vertex_names.size ();
}

namespace
{

typedef std::vector<guint> VertexEdges;
typedef VertexEdges::iterator VertexEdgeIter;
typedef VertexEdges::const_iterator VertexEdgeConstIter;
typedef std::vector<VertexEdges> VertexEdgesVector;
typedef VertexEdgesVector::iterator VertexEdgesIter;
typedef VertexEdgesVector::const_iterator VertexEdgesConstIter;

};

void
GraphColoringModule::generate (CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  cnf.clear ();
  guint color_bits_n = 0;
  for (guint v = 1; v < colors_n; v <<= 1, color_bits_n++);
  
  if (!edge_coloring) /* if vertex coloring */
  {
    gint32 start_vertices = cnf.next_var ();
    cnf.add_vars (color_bits_n * vertices_n);
    
    if (with_outmap) /* generating outmap */
    {
      outmap_string.clear ();
      for (guint i = 0; i < vertices_n; i++)
      {
	std::ostringstream os;
	os << vertex_names[i] << " = {u:";
	for (guint v = 0; v < color_bits_n; v++)
	  os << start_vertices + i*color_bits_n+v << ' ';
	os << "}\n";
	outmap_string += os.str ();
      }
    }
    
    /* if required - generate for all vertices v<=maxcolor */
    if ((1U << color_bits_n) != colors_n)
    {
      for (guint i = 0; i < vertices_n; i++)
      {
	LiteralVector v = get_literals_from_range (start_vertices + i*color_bits_n,
	    start_vertices + (i+1)*color_bits_n);
	cnf_gen_lesseq (cnf, 0, v, colors_n-1, DEF_POSITIVE);
      }
    }
    
    /* generate main conditions */
    for (EdgeConstIter it = edges.begin (); it != edges.end (); ++it)
    {
      LiteralVector a =
	  get_literals_from_range (start_vertices + (it->first)*color_bits_n,
	      start_vertices + (it->first + 1)*color_bits_n);
      LiteralVector b =
	  get_literals_from_range (start_vertices + (it->second)*color_bits_n,
	      start_vertices + (it->second + 1)*color_bits_n);
      cnf_gen_equal (cnf, 0, a, b, DEF_NEGATIVE); /* a != b */
    }
  }
  else /* edge coloring */
  {
    VertexEdgesVector vertex_edges (vertices_n);
    
    /* prepare vertex edges */
    for (guint i = 0; i < edges.size (); i++)
    {
      const Edge& edge = edges[i];
      vertex_edges[edge.first].push_back (i);
      if (edge.first != edge.second) /* if ends of edge is not same */
        vertex_edges[edge.second].push_back (i);
    }
    
    gint32 start_edges = cnf.next_var ();
    cnf.add_vars (color_bits_n * edges.size ());
    
    /* if required - generate for all vertices v<=maxcolor */
    if ((1U << color_bits_n) != colors_n)
    {
      for (guint i = 0; i < edges.size (); i++)
      {
	LiteralVector v = get_literals_from_range (start_edges + i*color_bits_n,
	    start_edges + (i+1)*color_bits_n);
	cnf_gen_lesseq (cnf, 0, v, colors_n-1, DEF_POSITIVE);
      }
    }
    
    if (with_outmap) /* generating outmap */
    {
      outmap_string.clear ();
      for (guint i = 0; i < edges.size (); i++)
      {
	std::ostringstream os;
	os << vertex_names[edges[i].first] << '-' <<
	   vertex_names[edges[i].second] << " = {u:";
	for (guint v = 0; v < color_bits_n; v++)
	  os << start_edges + i*color_bits_n+v << ' ';
	os << "}\n";
	outmap_string += os.str ();
      }
    }
    
    /* generate main conditions */
    for (VertexEdgesConstIter it = vertex_edges.begin ();
         it != vertex_edges.end (); ++it)
    {
      guint vertex_edges_n = it->size ();
      for (guint i = 1; i < vertex_edges_n; i++)
      {
        guint edge_a = (*it)[i];
        LiteralVector a =
	    get_literals_from_range (start_edges + (edge_a)*color_bits_n,
		start_edges + (edge_a + 1)*color_bits_n);
        for (guint j = 0; j < i; j++)
        {
          guint edge_b = (*it)[j];
	  LiteralVector b =
	      get_literals_from_range (start_edges + (edge_b)*color_bits_n,
		  start_edges + (edge_b + 1)*color_bits_n);
          cnf_gen_equal (cnf, 0, a, b, DEF_NEGATIVE); /* a != b */
        }
      }
    }
  }
}
