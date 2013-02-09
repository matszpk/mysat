/*
 * cdcl.h - Conflict-Driven Clause Learning module header
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_CDCL_H__
#define __MYSAT_CDCL_H__

#include <assert.h>
#include <deque>
#include <string>
#include <vector>
#include <glibmm.h>
#include "module.h"

#define CLAUSE_NULL (reinterpret_cast<ClausePtr>(0))

class CDCLModule: public MySatModule
{
private:
  //class Clause;

  class Clause
  {
  public:
    typedef Clause &Ref;
    typedef Clause *Ptr;

    double score;
    guint32 n;
    gint32 lits[0];

    Clause () : score (0), n (1)
    { }
    Clause (int lits_n, bool learn = false) : score (0), n (lits_n | ((learn) ? 1 : 0))
    { }

    void init (int lits_n, bool learn = false)
    {
      score = 0;
      n = (lits_n<<1) | ((learn) ? 1 : 0);
    }

    gint32& operator[] (guint32 i)
    { return lits[i]; }
    const gint32& operator[] (guint32 i) const
    { return lits[i]; }

    guint32 size () const
    { return n>>1; }

    void shrink (guint32 lits_n)
    { n -= lits_n<<1; }

    bool learned () const
    { return (n & 1) != 0; }
  };

  typedef Clause::Ref ClauseRef;
  typedef Clause::Ptr ClausePtr;

  ClausePtr create_clause_n (guint32 lits_n, bool learn = false)
  {
    guint32 sz = sizeof (double) + sizeof (gint32) * (lits_n + 1);
    guchar* m = new guchar[sz];

    ClausePtr ptr = (reinterpret_cast<Clause*>(m));
    ptr->init (lits_n, learn);
    return ptr;
  }

  template<typename Iter>
  ClausePtr create_clause (guint32 lits_n, Iter it, bool learn = false)
  {
    ClausePtr ptr = create_clause_n (lits_n, learn);
    ClauseRef ref = cref (ptr);
    for (guint32 i = 0; i < (lits_n); i++)
      ref[i] = it[i];
    return ptr;
  }

  /* clause reference */
  ClauseRef cref (ClausePtr ptr)
  {
    return *ptr;
  }

  void delete_clause (ClausePtr c)
  {
//#if 0
    ClauseRef clause = cref (c);
    //if (clause.size () >= 2)
    {
      /*guint32 implvidx = (clause.size () >= 2) ? lit2var (clause[0]) :
	lit2var (clause[0]);*/
      guint32 implvidx = lit2var (clause[0]);
      guint32 pos_stack = varlist[implvidx].pos_stack;
      /* if pos_stack is good and if clause in assign stack item is c then
       * clause to null in stack item */
      if (pos_stack < assign_stack.size () && assign_stack[pos_stack].clause == c)
	assign_stack[pos_stack].clause = CLAUSE_NULL;
    }
//#endif
    delete[] reinterpret_cast<guchar*>(c);
  }

  void add_to_twl (ClausePtr clauseptr)
  {
    ClauseRef clause = cref (clauseptr);
    lit2w (clause[0]).push_back (clauseptr);
    lit2w (clause[1]).push_back (clauseptr);
  }

  void delete_from_twl (ClausePtr clauseptr)
  {
    ClauseRef clause = cref (clauseptr);
    WatchedList& wc1 = lit2w (clause[0]);
    WatchedList& wc2 = lit2w (clause[1]);
    std::remove (wc1.begin (), wc1.end (), clauseptr);
    std::remove (wc2.begin (), wc2.end (), clauseptr);
    wc1.resize (wc1.size () - 1);
    wc2.resize (wc2.size () - 1);
  }

  struct ClauseLessFunctor
  {
    CDCLModule& cdcl_mod;

    ClauseLessFunctor (CDCLModule& m) : cdcl_mod (m)
    { }

    bool operator() (const ClausePtr& c1, const ClausePtr& c2)
    {
      return cdcl_mod.cref (c1).score < cdcl_mod.cref (c2).score;
    }
  };

  enum {
    FLAGS_UNASSIGN = 0,
    FLAGS_FALSE = 2,
    FLAGS_TRUE = 1,
    FLAGS_VALMASK = 3,
    FLAGS_USED = 4,
  };

  enum {
    NO_CONFLICT = G_MAXUINT32,
    NO_CLAUSES = G_MAXUINT32
  };

  //static const ClausePtr CLAUSE_NULL = reinterpret_cast<const ClausePtr>(0);

  enum RestartStrategy
  {
    RESTARTS_MINISAT,
    RESTARTS_RSAT,
    RESTARTS_PICOSAT
  };

  float learn_factor;
  guint learn_start;
  std::string restart_strategy_string;
  RestartStrategy restart_strategy;

  guint32 vars_n;
  guint32 decision_level;
  ClausePtr current_conflict;
  guint64 decision_count;
  guint64 conflict_count;
  guint64 removed_literals_n;

  //guint32 orig_clauses_n;
  guint32 restarts_n;
  guint64 max_conflicts_n; /* threshold of learned clauses */
  //guint32 learned_n;
  guint32 learned_lits_n;

  guint32 orig_literals_n;
  std::vector<ClausePtr> orig_clauses;
  std::vector<ClausePtr> learned_clauses;

  guint32 used_vars_n;

  /* max learned clauses before reduction knowledge */
  guint64 max_learned;

  /* for literals scoring */
  static const double score_inc_factor = 1.0 / 0.95;
  static const double score_limit = 1.0e100;
  static const double score_divider = 1.0e-100;
  /* for clauses scoring */
  static const double cl_score_inc_factor = 1.0 / 0.999;
  static const double cl_score_limit = 1.0e20;
  static const double cl_score_divider = 1.0e-20;
  double score_inc;
  double cl_score_inc;
  bool after_first_reduce;

  struct Variable
  {
    guint32 flags; /* value and flags */
    guint32 level; /* assign stack position */
    guint32 pos_stack;
    guint32 sorted;

    Variable () : flags (0), level (0), pos_stack (G_MAXUINT32)
    { }

    /*Variable& operator= (bool v)
    {
      flags = (flags & ~FLAGS_VALMASK) | ((v) ? FLAGS_TRUE : FLAGS_FALSE);
      return *this;
    }*/

    bool operator== (bool v) const
    {
      guint32 tocompare = (v) ? FLAGS_TRUE : FLAGS_FALSE;
      return ((flags & tocompare) == 0);
    }
    bool operator!= (bool v) const
    {
      guint32 tocompare = (v) ? FLAGS_TRUE : FLAGS_FALSE;
      return ((flags & tocompare) == 0);
    }

    bool get () const
    { return ((flags & FLAGS_VALMASK) == FLAGS_TRUE); }

    bool assigned () const
    { return ((flags & FLAGS_VALMASK) != 0); }
    void unassign ()
    { flags &= ~FLAGS_VALMASK;
      pos_stack = G_MAXUINT32; }

    /*Variable& setbylit (gint32 lit)
    {
      *this = (lit >= 0);
      return *this;
    }
    bool comparebylit (gint32 lit) const
    { return (*this == (lit >= 0)); }*/
    void use ()
    { flags |= FLAGS_USED; }
    void unuse ()
    { flags &= ~FLAGS_USED; }
    bool is_used () const
    { return ((flags & FLAGS_USED) != 0); }
  };

  Variable* varlist;

  /* scored variable for VMTF */
  struct ScoredVar
  {
    gint32 var;
    double score;

    ScoredVar () : score (0)
    { }

    void increment (gint32 lit, double inc)
    {
      score += inc;
    }
    void decrement (gint32 lit, double inc)
    {
      score -= inc;
    }

    double value () const
    { return score; }

    ScoredVar& operator*= (double fac)
    {
      score *= fac;
      return *this;
    }

    bool operator< (const ScoredVar& var) const
    { return (score) < (var.score); }
    bool operator<= (const ScoredVar& var) const
    { return (score) <= (var.score); }
    bool operator> (const ScoredVar& var) const
    { return (score) > (var.score); }
    bool operator>= (const ScoredVar& var) const
    { return (score) >= (var.score); }
  };
  guint32 sorted_highest;
  ScoredVar* sorted_varlist;

  void rescale_vars_scores ();

  bool istrue (gint32 lit)
  {
    if (lit >= 0)
      return ((varlist[lit-1].flags & FLAGS_TRUE) != 0);
    else
      return ((varlist[-lit-1].flags & FLAGS_FALSE) != 0);
  }
  gint32 value (gint32 lit)
  {
    if (lit >= 0)
      return varlist[lit-1].flags & FLAGS_VALMASK;
    else
      return ((varlist[-lit-1].flags >> 1) & FLAGS_TRUE) |
	((varlist[-lit-1].flags << 1) & FLAGS_FALSE);
  }
  void setlit (gint32 lit)
  {
    guint32 used = (decision_level == 0) ? FLAGS_USED : 0;
    if (lit >= 0)
    {
      Variable& v = varlist[lit-1];
      /*varlist[lit-1].flags = (varlist[lit-1].flags & ~FLAGS_FALSE) | FLAGS_TRUE;
      varlist[lit-1].level = decision_level;
      varlist[lit-1].pos_stack = assign_stack.size ()-1;
      if (decision_level == 0)
	varlist[lit-1].use ();*/
      v.flags = (v.flags & ~FLAGS_FALSE) | FLAGS_TRUE | used;
      v.level = decision_level;
      v.pos_stack = assign_stack.size ()-1;
    }
    else
    {
      Variable& v = varlist[-lit-1];
      /*varlist[-lit-1].flags = (varlist[-lit-1].flags & ~FLAGS_TRUE) | FLAGS_FALSE;
      varlist[-lit-1].level = decision_level;
      varlist[-lit-1].pos_stack = assign_stack.size ()-1;
      if (decision_level == 0)
	varlist[-lit-1].use ();*/
      v.flags = (v.flags & ~FLAGS_TRUE) | FLAGS_FALSE | used;
      v.level = decision_level;
      v.pos_stack = assign_stack.size ()-1;
    }
  }

  Variable& getvar (gint32 lit)
  {
    return (lit >= 0) ? varlist[lit-1] : varlist[-lit-1];
  }

  /* watched list for TWL algorithm */
  typedef std::vector<ClausePtr> WatchedList;
  WatchedList* watched;

  WatchedList& lit2w (gint32 l)
  {
    return (l >= 0) ? watched[l-1] : watched[-l-1 + vars_n];
  }
  guint32 lit2var (gint32 l)
  {
    return std::abs (l)-1;
  }

  struct Implicate
  {
    ClausePtr clause; /* clause index */
    gint32 lit; /* literal */
    Implicate (ClausePtr inclause, gint32 inlit)
        : clause (inclause), lit (inlit)
    { }
  };
  typedef std::deque<Implicate> ImplQueue;
  /* implications queue */
  ImplQueue impl_queue;

  struct Assignment
  {
    ClausePtr clause; /* clause index */
    gint32 lit; /* literal */
    Assignment () { }
    Assignment (ClausePtr inclause, gint32 inlit)
	: clause (inclause), lit (inlit)
    { }
  };

  typedef std::vector<Assignment> AssignStack;
  typedef AssignStack::iterator AssignStackIter;
  typedef AssignStack::const_iterator AssignStackConstIter;
  /* assignments start position for particular decision levels */
  std::vector<guint32> assign_indices;
  /* assignment stack */
  AssignStack assign_stack;

  void assign (ClausePtr clause, gint32 lit)
  {
    //assert (assign_stack.size () < vars_n);
    assign_stack.push_back (Assignment (clause, lit));
    setlit (lit);
  }

  void increment_clause_score (ClausePtr clause)
  {
    ClauseRef ref = cref (clause);
    ref.score += cl_score_inc;
    if (ref.score >= cl_score_limit)
      rescale_clauses_score ();
  }

  void rescale_clauses_score ();

  CDCLModule ();
  void post_process_params ();

  Result initialize ();

  /* make BCP */
  Result do_propagate (gint32 literal);
  /* backtracking */
  void undo_propagate (guint32 level);

  /* derivative implications -> simply generate assertions clause,
   * returns undo decision_level */
  guint32 derivate_impls (gint32& uip);

  /* minimize assert clause: returns good undo decision_level */
  guint32 minimize_clause (LiteralVector& clause, std::vector<guint32>& used_indices);
  /* choose literal */
  gint32 choose_literal ();

  /* reduce_learned clause */
  void reduce_learned ();

  /* simplify formulae after restart */
  Result simplify_formulae ();

  void report_progress (bool newrestart) const;

public:
  static MySatModule* create ();

  ~CDCLModule ();

  void fetch_problem (const CNF& cnf);
  Result solve (std::vector<bool>& model);
};

#endif /* CDCL_H_ */
