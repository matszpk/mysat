/*
 * graph-coloring-test.cc - ....
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <vector>
#include "graph-coloring.h"

using namespace SatUtils;

//typedef GraphColoringModule::Edge Edge;

/* parsing test */

struct Graph
{
  guint vertices_n;
  guint edges_n;
  const char** names;
  const Edge* edges;
};

struct TestGraph
{
  const char* input;
  Graph graph;
};

static bool
compare_graph (const GraphColoringModule* module, const Graph& graph)
{
  const EdgeVector& edges = module->get_edges ();
  if (module->get_vertices_n () != graph.vertices_n ||
      edges.size () != graph.edges_n)
    return false;
  const std::vector<std::string>& names = module->get_vertex_names ();
  for (guint i = 0; i < graph.vertices_n; i++)
    if (names[i] != graph.names[i])
      return false;
  for (guint i = 0; i < graph.edges_n; i++)
    if (edges[i] != graph.edges[i])
      return false;
  return true;
}

static bool
graph_parsing_test ()
{
  std::cout << "Testing good graph parsing" << std::endl;
  
  const char* input1 =
  "a1-b2\n"
  "-b3 b3-a1\n";
  
  static const char* vertex_names_1[] =
  { "a1", "b2", "b3" };
  static const Edge edges_1[] =
  { Edge (0, 1), Edge (1, 2), Edge (2, 0), };
  
  const char* input2 =
  "ax_a-b_x x-ax_a x-d d-ax_a-x\n"
  "d-e e-x\n";
  
  static const char* vertex_names_2[] =
  { "ax_a", "b_x", "x", "d", "e" };
  static const Edge edges_2[] =
  {
    Edge (0, 1), Edge (2, 0), Edge (2, 3), Edge (3, 0), Edge (0, 2),
    Edge (3, 4), Edge (4, 2)
  };
  
  const char* input3 =
  "p1-p5 p1-p6 p2-p6 p2-p8 p3-p4 p3-p7 p3-p8 p4-p6 p4-p7 p5-p6 p5-p8";
  
  static const char* vertex_names_3[] =
  { "p1", "p5", "p6", "p2", "p8", "p3", "p4", "p7" };
  static const Edge edges_3[] =
  {
    Edge (0, 1), Edge (0, 2), Edge (3, 2), Edge (3, 4),
    Edge (5, 6), Edge (5, 7), Edge (5, 4),
    Edge (6, 2), Edge (6, 7), Edge (1, 2), Edge (1, 4)
  };
  
  const char* input4 =
  "p3-p1-p2-p4-p5-p6-p3\np1-p4 \n p2-p6";
  
  static const char* vertex_names_4[] =
  { "p3", "p1", "p2", "p4", "p5", "p6" };
  static const Edge edges_4[] =
  {
    Edge (0, 1), Edge (1, 2), Edge (2, 3), Edge (3, 4), Edge (4, 5), Edge (5, 0),
    Edge (1, 3), Edge (2, 5)
  };
  
  static TestGraph test_graphs[4] =
  {
    {
      input1,
      { 3, 3, vertex_names_1, edges_1 },
    },
    {
      input2,
      { 5, 7, vertex_names_2, edges_2 },
    },
    {
      input3,
      { 8, 11, vertex_names_3, edges_3 },
    },
    {
      input4,
      { 6, 8, vertex_names_4, edges_4 },
    },
  };
  static const guint test_graphs_n = sizeof (test_graphs)/sizeof (TestGraph);
  
  try
  {
    GraphColoringModule* module = static_cast<GraphColoringModule*>
        (GraphColoringModule::create ());
    
    for (guint i = 0; i < test_graphs_n; i++)
    {
      std::cout << "Parsing " << i << " graph" << std::endl;
      module->parse_input (test_graphs[i].input);
      if (!compare_graph (module, test_graphs[i].graph))
      {
	delete module;
	return false;
      }
    }
    
    delete module;
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return false;
  }
  
  std::cout << "Testing bad graph parsing" << std::endl;
  
  static const char* bad_inputs[4] =
  {
    "ax-bx cx dx",
    "axa-bx-cx dx-",
    "axb-gd gd-ax$",
    "axa axa",
  };
  
  {
    GraphColoringModule* module = static_cast<GraphColoringModule*>
        (GraphColoringModule::create ());
    
    for (guint i = 0; i < 4; i++)
    {
      std::cout << "Parsing " << i << " bad input" << std::endl;
      bool bad = false;
      try
      {
        module->parse_input (bad_inputs[i]);
      }
      catch (Glib::Exception& ex)
      {
        std::cout << ex.what () << std::endl;
        bad = true;
      }
      if (!bad)
      {
        delete module;
        return false;
      }
    }
    delete module;
  }
  
  return true;
}

static bool
gen_graph_coloring_test ()
{
  return true;
}

/* main function */

int
main (int argc, char** argv)
{
  Glib::init ();
  
  if (!graph_parsing_test ())
  {
    std::cerr << "Graph parsing is FAILED" << std::endl;
    return 1;
  }
  if (!gen_graph_coloring_test ())
  {
    std::cerr << "Graph formulae generating is FAILED" << std::endl;
    return 1;
  }
  
  return 0;
}
