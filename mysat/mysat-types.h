/*
 * mysat-types.h -
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_MYSAT_TYPES_H__
#define __MYSAT_MYSAT_TYPES_H__

#include <vector>
#include <glibmm.h>

enum Result
{
  UNSATISFIABLE = 0,
  SATISFIABLE,
  UNKNOWN
};

enum
{
  ASSIGN_FALSE = 2,
  ASSIGN_TRUE = 1,
  ASSIGN_NONE = 0,
  ASSIGN_MASK = 3,
};

struct AssignLiteral
{
  guchar assign;

  AssignLiteral () : assign (ASSIGN_NONE)
  { }
  AssignLiteral (bool v)
  { *this = v; }
  bool is_false () const
  { return (assign & ASSIGN_FALSE); }
  bool is_true () const
  { return (assign & ASSIGN_TRUE); }

  AssignLiteral& operator= (bool v)
  {
    assign = (assign & ~ASSIGN_MASK) | ((v) ? ASSIGN_TRUE : ASSIGN_FALSE);
    return *this;
  }

  AssignLiteral& unassign ()
  {
    assign &= ~ASSIGN_MASK;
    return *this;
  }

  bool operator! () const
  { return (assign & ASSIGN_FALSE); }
  bool operator== (bool v) const
  { return (v && (assign & ASSIGN_TRUE)) || (!v && (assign & ASSIGN_FALSE)); }
  bool operator!= (bool v) const
  { return !(*this != v); }
  bool assigned () const
  { return (assign & ASSIGN_MASK) != 0; }

  AssignLiteral operator-() const

  {
    AssignLiteral l = *this;
    l = ((l.assign & ASSIGN_FALSE) >> 1) | ((l.assign & ASSIGN_TRUE) << 1);
    return l;
  }
};

class AssignVector: public std::vector<AssignLiteral>
{
public:
  AssignVector()
  { }
  AssignVector(int n) : std::vector<AssignLiteral> (n)
  { }
  AssignVector(int n, const AssignLiteral& c) :
      std::vector<AssignLiteral> (n, c)
  { }
  template<typename Iter>
  AssignVector(Iter first, Iter last) :
      std::vector<AssignLiteral> (first, last)
  { }

  void set(gint32 lit)
  {
    if (lit >= 0)
      (*this)[lit-1] = true;
    else
      (*this)[-lit-1] = false;
  }
  /* get true if literal set as true */
  bool get(gint32 lit)
  {
    return (lit >= 0) ? (((*this)[lit-1].assign & ASSIGN_TRUE) != 0) :
      (((*this)[-lit-1].assign & ASSIGN_FALSE) != 0);
  }
};

typedef AssignVector::const_iterator AssignConstIter;
typedef AssignVector::iterator AssignIter;

typedef std::vector<gint32> ImplVector;
typedef ImplVector::iterator ImplIter;
typedef ImplVector::const_iterator ImplConstIter;

#endif /* MYSATTYPES_H_ */
