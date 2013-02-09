/*
 * parse-graph.h - parsing graph routine
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_PARSE_GRAPH_H__
#define __SATGEN_PARSE_GRAPH_H__

#include <string>
#include <vector>
#include "module.h"

typedef std::pair<guint32, guint32> Edge;
typedef std::vector<Edge> EdgeVector;
typedef std::vector<Edge>::iterator EdgeIter;
typedef std::vector<Edge>::const_iterator EdgeConstIter;

void parse_graph (const std::string& input,
    std::vector<std::string>& vertex_names, EdgeVector& edges);

#endif /* __SATGEN_PARSE_GRAPH_H__ */
