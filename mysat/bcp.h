/*
 * bcp.h - Boolean Constraint Propagation
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_BCP_H__
#define __MYSAT_BCP_H__

#include <vector>
#include <deque>
#include <satutils.h>
#include "mysat-types.h"

using namespace SatUtils;

inline guint32 lit2wi (gint32 l)
{
  return (l >= 0) ? (l-1)<<1 : ((-l-1)<<1) + 1;
}

inline gint32 lit2var (gint32 l)
{
  return (l >= 0) ? l-1 : -l-1;
}

class BoolConstraintPropagate
{
private:
  CNF& cnf;
  AssignVector& assigns;
  gint32 assigned_n;
  guint decision_level; /* decision decision_level */
  std::vector<guint32> impl_indices;
  ImplVector implicates;

  typedef std::vector<guint32> WatchedList;

  std::vector<WatchedList> watched;

  std::deque<gint32> queue;

  guint32 conflict;

  bool before_unsatisfiable;

public:
  BoolConstraintPropagate (CNF& cnf, AssignVector& inassigns);

  Result do_propagate (gint32 literal);

  guint get_level () const
  { return decision_level; }
  void undo_propagate (guint levels = 1);

  bool empty_queue () const
  { return queue.empty (); }
  gint32 get_assigned_n () const
  { return assigned_n; }

  guint32 conflict_clause () const
  { return conflict; }

  bool is_before_unsatisfiable () const
  { return before_unsatisfiable; }
};

#endif /* _MYSAT_BCP_H_ */

