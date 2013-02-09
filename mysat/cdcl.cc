/*
 * cdcl.cc - Conflict Driven Clause Learning Code
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <assert.h>
#include <vector>
#include <sstream>
#include <glibmm.h>
#include "cdcl.h"

/* CDCL implementation */

CDCLModule::CDCLModule ()
    : learn_factor (0.2), learn_start(0), restart_strategy_string ("minisat"),
      restart_strategy (RESTARTS_MINISAT),
      varlist (0), sorted_varlist (0), watched (0)
{
  add_param ("restarts", "specify restart strategy (minisat,rsat,picosat)",
      restart_strategy_string);
  add_param ("learn_fac", "specify learn clause number factor", learn_factor);
  add_param ("learn_start", "specify learn max clauses max number", learn_start);
}

CDCLModule::~CDCLModule ()
{
  delete []varlist;
  delete []watched;
}

MySatModule*
CDCLModule::create ()
{
  return static_cast<MySatModule*>(new CDCLModule ());
}

void
CDCLModule::post_process_params ()
{
  if (restart_strategy_string == "minisat")
    restart_strategy = RESTARTS_MINISAT;
  else if (restart_strategy_string == "rsat")
    restart_strategy = RESTARTS_RSAT;
  else if (restart_strategy_string == "picosat")
    restart_strategy = RESTARTS_PICOSAT;
  else
    throw ModuleError (ModuleError::BAD_VALUE, "Unknown restart strategy");
}

void
CDCLModule::fetch_problem (const CNF& cnf)
{
  //this->cnf = cnf;
  guint32 idx = 0;
  guint32 clauses_n = cnf.get_clauses_n ();
  const LiteralVector& form = cnf.get_formulae ();

  orig_clauses.resize (clauses_n);
  for (guint i = 0; i < clauses_n; i++)
  {
    gint32 cl_size = form[idx];
    ClausePtr ptr = create_clause (cl_size, form.begin () + idx + 1);
    orig_clauses[i] = ptr;
    idx += cl_size + 1;
    orig_literals_n += cl_size;
  }
  vars_n = cnf.get_vars_n ();
}

Result
CDCLModule::initialize ()
{
  //LiteralVector& form = cnf.get_formulae ();

  decision_level = 0;
  //vars_n = cnf.get_vars_n ();
  current_conflict = CLAUSE_NULL;
  //orig_clauses_n = orig_clauses.size ();
  max_conflicts_n = 100;
  //max_conflicts_n = 51200000;
  //max_learned = 256;
  //max_learned = (orig_clauses.size () * learn_factor);
  //max_learned = 2560000;
  restarts_n = 0;
  //learned_n = 0;
  learned_lits_n = 0;
  removed_literals_n = 0;
  /* first initialize */
  delete []varlist;
  varlist = new Variable[vars_n];
  delete []sorted_varlist;
  sorted_varlist = new ScoredVar[vars_n];
  //scored_learned.clear ();

  //orig_clauses.clear ();
  learned_clauses.clear ();

  used_vars_n = vars_n;

  after_first_reduce = false;
  score_inc = 1.0;
  cl_score_inc = 1.0;

  delete []watched;
  watched = new WatchedList[vars_n<<1];

  impl_queue.clear ();
  assign_indices.clear ();
  assign_stack.clear ();

  for (guint32 i = 0; i < vars_n; i++)
    sorted_varlist[i].var = i;

  {
    //for (guint i = 0; i < cnf.get_clauses_n (); i++)
    for (guint i = 0; i < orig_clauses.size (); i++)
    {
      ClauseRef ref = cref (orig_clauses[i]);
      //gint32 clause_size = form[idx + 1];
      if (ref.size () >= 2)
	add_to_twl (orig_clauses[i]);
      else if (ref.size () == 1) /* unit clause */
      {
	gint32 lit = ref[0];
	guint32 value_lit = value (lit);
	if (value_lit == 0)
	{
	  impl_queue.push_back (Implicate (orig_clauses[i], lit));
	  assign (orig_clauses[i], lit);
	}
	else if (value_lit == FLAGS_FALSE)
	  return UNSATISFIABLE;
      }
      /* initializing scoring (VMTF) */
      for (guint32 j = 0; j < ref.size (); j++)
      {
	gint32 lit = ref[j];
	sorted_varlist[lit2var (lit)].increment (lit, score_inc);
      }
      //idx += clause_size + 2;
    }
  }
  /* sort sorted_varlist */
  std::sort (sorted_varlist, sorted_varlist + vars_n);
  for (guint32 i = 0; i < vars_n; i++)
    varlist[sorted_varlist[i].var].sorted = i;

  for (sorted_highest = vars_n; sorted_highest > 0; sorted_highest--)
    if (!varlist[sorted_varlist[sorted_highest-1].var].assigned ())
      break;
  sorted_highest--;

  return SATISFIABLE;
}

