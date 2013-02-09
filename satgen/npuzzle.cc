/*
 * npuzzle.cc - npuzzle module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <algorithm>
#include <sstream>
#include "npuzzle.h"

using namespace SatUtils;

NPuzzleModule::NPuzzleModule ()
    : no_obsolete (false), moves_n (10)
{
  add_param ("no_obsolete", "Dont permit obsolete moves", no_obsolete);
  add_param ("moves", "Maximum number of moves", moves_n);
}


NPuzzleModule::~NPuzzleModule ()
{
}

SatGenModule*
NPuzzleModule::create ()
{
  return static_cast<SatGenModule*>(new NPuzzleModule);
}

void
NPuzzleModule::post_process_params ()
{
  if (moves_n == 0)
    throw ModuleError (ModuleError::BAD_VALUE,
          "Number of moves must be greater than 0.");
}

void
NPuzzleModule::parse_input (const std::string& input)
{
  std::istringstream is (input);
  
  is >> rows_n >> cols_n;
  if (is.fail ())
    throw InputSyntaxError ("Cant parse number of rows and/or columns");
  
  elems.resize (rows_n*cols_n);
  
  for (guint i = 0; i < rows_n*cols_n; i++)
  {
    is >> elems[i];
    if (is.fail ())
      throw InputSyntaxError ("Cant parse NPuzzle element");
    if (elems[i] >= rows_n*cols_n)
      throw InputSyntaxError ("NPuzzle element value out of range");
  }
  /* checking if good state */
  std::vector<bool> used (rows_n*cols_n);
  std::fill (used.begin (), used.end (), false);
  
  for (guint i = 0; i < rows_n*cols_n; i++)
    if (used[elems[i]])
      throw InputSyntaxError ("Bad state of NPuzzle");
    else
      used[elems[i]] = true;
}

