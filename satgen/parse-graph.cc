/*
 * parse-graph.cc - parse-graph module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <cctype>
#include <map>
#include "parse-graph.h"

void parse_graph (const std::string& input,
    std::vector<std::string>& vertex_names, EdgeVector& edges)
{
  std::map<std::string, guint32> vertices;
  
  edges.clear ();
  vertex_names.clear ();
  guint vertices_n = 0;
  
  std::string::const_iterator input_it;
  input_it = input.begin ();
  
  /* if previous vertex is defined */
  bool is_edge = true; /* first define it edge is exist for first edge parsing */
  bool is_prev = false;
  guint32 prev = 0;
  
  guint lineno = 1;
  
  while (1)
  {
    bool do_stop = false;
    std::string name;
    while (1)
    {
      while (input_it != input.end () && isspace (*input_it))
      {
        if (*input_it == '\n')
          lineno++;
        input_it++;
      }
      if (input_it == input.end ())
      {
        do_stop = true;
        break;
      }
      else
        break;
    }
    if (do_stop) /* if end of stream */
    {
      if (is_prev) /* if connect parsed but end of edge is not */
        throw InputSyntaxError (lineno, "Edge must be connected with vertex.");
      else if (!is_edge) /* parsed vertex but is not edge */
        throw InputSyntaxError (lineno, "Single vertices is not accepted.");
      break;
    }
    if (*input_it != '-')
    {
      while (input_it != input.end () && (isalnum (*input_it) || *input_it == '_'))
        name.push_back (*input_it++);
      
      if (name.size () == 0)
        throw InputSyntaxError (lineno, "Bad name of vertex.");
      
      guint32 current;
      if (vertices.find (name) == vertices.end ())
      {
	current = vertices_n;
	vertex_names.push_back (name);
	vertices[name] = vertices_n++;
      }
      else
	current = vertices[name];
      if (is_prev) /* if edge */
      {
        edges.push_back (Edge (prev, current));
        is_prev = false; /* disable previous */
      }
      else
      {
        if (!is_edge)
          throw InputSyntaxError (lineno, "Single vertices is not accepted.");
        is_edge = false;
      }
      prev = current;
    }
    else /* if connector '-' */
    {
      is_prev = true; /* enable previous: connect with previous */
      is_edge = true;
      ++input_it;
    }
  }
}