/*
 * Boolean Constraints Propagation.
 * return - satisfiable if no conflicts, of unsat if conflict clause
 */

Result
CDCLModule::do_propagate (gint32 literal)
{
  //LiteralVector& form = cnf.get_formulae ();

  if (literal != 0)
  {
    impl_queue.push_back (Implicate (CLAUSE_NULL, literal));
    assign (CLAUSE_NULL, literal);
  }

  /* clauses in form:
   * { zeroed_literal, implication_literal, others literals }
   */
  while (!impl_queue.empty ())
  {
    Implicate impl_elem = impl_queue.front ();
    impl_queue.pop_front ();

    WatchedList& wc = lit2w (-impl_elem.lit);
    //assert (istrue (impl_elem.lit));

    guint32 ii = 0;
    for (guint32 i = 0; i < wc.size (); i++)
    {
      //guint32 clidx = wc[i];
      ClausePtr clauseptr = wc[i];
      ClauseRef clause = cref (clauseptr);

      guint32 cl_size = clause.size ();
      //clidx += 2;

      if (-impl_elem.lit != clause[1])
        std::swap (clause[0], clause[1]);

      //assert (-impl_elem.lit == clause[1]);
      bool if_newwatch = false;

      if (istrue (clause[0]))
	wc[ii++] = wc[i];
      else
      {
	for (guint j = 2; j < cl_size; j++)
	{
	  gint32 lit = clause[j];
	  if (istrue (-lit))
	    continue; /* skip false's */
	  else
	  { /* if other literal is free or true then is new watch */
	    lit2w (lit).push_back (clauseptr);
	    std::swap (clause[1], clause[j]);
	    if_newwatch = true;
	    break;
	  }
	}

	if (!if_newwatch)
	{
	  wc[ii++] = wc[i];
	  gint32 implit = clause[0];
	  gint32 implit_val = value (implit);
	  if (implit_val == FLAGS_FALSE)
	  { /* if false */
	    /*assert (assigns.get (-form[clidx]) ||
		!assigns[lit2var (form[clidx])].assigned ()); */
	    /* if remaining clauses to copy */
	    for (i++ ; i < wc.size (); i++, ii++)
	      wc[ii] = wc[i];
	    impl_queue.clear ();
	    wc.resize (ii);
	    current_conflict = clauseptr;
	    return UNSATISFIABLE;
	  }
	  else if (implit_val == 0)
	  { /* if free */
	    //std::cout << "To queue: " << implit << ", clidx: " << clidx << std::endl;
	    assign (clauseptr, implit);
	    impl_queue.push_back (Implicate (clauseptr, implit));
	  }
	  /* otherwise is satisfiable clause */
	}
      }
    }
    wc.resize (ii);
  }

  return SATISFIABLE;
}

/*
 * backtracking routine -
 */

void
CDCLModule::undo_propagate (guint32 level)
{
  if (level == decision_level)
    return;
  guint32 end = assign_indices[level+1];
  for (guint32 i = end; i < assign_stack.size (); i++)
  {
    Variable& v = getvar (assign_stack[i].lit);
    v.unassign ();
    sorted_highest = std::max (v.sorted, sorted_highest);
  }
  assign_indices.resize (level+1);
  assign_stack.resize (end);
  decision_level = level;
}

/*
 * derivate_impls - derivate implicates and add assert clause to formulae
 */

