/*
 * dpll.cc - simple DPLL algorithm
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <assert.h>
#include <algorithm>
#include <stack>
#include <glibmm.h>
#include "dpll.h"
#include "bcp.h"

DPLLModule::DPLLModule ()
{
}

DPLLModule::~DPLLModule ()
{
}

MySatModule*
DPLLModule::create ()
{
  return static_cast<MySatModule*>(new DPLLModule ());
}

void
DPLLModule::post_process_params ()
{
}


guint32
DPLLModule::choose_variable(const AssignVector& assigns, guint32 curlit)
{
  for (; curlit < assigns.size () && assigns[curlit].assigned (); curlit++);
  return curlit;
}

void
DPLLModule::fetch_problem (const CNF& cnf)
{
  this->cnf = cnf;
}

Result
DPLLModule::solve (std::vector<bool>& model)
{
  std::stack<gint32> decision_stack;
  /* variable of literal for current decision */
  guint32 curlit = 0;

  AssignVector assigns (cnf.get_vars_n ());
  BoolConstraintPropagate bcp (cnf, assigns);

  Result main_result = UNSATISFIABLE;
  if (bcp.is_before_unsatisfiable ())
    return UNSATISFIABLE;

  if (!bcp.empty_queue())
  {
    if (bcp.do_propagate (0) == UNSATISFIABLE)
      return UNSATISFIABLE;
  }

  curlit = choose_variable (assigns, curlit);

  if (curlit == assigns.size ())
    /* if all variables is set */
    main_result = SATISFIABLE;
  else
    decision_stack.push (curlit+1);

  guint64 decision_count = 0;

  while (!decision_stack.empty ())
  {
    gint32 lit = decision_stack.top ();

    assert (!assigns[lit2var (lit)].assigned ());

    if (lit != 0)
    {
      Result result = bcp.do_propagate (lit);

      ++decision_count;
      if ((decision_count & 0xffff) == 0)
      {
	std::ostringstream os;
	os << "Decisions: " << decision_count << ", assigned: " << bcp.get_assigned_n() <<
	  ", depth: " << decision_stack.size () << " ";
	/*guint maxdepth = std::min(curlit, 100U);
	for (guint i = 0; i < maxdepth; i++)
	  os << ((assigns[i].assign == ASSIGN_TRUE) ? '1' : '0');*/
	signal_progress_def.emit (os.str ());
      }

      decision_stack.top () = (lit > 0) ? -lit : 0;
      if (result == SATISFIABLE)
      {
	if (bcp.get_assigned_n() == cnf.get_vars_n ())
	{
	  main_result = SATISFIABLE;
	  break;
	}
	curlit = choose_variable (assigns, curlit);

	decision_stack.push (curlit+1);
      }
      /* if unsatisfiable then undo decision when is not last combination */
      else if (lit > 0)
        bcp.undo_propagate ();
    }
    else
    {
      while (!decision_stack.empty ())
      {
	lit = decision_stack.top ();
	bcp.undo_propagate ();
	if (lit != 0)
	  break;
	else /* pop if 0 and 1 is unsatisfiable */
	  decision_stack.pop ();
      }

      if (!decision_stack.empty ())
        curlit = lit2var (decision_stack.top ());
      else
	curlit = 0;
    }
  }

  {
    std::ostringstream os;
    os << "Decisions: " << decision_count;
    signal_progress_def.emit (os.str ());
  }

  if (main_result == SATISFIABLE)
  {
    model.resize (cnf.get_vars_n ());
    for (guint i = 0; i < assigns.size (); i++)
      model[i] = (assigns[i] == true);
  }

  return main_result;
}