namespace
{
static const gint32 LIT_TRUE = -1;

static gint32
gen_move_cell_def (CNF& cnf, gint32 current,
    gint32 move_left, gint32 move_right, gint32 move_up, gint32 move_down,
    gint32 left, gint32 right, gint32 up, gint32 down)
{
  /* generate definition:
   * cell = (left^mleft)v(right^mright)v(up^mup)v(down^mdown)v
   *      (oldcell^-mleft^-mright^-mup^-mdown)
   *  left,right,up,down - previous values from left,right cell,
   *  mleft,mright,mup,mdown - if true then move in specified direction
   *  oldcell - old cell value
   */

  gint32 newcell;
  /* if not only old cell value */
  if ((left != 0 && move_left != 0) || (right != 0 && move_right != 0) ||
      (up != 0 && move_up != 0) || (down != 0 && move_down != 0))
  {
    LiteralVector or_clause;
    gint32 temp;
    newcell = cnf.add_var ();
    or_clause.push_back (-newcell);
    
    /* if to value from left side */
    if (left != 0 && move_left != 0)
    {
      if (left == LIT_TRUE && move_left == LIT_TRUE)
        return LIT_TRUE; /* if always true */
      if (left == LIT_TRUE)
      {
        or_clause.push_back (move_left);
        cnf.add_clause (newcell, -move_left);
      }
      else if (move_left == LIT_TRUE)
      {
        or_clause.push_back (left);
        cnf.add_clause (newcell, -left);
      }
      else
      {
	temp = cnf.add_var ();
	or_clause.push_back (temp);
	cnf.add_clause (-temp, left);
	cnf.add_clause (-temp, move_left);
	cnf.add_clause (newcell, -left, -move_left);
      }
    }
    
    /* if to value from right side */
    if (right != 0 && move_right != 0)
    {
      if (right == LIT_TRUE && move_right == LIT_TRUE)
        return LIT_TRUE; /* if always true */
      if (right == LIT_TRUE)
      {
        or_clause.push_back (move_right);
        cnf.add_clause (newcell, -move_right);
      }
      else if (move_right == LIT_TRUE)
      {
        or_clause.push_back (right);
        cnf.add_clause (newcell, -right);
      }
      else
      {
	temp = cnf.add_var ();
	or_clause.push_back (temp);
	cnf.add_clause (-temp, right);
	cnf.add_clause (-temp, move_right);
	cnf.add_clause (newcell, -right, -move_right);
      }
    }
    
    /* if to value from down side */
    if (down != 0 && move_down != 0)
    {
      if (down == LIT_TRUE && move_down == LIT_TRUE)
        return LIT_TRUE; /* if always true */
      if (down == LIT_TRUE)
      {
        or_clause.push_back (move_down);
        cnf.add_clause (newcell, -move_down);
      }
      else if (move_down == LIT_TRUE)
      {
        or_clause.push_back (down);
        cnf.add_clause (newcell, -down);
      }
      else
      {
	temp = cnf.add_var ();
	or_clause.push_back (temp);
	cnf.add_clause (-temp, down);
	cnf.add_clause (-temp, move_down);
	cnf.add_clause (newcell, -down, -move_down);
      }
    }
    
    /* if to value from up side */
    if (up != 0 && move_up != 0)
    {
      if (up == LIT_TRUE && move_up == LIT_TRUE)
        return LIT_TRUE; /* if always true */
      if (up == LIT_TRUE)
      {
        or_clause.push_back (move_up);
        cnf.add_clause (newcell, -move_up);
      }
      else if (move_up == LIT_TRUE)
      {
        or_clause.push_back (up);
        cnf.add_clause (newcell, up);
      }
      else
      {
	temp = cnf.add_var ();
	or_clause.push_back (temp);
	cnf.add_clause (-temp, up);
	cnf.add_clause (-temp, move_up);
	cnf.add_clause (newcell, -up, -move_up);
      }
    }
    
    /* add definition of copy if all moves is not set.
       (obsolete in empties definition) */
    if (current != 0 && move_left != LIT_TRUE && move_right != LIT_TRUE &&
        move_up != LIT_TRUE && move_down != LIT_TRUE)
    {
      if (move_left == 0 && move_right == 0 && move_up == 0 && move_down == 0)
      {
	if (current != LIT_TRUE)
	{
	  /*or_clause.push_back (current);
	  cnf.add_clause (newcell, -current);*/
	  newcell = current;
	}
      }
      else
      {
        /* or2_clause - newcell <= current^-mleft^...^-mdown */
        LiteralVector or2_clause;
        temp = cnf.add_var ();
        or_clause.push_back (temp);
        or2_clause.push_back (newcell);
        if (current != LIT_TRUE)
	{
	  cnf.add_clause (-temp, current);
	  or2_clause.push_back (-current);
	}
        
	if (move_left != 0)
	{ /* move_left is variable */
	  cnf.add_clause (-temp, -move_left);
	  or2_clause.push_back (move_left);
	}
	if (move_right != 0)
	{ /* move_right is variable */
	  cnf.add_clause (-temp, -move_right);
	  or2_clause.push_back (move_right);
	}
	if (move_up != 0)
	{ /* move_up is variable */
	  cnf.add_clause (-temp, -move_up);
	  or2_clause.push_back (move_up);
	}
	if (move_down != 0)
	{ /* move_down is variable */
	  cnf.add_clause (-temp, -move_down);
	  or2_clause.push_back (move_down);
	}
	cnf.add_clause (or2_clause);
      }
    }
    
    /* positive clause for logical sum for empties cell */
    cnf.add_clause (or_clause);
  }
  else
    newcell = current;
  return newcell;
}

};