guint32
CDCLModule::derivate_impls (gint32& uip)
{
  LiteralVector aclause; /* asserting clause */
  ClausePtr clauseptr = current_conflict;
  std::vector<guint32> used_indices;

  /* check for assigned in 0 level */
  /*for (guint32 i = 0; i < vars_n; i++)
    if (varlist[i].assigned () && varlist[i].level == 0)
      assert (varlist[i].is_used ());
    else
      assert (!varlist[i].is_used ());*/

  guint32 impls_n = 0;
  guint32 pos = assign_stack.size ()-1; /* stack pos */

  guint32 shallowest_level = 0;

  bool to_rescale = false;

//#if 0
  while (true)
  {
    if (clauseptr != CLAUSE_NULL)
    {
      ClauseRef clause = cref (clauseptr);

      gint32 cl_size = clause.size ();

      if (clause.learned ())
        increment_clause_score (clauseptr);

      for (gint32 j = 0; j < cl_size; j++)
      {
	guint32 vidx = lit2var (clause[j]);
	if (!varlist[vidx].is_used ())
	{
	  assert (varlist[vidx].level != 0);
	  {
	    guint32 idx = varlist[vidx].sorted;
	    sorted_varlist[idx].increment (clause[j], score_inc);
	    if (sorted_varlist[idx].value () > score_limit)
	      to_rescale = true;

	    for (; idx + 1 < vars_n && sorted_varlist[idx+1] < sorted_varlist[idx]; idx++)
	    {
	      std::swap (sorted_varlist[idx], sorted_varlist[idx+1]);
	      varlist[sorted_varlist[idx].var].sorted = idx;
	    }
	    varlist[sorted_varlist[idx].var].sorted = idx;
	  }

	  varlist[vidx].use ();
	  used_indices.push_back (vidx);
	  if (varlist[vidx].level < decision_level)
	  {
	    aclause.push_back (clause[j]);
	    shallowest_level = std::max (varlist[vidx].level, shallowest_level);
	  }
	  else
	    impls_n++;
	}
      }
    }

    gint32 lit = assign_stack[pos].lit;
    Variable& v = getvar (lit);

    if (v.is_used ())
    { /* to resolve */
      if (impls_n > 1)
      {
	clauseptr = assign_stack[pos].clause;
        impls_n--;
      }
      else
      {
	aclause.push_back (-lit);
	impls_n = 0;
	/*if (pos != assign_indices.back ())
	{
	  std::cout << "Bad: " << pos << ", " << assign_indices.back () << std::endl;
	  assert (pos == assign_indices.back ());
	}*/
        break;
      }
    }
    else
      clauseptr = CLAUSE_NULL;
      //clidx = NO_CLAUSES;
    if (pos == assign_indices.back ())
      break;
    pos--;
    assert (pos >= assign_indices.back ());
  }
//#endif

#if 0
  gint32 impl = 0;
  do {
    ClauseRef clause = cref (clauseptr);

    gint32 cl_size = clause.size ();

    if (clause.learned ())
      increment_clause_score (clauseptr);

    for (gint32 j = (impl != 0) ? 1 : 0; j < cl_size; j++)
    {
      guint32 vidx = lit2var (clause[j]);
      if (!varlist[vidx].is_used ())
      {
	assert (varlist[vidx].level != 0);
	{
	  guint32 idx = varlist[vidx].sorted;
	  sorted_varlist[idx].increment (clause[j], score_inc);
	  if (sorted_varlist[idx].value () > score_limit)
	    to_rescale = true;

	  for (; idx + 1 < vars_n && sorted_varlist[idx+1] < sorted_varlist[idx]; idx++)
	  {
	    std::swap (sorted_varlist[idx], sorted_varlist[idx+1]);
	    varlist[sorted_varlist[idx].var].sorted = idx;
	  }
	  varlist[sorted_varlist[idx].var].sorted = idx;
	}

	varlist[vidx].use ();
	used_indices.push_back (vidx);
	if (varlist[vidx].level < decision_level)
	{
	  aclause.push_back (clause[j]);
	  shallowest_level = std::max (varlist[vidx].level, shallowest_level);
	}
	else
	  impls_n++;
      }
    }

    while (!varlist[lit2var (assign_stack[pos--].lit)].is_used ());
    impl = assign_stack[pos+1].lit;
    clauseptr = assign_stack[pos+1].clause;
    getvar (impl).unuse ();
    impls_n--;

  } while (impls_n > 0);

  aclause.push_back (-impl);
#endif

  if (impls_n != 0)
  {
    std::cout << "Bad impls_n: " << impls_n << std::endl;
    assert (impls_n == 0);
  }

  if (aclause.size () >= 2)
    shallowest_level = minimize_clause (aclause, used_indices);

  for (std::vector<guint32>::const_iterator it = used_indices.begin ();
	it != used_indices.end (); ++it)
    varlist[*it].unuse ();

  /* set UIP */
  uip = aclause.back ();

  /*for (guint i = 0; i < aclause.size ()-1; i++)
    assert (varlist[lit2var (aclause[i])].level < decision_level);*/

  if (aclause.size () >= 2)
  {
    std::swap (aclause[0], aclause.back ());
    ClausePtr newclauseptr = create_clause (aclause.size (), aclause.begin (), true);
    /* adding assert clause */
    //if (aclause.size () >= 2)
      /* put last literal to properly location on clause */
      add_to_twl (newclauseptr);

    /* add to scored list and to CNF */
    ClauseRef newclause = cref (newclauseptr);
    learned_clauses.push_back (newclauseptr);
    newclause.score += cl_score_inc;

    learned_lits_n += aclause.size ();
  }

  /* check if good */
  /*for (guint32 i = 0; i < vars_n-1; i++)
  {
    assert (sorted_varlist[i] <= sorted_varlist[i + 1]);
    assert (i == varlist[sorted_varlist[i].var].sorted);
  }*/

  if (to_rescale)
    rescale_vars_scores ();

  score_inc *= score_inc_factor;
  cl_score_inc *= cl_score_inc_factor;


  //assert (varlist[lit2var (uip)].level == decision_level);

  //learned_n++;
  return shallowest_level;
}

