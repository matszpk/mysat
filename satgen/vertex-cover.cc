/*
 * vertex-cover.cc - vertex cover module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <cctype>
#include <map>
#include <sstream>
#include "gen-arith.h"
#include "vertex-cover.h"

using namespace SatUtils;

VertexCoverModule::VertexCoverModule ()
    : cover_size (3)
{
  add_param ("cover_size", "Cover size", cover_size);
}

VertexCoverModule::~VertexCoverModule ()
{
}

SatGenModule*
VertexCoverModule::create ()
{
  return static_cast<SatGenModule*>(new VertexCoverModule ());
}

void
VertexCoverModule::post_process_params ()
{
  if (cover_size == 0)
    throw ModuleError (ModuleError::BAD_VALUE,
          "Cover size must be greater than 0.");
}

void
VertexCoverModule::parse_input (const std::string& input)
{
  parse_graph (input, vertex_names, edges);
  vertices_n = vertex_names.size ();
}

void
VertexCoverModule::generate (CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  guint cover_size_bits_n = 0;
  for (guint v = 1; v <= cover_size; v <<= 1, cover_size_bits_n++);
  
  gint32 vertex_vars = cnf.next_var ();
  cnf.add_vars (vertices_n);
  
  if (with_outmap)
  {
    outmap_string.clear ();
    for (guint i = 0; i < vertices_n; i++)
    {
      std::ostringstream os;
      os << "{ {|" << vertex_names[i] << "\n}: " << vertex_vars + i << '}';
      outmap_string += os.str ();
    }
  }
  
  for (EdgeConstIter edge = edges.begin (); edge != edges.end (); ++edge)
    cnf.add_clause (vertex_vars + edge->first, vertex_vars + edge->second);
  
  LiteralVector out = cnf.add_vars_with_literals (cover_size_bits_n);
  LiteralMatrix dadda_matrix (cover_size_bits_n);
  /* prepare dadda adder matrix */
  add_literals_from_range (dadda_matrix[0], vertex_vars, vertex_vars + vertices_n);
  /* with dadda with carry where carry must be zero */
  cnf_gen_dadda_with_carry (cnf, out, dadda_matrix, true, 0, DEF_NEGATIVE);
  /* must be equal with cover size */
  cnf_gen_equal (cnf, 0, out, cover_size, DEF_POSITIVE);
}