void
NPuzzleModule::generate (CNF& cnf, std::string& outmap_string,
    bool with_outmap) const
{
  if (with_outmap)
    outmap_string.clear ();

  guint elems_bits_n = 0;
  guint elems_n = rows_n*cols_n;
  for (guint v = 1; v < elems_n; v <<= 1, elems_bits_n++);
  
  LiteralVector state0 (elems_n*elems_bits_n);
  LiteralVector empties0 (elems_n);
  gint32 move0, move1; /* move bits */
  LiteralVector state1 (elems_n*elems_bits_n);
  LiteralVector empties1 (elems_n);
  
  LiteralVector* state = &state0;
  LiteralVector* empties = &empties0;
  LiteralVector* prev_state = &state1;
  LiteralVector* prev_empties = &empties1;
  
  for (guint i = 0; i < elems_n; i++)
  {
    (*prev_empties)[i] = (elems[i] == 0) ? LIT_TRUE : 0;
    guint v = 1;
    for (guint k = 0; k < elems_bits_n; v <<= 1, k++)
      (*prev_state)[i*elems_bits_n + k] = (elems[i] & v) ? LIT_TRUE : 0;
  }
  
  gint32 prev_move0 = 0;
  gint32 prev_move1 = 0;
  
  for (guint m = 0; m < moves_n; m++)
  {
    gint32 move_left, move_right, move_up, move_down;
    move0 = cnf.add_var ();
    move1 = cnf.add_var ();
    
    move_left = cnf.add_var ();
    cnf.add_clause (-move_left, -move0);
    cnf.add_clause (-move_left, -move1);
    cnf.add_clause (move_left, move0, move1);
    move_right = cnf.add_var ();
    cnf.add_clause (-move_right, move0);
    cnf.add_clause (-move_right, -move1);
    cnf.add_clause (move_right, -move0, move1);
    move_up = cnf.add_var ();
    cnf.add_clause (-move_up, -move0);
    cnf.add_clause (-move_up, move1);
    cnf.add_clause (move_up, move0, -move1);
    move_down = cnf.add_var ();
    cnf.add_clause (-move_down, move0);
    cnf.add_clause (-move_down, move1);
    cnf.add_clause (move_down, -move0, -move1);
    
    if (with_outmap)
    {
      std::ostringstream os;
      os << "{ {l|r|u|d} : " << move0 << ' ' << move1 << '}';
      outmap_string += os.str ();
    }
    
    /* empties and elems definitions */
    for (guint i = 0; i < rows_n; i++)
      for (guint j = 0; j < cols_n; j++)
      {
        gint32 left, right, down, up; /* empties values */
        gint32 sleft, sright, sdown, sup; /* state values */
        left = (j < cols_n-1) ? (*prev_empties)[i*cols_n+j+1] : 0;
        right = (j > 0) ? (*prev_empties)[i*cols_n+j-1] : 0;
        up = (i < rows_n-1) ? (*prev_empties)[(i+1)*cols_n+j] : 0;
        down = (i > 0) ? (*prev_empties)[(i-1)*cols_n+j] : 0;
        
        /* oldcell is 0 - because is empty cell */
        (*empties)[i*cols_n+j] = gen_move_cell_def (cnf, 0,
            move_left, move_right, move_up, move_down, left, right, up, down);
        
        /* toleft, ..., toup - move predicates for state cell */
        gint32 toleft, toright, toup, todown;
        gint32 prev_empty = (*prev_empties)[i*cols_n+j];
        {
          if (prev_empty == LIT_TRUE)
          {
            toleft = (j > 0) ? move_left : 0;
            toright = (j < cols_n-1) ? move_right : 0;
            toup = (i > 0) ? move_up : 0;
            todown = (i < rows_n-1) ? move_down : 0;
          }
          else if (prev_empty != 0)
          {
            if (j > 0)
            {
	      toleft = cnf.add_var ();
	      cnf.add_clause (-toleft, prev_empty);
	      cnf.add_clause (-toleft, move_left);
	      cnf.add_clause (toleft, -prev_empty, -move_left);
	    }
	    else
	      toleft = 0;
	    if (j < cols_n-1)
	    {
	      toright = cnf.add_var ();
	      cnf.add_clause (-toright, prev_empty);
	      cnf.add_clause (-toright, move_right);
	      cnf.add_clause (toright, -prev_empty, -move_right);
	    }
	    else
	      toright = 0;
	    if (i > 0)
	    {
	      toup = cnf.add_var ();
	      cnf.add_clause (-toup, prev_empty);
	      cnf.add_clause (-toup, move_up);
	      cnf.add_clause (toup, -prev_empty, -move_up);
	    }
	    else
	      toup = 0;
	    if (i < rows_n-1)
	    {
	      todown = cnf.add_var ();
	      cnf.add_clause (-todown, prev_empty);
	      cnf.add_clause (-todown, move_down);
	      cnf.add_clause (todown, -prev_empty, -move_down);
            }
            else
              todown = 0;
          }
          else
            toleft = toright = toup = todown = 0;
        }
        guint v = 1;
        for (guint k = 0; k < elems_bits_n; v <<= 1, k++)
	  {
	    sleft = (j > 0) ?
		(*prev_state)[(i*cols_n+j-1)*elems_bits_n+k] : 0;
	    sright = (j < cols_n-1) ?
		(*prev_state)[(i*cols_n+j+1)*elems_bits_n+k] : 0;
	    sup = (i > 0) ?
		(*prev_state)[((i-1)*cols_n+j)*elems_bits_n+k] : 0;
	    sdown = (i < rows_n-1) ?
		(*prev_state)[((i+1)*cols_n+j)*elems_bits_n+k] : 0;
	    
	    (*state)[(i*cols_n+j)*elems_bits_n + k] =
		gen_move_cell_def (cnf,
		    (*prev_state)[(i*cols_n+j)*elems_bits_n + k],
		    toleft, toright, toup, todown, sleft, sright, sup, sdown);
	  }
      }
    
    /* good move definition */
    for (guint i = 0; i < cols_n; i++)
    {
      gint32 var = (*prev_empties)[i];
      if (var == LIT_TRUE)
        cnf.add_clause (-move_up);
      else if (move_up == LIT_TRUE)
        cnf.add_clause (-var);
      else if (var == LIT_TRUE && move_up == LIT_TRUE)
        cnf.add_clause (0);
      else if (var != 0 && move_up != 0)
        cnf.add_clause (-var, -move_up);
      
      var = (*prev_empties)[(rows_n-1)*cols_n+i];
      if (var == LIT_TRUE)
        cnf.add_clause (-move_down);
      else if (move_down == LIT_TRUE)
        cnf.add_clause (-var);
      else if (var == LIT_TRUE && move_down == LIT_TRUE)
        cnf.add_clause (0);
      else if (var != 0 && move_down != 0)
        cnf.add_clause (-var, -move_down);
    }
    for (guint i = 0; i < rows_n; i++)
    {
      gint32 var = (*prev_empties)[i*cols_n];
      if (var == LIT_TRUE)
        cnf.add_clause (-move_left);
      else if (move_left == LIT_TRUE)
        cnf.add_clause (-var);
      else if (var == LIT_TRUE && move_left == LIT_TRUE)
        cnf.add_clause (0);
      else if (var != 0 && move_left != 0)
        cnf.add_clause (-var, -move_left);
      
      var = (*prev_empties)[i*cols_n+cols_n-1];
      if (var == LIT_TRUE)
        cnf.add_clause (-move_right);
      else if (move_right == LIT_TRUE)
        cnf.add_clause (-var);
      else if (var == LIT_TRUE && move_right == LIT_TRUE)
        cnf.add_clause (0);
      else if (var != 0 && move_right != 0)
        cnf.add_clause (-var, -move_right);
    }
    
    /* swap prevs */
    std::swap (state, prev_state);
    std::swap (empties, prev_empties);
    
    /* special move condition:
     * current move shouldn't be opposite to previous move (left-right, up-down) */
    if (no_obsolete)
      if (prev_move0 != 0 && prev_move1 != 0)
      {
	gint32 c1, c2;
	c1 = cnf.add_var ();
	c2 = cnf.add_var ();
	/* c1 -> move1 != prev_move1 */
	cnf.add_clause (-c1, prev_move1, move1);
	cnf.add_clause (-c1, -prev_move1, -move1);
	/* c2 -> move0 == prev_move0 */
	cnf.add_clause (-c2, -prev_move0, move0);
	cnf.add_clause (-c2, prev_move0, -move0);
	cnf.add_clause (c1, c2);
      }
    
    prev_move0 = move0;
    prev_move1 = move1;
  }
  
  /* last condition */
  for (guint i = 0; i < elems_n-1; i++)
  { /* after swapping current state defined as previous state */
    guint v = 1;
    for (guint k = 0; k < elems_bits_n; v <<= 1, k++)
    {
      gint32 var = (*prev_state)[i*elems_bits_n+k];
      if (var != LIT_TRUE && var != 0)
        cnf.add_clause (((i+1) & v) ? (*prev_state)[i*elems_bits_n+k] :
            -(*prev_state)[i*elems_bits_n+k]);
      else if ((var == LIT_TRUE) != (((i+1)&v) ? true : false))
        /* if always false */
        cnf.add_clause (0);
    }
  }
  /* empty must be in right-down corner */
  if ((*prev_empties)[elems_n-1] != LIT_TRUE && (*prev_empties)[elems_n-1] != 0)
    cnf.add_clause ((*prev_empties)[elems_n-1]);
  else if ((*prev_empties)[elems_n-1] != LIT_TRUE)
    cnf.add_clause (0);
}