/*
 * minimize clause (based on MiniSat solution)
 */
guint32
CDCLModule::minimize_clause (LiteralVector& aclause, std::vector<guint32>& used_indices)
{
  guint32 shallowest_level;
  //gint32 uip = aclause.back ();
  guint32 used_levels = 0;
  //std::vector<bool> used_levels (decision_level);
  //std::fill (used_levels.begin (), used_levels.end (), false);
  for (LiteralConstIter it = aclause.begin (); it != aclause.end (); ++it)
    used_levels |= (1ULL << (varlist[lit2var (*it)].level & 31));
    //used_levels[varlist[lit2var (*it)].level] = true;

  guint ii = 0;
  for (guint i = 0; i < aclause.size (); i++)
  {
    bool to_remove = false;
    guint32 firstvidx = lit2var (aclause[i]);

    if (varlist[firstvidx].level != decision_level)
    { /* check if this literal (if removable if not) */
      std::stack<guint32> impl_stack;
      impl_stack.push (firstvidx);

      to_remove = true;
      guint32 first_used_index_in_search = used_indices.size ();

      while (!impl_stack.empty ())
      {
	guint32 vidx = impl_stack.top ();
	impl_stack.pop ();

	guint32 pos_stack = varlist[vidx].pos_stack;
	if (varlist[vidx].level != 0 && assign_stack[pos_stack].clause == CLAUSE_NULL)
	{ /* if decision assign */
	  assert (decision_level > varlist[vidx].level);
	  to_remove = false;
	  break;
	}
	else if (assign_stack[pos_stack].clause == CLAUSE_NULL)
	{
	  assert (varlist[vidx].is_used ());
	  continue;
	}

	ClausePtr clauseptr = assign_stack[pos_stack].clause;
	ClauseRef clause = cref (clauseptr);
	guint32 cl_size = clause.size ();

	for (guint32 k = 0; k < cl_size; k++)
	{
	  guint32 vidx2 = lit2var (clause[k]);
	  /* if not marked then analyze literal, */
	  if (!varlist[vidx2].is_used () && varlist[vidx2].level != 0)
	  {
	    assert (varlist[vidx2].level != 0);
	    if (varlist[vidx2].assigned () &&
		//(used_levels[varlist[vidx2].level])
		((1ULL << (varlist[vidx2].level & 31)) & used_levels) != 0)
	    { /* if this path is not visited */
	      assert (varlist[vidx2].assigned ());
	      impl_stack.push (vidx2);
	      varlist[vidx2].use ();
	      used_indices.push_back (vidx2);
	    }
	    else
	    { /* if literals is not part of assert clause */
	      if (!(varlist[vidx2].level != 0 && !varlist[vidx2].is_used ()))
	      {
		std::cout << "Error:" << varlist[vidx2].level << ',' <<
		    varlist[vidx2].is_used () << ',' << varlist[vidx2].assigned () << std::endl;
		assert (false);
	      }
	      to_remove = false;
	      break;
	    }
	  }
	}
      }
      if (!to_remove) /* not to remove */
      {
	for (guint l = first_used_index_in_search; l < used_indices.size (); l++)
	  varlist[used_indices[l]].unuse ();
	used_indices.resize (first_used_index_in_search);
      }
      else
	removed_literals_n++;
    }

    if (!to_remove)
      aclause[ii++] = aclause[i];
  }
  aclause.resize (ii);

  /* recompute compute shallowest level */
  shallowest_level = 0;
  LiteralConstIter end = --aclause.end ();

  for (LiteralConstIter it = aclause.begin (); it != end; ++it)
    shallowest_level = std::max (shallowest_level, varlist[lit2var (*it)].level);

  //assert (aclause.back () == uip);

  return shallowest_level;
}


