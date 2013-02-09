/*
 * bcp.cc - Boolean Constraint Propagation
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <assert.h>
#include <iostream>
#include <glibmm.h>
#include "bcp.h"

using namespace SatUtils;

BoolConstraintPropagate::BoolConstraintPropagate (CNF& incnf, AssignVector& inassigns)
    : cnf (incnf), assigns (inassigns), decision_level (0), before_unsatisfiable (false)
{
  gint32 vars_n = cnf.get_vars_n ();
  LiteralVector& form = cnf.get_formulae ();

  watched.resize ((vars_n << 1) + 2);
  assigned_n = 0;

  guint idx = 0;
  for (guint32 i = 0; i < cnf.get_clauses_n (); i++)
  {
    if (form[idx] >= 2)
    {
      guint32 wi = lit2wi (form[idx+1]);
      watched[wi].push_back (idx);
      wi = lit2wi (form[idx+2]);
      watched[wi].push_back (idx);
    }
    else
    {
      gint32 lit = form[idx+1];
      if (assigns.get (-lit))
	before_unsatisfiable = true;
      else if (!assigns[lit2var (lit)].assigned ())
      {
	queue.push_back (lit);
	assigned_n++;
	assigns.set (lit);
	implicates.push_back (lit);
      }
    }

    idx += form[idx] + 1;
    //impl_indices.push_back (0);
  }
}

Result
BoolConstraintPropagate::do_propagate (gint32 literal)
{
  //std::cout << "Propagate " << literal << std::endl;

  LiteralVector& form = cnf.get_formulae ();

  if (queue.empty ())
    impl_indices.push_back (implicates.size ());
  else /* if first propagation */
    impl_indices.push_back (0);
  decision_level++;

  if (literal != 0)
  {
    queue.push_back (literal);
    assigned_n++;
    assigns.set (literal);
    implicates.push_back (literal);
  }
  /* clauses in form:
   * { zeroed_literal, implication_literal, others literals }
   */
  while (!queue.empty ())
  {
    gint32 firstlit = queue.front ();
    queue.pop_front ();

    WatchedList& wc = watched[lit2wi (-firstlit)];

    if (assigns[lit2var (firstlit)].assigned ())
    {
      if (assigns.get (firstlit) == false)
      {
	assert(false);
	queue.clear ();
	return UNSATISFIABLE;
      }
      /*else
	continue;*/
    }
    else
      assert(false);

    guint ii = 0;

    for (guint i = 0; i < wc.size (); i++)
    {
      guint32 clidx = wc[i];

      guint32 cl_size = form[clidx];
      clidx++;

      if (-firstlit != form[clidx])
        std::swap (form[clidx], form[clidx+1]);

      //std::cout << "clidx: " << clidx << " firstlit: " << firstlit << std::endl;
      assert (-firstlit == form[clidx]);
      bool if_newwatch = false;

      if (assigns.get (form[clidx+1])) /* if true */
      {
	//std::cout << "Is true. clidx: " << clidx << std::endl;
	wc[ii++] = wc[i];
      }
      else
      {
	for (guint j = 2; j < cl_size; j++)
	{
	  gint32 lit = form[clidx+j];
	  if (assigns.get (-lit)) /* if false literal */
	    continue; /* skip false's */
	  else
	  { /* if other literal is free or true then is new watch */
	    /*std::cout << "New watch: " << lit << ", clidx: " << clidx <<
		",j: " << j << " old: " << form[clidx] << std::endl;*/
	    guint32 wlit = lit2wi (lit);
	    watched[wlit].push_back (clidx-1);
	    std::swap (form[clidx], form[clidx+j]);
	    if_newwatch = true;
	    break;
	  }
	}

	if (!if_newwatch)
	{
	  wc[ii++] = wc[i];
	  gint32 implit = form[clidx+1];
	  if (assigns.get (-implit))
	  {
	    assert (assigns.get (-form[clidx]) || !assigns[lit2var (form[clidx])].assigned ());
	    /* if remaining clauses to copy */
	    for (i++ ; i < wc.size (); i++, ii++)
	      wc[ii] = wc[i];
	    queue.clear ();
	    wc.resize (ii);
	    return UNSATISFIABLE;
	  }
	  else if (!assigns[lit2var (implit)].assigned ())
	  {
	    //std::cout << "To queue: " << implit << ", clidx: " << clidx << std::endl;
	    queue.push_back (implit);
	    assigns.set (implit);
	    assigned_n++;
	    implicates.push_back (implit);
	  }
	  /* otherwise is satisfiable clause */
	}
      }
      /* otherwise current clause skip it */
    }

    /* resizing of wc */
    wc.resize (ii);
  }

  return SATISFIABLE;
}

void
BoolConstraintPropagate::undo_propagate (guint levels)
{
  decision_level -= levels;
  for (guint32 i = impl_indices[decision_level]; i < implicates.size (); i++)
    assigns[lit2var (implicates[i])].unassign ();

  implicates.resize (impl_indices[decision_level]);
  impl_indices.resize (decision_level);
  assigned_n = implicates.size ();
}