gint32
CDCLModule::choose_literal ()
{
  for (sorted_highest = vars_n; sorted_highest > 0; sorted_highest--)
    if (!varlist[sorted_varlist[sorted_highest-1].var].assigned ())
      break;
  sorted_highest--;

  ScoredVar& sv = sorted_varlist[sorted_highest];
  //gint32 lit = (sv.pos_score > sv.neg_score) ? sv.var + 1 : -sv.var - 1;
  gint32 lit = -sv.var - 1;
  /*if (do_rand)
    lit = (rand () & 1) ? -sv.var - 1 : sv.var + 1;*/

  assert (!varlist[sorted_varlist[sorted_highest].var].assigned ());
  return lit;
}

/*
 * rescale scores
 */
void
CDCLModule::rescale_vars_scores ()
{
  for (guint i = 0; i < vars_n; i++)
    sorted_varlist[i] *= score_divider;
  score_inc *= score_divider;
}


void
CDCLModule::rescale_clauses_score ()
{
  for (guint i = 0; i < learned_clauses.size (); i++)
  {
    ClauseRef ref = cref (learned_clauses[i]);
    ref.score *= cl_score_divider;
  }
  /*
  for (ScoredClauseIter it = scored_learned.begin (); it != scored_learned.end (); ++it)
    (*it) *= cl_score_divider;
  */
  cl_score_inc *= cl_score_divider;
}

/*
 * reduce learned clauses by removing with smallest score
 */

void
CDCLModule::reduce_learned ()
{
  ClauseLessFunctor lessfunc (*this);
  std::sort (learned_clauses.begin (), learned_clauses.end (), lessfunc);
  /*for (guint i = 0; i < learned_n; i++)
    form[scored_learned[i].idx] = i;*/

  guint32 to_erase_idx = learned_clauses.size () >> 1;
  //std::vector<AssignElem>::const_iterator cur_assigned = assigned_clauses.begin ();

  //double litsperclause = double (learned_lits_n) / double (learned_clauses.size ());
  double score_limit = double (cl_score_inc) / (learned_clauses.size ());

  //guint clidx = orig_end;
  //guint32 newclidx = orig_end;
  guint32 i = 0;
  guint32 ii = 0;
  for (i = 0; i < to_erase_idx; i++)
  {
    ClausePtr clauseptr = learned_clauses[i];
    ClauseRef clause = cref (clauseptr);

    bool to_remove = false;
    if (clause.size () > 2)
    {
      guint32 posstack = varlist[lit2var (clause[0])].pos_stack;
      /* if resolved by propagation */
      if (posstack >= assign_stack.size () || assign_stack[posstack].clause != clauseptr)
	to_remove = true;
    }
    if (!to_remove)
      learned_clauses[ii++] = clauseptr;
    else
    {
      learned_lits_n -= clause.size ();
      delete_from_twl (clauseptr);
      delete_clause (clauseptr);
    }
  }
  for (; i < learned_clauses.size (); i++)
  {
    ClausePtr clauseptr = learned_clauses[i];
    ClauseRef clause = cref (clauseptr);

    bool to_remove = false;
    if (clause.size () > 2)
    {
      guint32 posstack = varlist[lit2var (clause[0])].pos_stack;
      /* if resolved by propagation */
      if ((posstack >= assign_stack.size () || assign_stack[posstack].clause != clauseptr)
	  && (/*double (clause.size ()) >= litsperclause*1.333 ||*/
	    clause.score < score_limit))
	to_remove = true;
    }
    if (!to_remove)
      learned_clauses[ii++] = clauseptr;
    else
    {
      learned_lits_n -= clause.size ();
      delete_from_twl (clauseptr);
      delete_clause (clauseptr);
    }
  }

  learned_clauses.resize (ii);
  after_first_reduce = true;
}

/*
 * simplifying learned clauses and original formulae
 */

Result
CDCLModule::simplify_formulae ()
{
  Result prop_result;

  /* after this propagate (BCP) */
  prop_result = do_propagate (0);
  if (prop_result == UNSATISFIABLE)
    return UNSATISFIABLE;

  std::vector<bool> varscount (vars_n);
  std::fill (varscount.begin (), varscount.end (), false);

  /* delete satisfied clauses from original formulae */
  guint32 ii = 0;
  for (guint32 i = 0; i < orig_clauses.size (); i++)
  {
    ClausePtr clauseptr = orig_clauses[i];
    ClauseRef clause = cref (clauseptr);

    bool satisfied = false;
    guint32 cl_size = clause.size ();
    for (guint j = 0; j < cl_size; j++)
      if (istrue (clause[j]))
      {
	satisfied = true;
	break;
      }

    if (satisfied)
    { /* to remove */
      if (cl_size >= 2)
        delete_from_twl (clauseptr);
      delete_clause (clauseptr);
      orig_literals_n -= cl_size;
    }
    else
    { /* counting vars */
      for (guint j = 0; j < cl_size; j++)
	if (value (clause[j]) == 0)
	  varscount[lit2var (clause[j])] = true;
      orig_clauses[ii++] = orig_clauses[i];
    }
  }
  orig_clauses.resize (ii);

  /* delete satisfied learned clauses */
  ii = 0;
  for (guint32 i = 0; i < learned_clauses.size (); i++)
  {
    ClausePtr clauseptr = learned_clauses[i];
    ClauseRef clause = cref (clauseptr);

    bool satisfied = false;
    guint32 cl_size = clause.size ();
    for (guint j = 0; j < cl_size; j++)
      if (istrue (clause[j]))
      {
	satisfied = true;
	break;
      }

    if (satisfied)
    { /* to remove */
      learned_lits_n -= clause.size ();
      if (cl_size >= 2)
        delete_from_twl (clauseptr);
      delete_clause (clauseptr);
    }
    else
    {
      for (guint j = 0; j < cl_size; j++)
	if (value (clause[j]) == 0)
	  varscount[lit2var (clause[j])] = true;
      learned_clauses[ii++] = learned_clauses[i];
    }
  }
  learned_clauses.resize (ii);
  guint32 applied = 0;
  used_vars_n = 0;
  for (guint i = 0; i < vars_n; i++)
    if (varscount[i])
    {
      if (!varlist[i].assigned ())
        used_vars_n++;
      else
	assert (false);
    }
    else if (!varlist[i].assigned ())
    {
      applied++;
      assign (CLAUSE_NULL, i+1);
      impl_queue.push_back (Implicate (CLAUSE_NULL, i+1));
    }

  /* sort sorted_varlist */
  std::sort (sorted_varlist, sorted_varlist + vars_n);
  for (guint32 i = 0; i < vars_n; i++)
    varlist[sorted_varlist[i].var].sorted = i;

  for (sorted_highest = vars_n; sorted_highest > 0; sorted_highest--)
    if (!varlist[sorted_varlist[sorted_highest-1].var].assigned ())
      break;
  sorted_highest--;

  return prop_result;
}

void
CDCLModule::report_progress (bool newrestart) const
{
  std::ostringstream os;
  char c = (newrestart) ? 'N' : '-';
  os << '|' << c <<
	std::setw (3) << restarts_n << '|' <<
	std::setw (5) << decision_level << '|' <<
	std::setw (9) << decision_count << '|' <<
	std::setw (8) << conflict_count << '|' <<
	std::setw (6) << used_vars_n << '|' <<
	std::setw (8) << orig_clauses.size () << '|' <<
	std::setw (8) << orig_literals_n << '|' <<
	std::setw (8) << learned_clauses.size () << '|' <<
	std::setw (8) << learned_lits_n << '|' <<
	std::setprecision (1) << std::fixed <<
	(double (learned_lits_n) / double (learned_clauses.size ())) << '|' <<
	std::setw (8) << max_learned << '|' <<
	/*std::setw (8) << removed_literals_n << '|'*/
	std::setw (8) << max_conflicts_n << '|';
  signal_progress_def.emit (os.str ());
}

/*
 * main solve routine
 */

Result
CDCLModule::solve (std::vector<bool>& model)
{
  if (initialize () == UNSATISFIABLE)
    return UNSATISFIABLE;

  assign_indices.push_back (0);

  decision_count = 0;
  conflict_count = 0;

  signal_progress_def.emit (
	"--------------------------------------------------------------------------------------------------");
  signal_progress_def.emit (
	"| Rst|Level|Decisions|Conflict|  Vars| Clauses|Literals|Learned |LearnLts| LPC| LearnMax|ConflMax|");
  signal_progress_def.emit (
	"--------------------------------------------------------------------------------------------------");

  Result prop_result;
  gint32 choosen = 0;

  guint32 initial_factor = 1;

  guint32 outer = 400;
  if (restart_strategy == RESTARTS_PICOSAT)
    max_conflicts_n = 400;

  double my_factor = 1.0;

  //prop_result = do_propagate (choosen);
  {
    prop_result = simplify_formulae ();
    if (learn_start == 0)
      max_learned = (orig_clauses.size () * learn_factor);
    else
      max_learned = learn_start;
  }

  if (!(prop_result == UNSATISFIABLE || assign_stack.size () == vars_n))
  {
  while (true)
  {
    if (decision_level != 0)
      prop_result = do_propagate (choosen);

    if (prop_result == SATISFIABLE)
    {
      if (max_learned-assign_stack.size () <= learned_clauses.size ())
      {
	reduce_learned ();
	if (restart_strategy == RESTARTS_RSAT)
	  max_learned += (max_learned / 10);
      }

      if (assign_stack.size () != vars_n)
      {
	choosen = choose_literal ();
	assign_indices.push_back (assign_stack.size ());
	decision_level++;
	decision_count++;
	if ((decision_count & 0xfff) == 0)
	  report_progress (false);
      }
      else
	break;
    }
    else
    {
      conflict_count++;
      //local_conflict_count++;
      if (decision_level != 0)
      {
	if (conflict_count < max_conflicts_n)
	{
	  gint32 uip;
	  guint32 undo_level = derivate_impls (uip);
	  undo_propagate (undo_level);

	  ClausePtr clauseptr = (decision_level != 0) ? learned_clauses.back () : CLAUSE_NULL;
	  //ClauseRef clause = cref (clauseptr);

	  assign (clauseptr, uip);
	  impl_queue.push_back (Implicate (clauseptr, uip)); /* for finish BCP */
	  choosen = 0;
	  if (decision_level == 0)
	  {
	    prop_result = simplify_formulae ();
	    if (prop_result == UNSATISFIABLE || assign_stack.size () == vars_n)
	      break;
	  }
	}
	else
	{
	  undo_propagate (0); /* to back track */
	  prop_result = simplify_formulae ();
	  if (prop_result == UNSATISFIABLE || assign_stack.size () == vars_n)
	    break;

	  conflict_count = 0;
	  restarts_n++;
	  if (restart_strategy == RESTARTS_MINISAT)
	  {
	    /*if (after_first_reduce)*/
	    max_learned += (max_learned / 10);
	    report_progress (true);

	    //max_conflicts_n += guint32 (double (max_conflicts_n) / 2);
	    max_conflicts_n += (max_conflicts_n >> 1);
	  }
	  else if (restart_strategy == RESTARTS_RSAT)
	  {
	    guint32 factor = initial_factor;
	    for (; (factor & restarts_n) == 0; factor <<= 1);

	    /*if (initial_factor == restarts_n && restarts_n != 1)
	      initial_factor <<= 1;*/
	    //restarts_n += factor;

	    max_conflicts_n = 512 * factor;
	    report_progress (true);
	    //max_learned = double (orig_clauses.size ()) * learn_factor * factor;
	  }
	  else if (restart_strategy == RESTARTS_PICOSAT)
	  {
	    if (max_conflicts_n >= outer)
	    {
	      outer += outer / 10;
	      max_conflicts_n = double (orig_clauses.size ()) * learn_factor * my_factor;
	      max_learned = double (orig_clauses.size ()) * learn_factor * my_factor;
	      my_factor *= 1.03;
	      report_progress (true);
	    }
	    else
	    {
	      report_progress (true);
	      max_conflicts_n += max_conflicts_n / 5;
	      max_learned += max_learned / 5;
	    }
	  }

	  choosen = 0;
	}
      }
      else
	break;
    }
  }
  }

  report_progress (false);

  if (prop_result == SATISFIABLE)
  {
    model.resize (vars_n);
    for (guint i = 0; i < vars_n; i++)
      model[i] = varlist[i].get ();
  }

  return prop_result;
}
