/*
 * satio.cc - SAT I/O module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <fstream>
#include <iostream>
#include "satutils.h"

using namespace SatUtils;

namespace SatUtils
{

LiteralVector
get_literals_from_range (gint32 a, gint32 b)
{
  LiteralVector v;
  add_literals_from_range (v, a, b);
  return v;
};

void
add_literals_from_range (LiteralVector& v, gint32 a, gint32 b)
{
  if (a < b)
    for (gint32 i = a; i < b; i++)
      v.push_back (i);
  else
    for (gint32 i = a; i > b; i--)
      v.push_back (i);
};


};

/* CNFError class */

CNFError::CNFError ()
    : Glib::Error (Glib::Quark ("CNFIO"), CNFError::IO_ERROR, "IO Error")
{
}

CNFError::CNFError (int code, const Glib::ustring& text)
    : Glib::Error (Glib::Quark ("CNFIO"), code, text)
{
}

CNFError::Code
CNFError::code () const
{
  return static_cast<CNFError::Code>(Glib::Error::code ());
}

Glib::ustring
CNFError::what () const
{
  Glib::ustring out = "CNF I/O: ";
  out += Glib::Error::what ();
  return out;
}

/* CNF class */

CNF::CNF ()
    : vars_n (0), clauses_n (0)
{
}

CNF::CNF (gint32 input_vars_n, guint32 input_clauses_n)
    : vars_n (input_vars_n), clauses_n (input_clauses_n)
{
}

CNF::CNF (const std::string& filename)
{
  load_from_file (filename);
}

bool
CNF::operator== (const CNF& cnf) const
{
  return (vars_n == cnf.vars_n && clauses_n == cnf.clauses_n &&
      formulae == cnf.formulae);
}

void
CNF::load_from_file (const std::string& filename)
{
  std::ifstream file (filename.c_str (), std::ios::out);
  if (!file)
    throw CNFError (CNFError::OPEN_ERROR, "Open error");


  gint32 newvars_n;
  guint32 newclauses_n;
  LiteralVector newformulae;

  /* reading preamble */
  std::string line;
  while (!file.eof ())
  {
    std::getline (file, line);
    if (file.eof ())
      throw CNFError (CNFError::END_OF_FILE, "End of file");

    if (file.fail ())
    {
      if (file.badbit)
        throw CNFError (CNFError::IO_ERROR, "I/O error");
      else
        throw CNFError (CNFError::BAD_FORMAT, "Bad format");
    }

    if (line.size () == 0 || line[0] != 'c')
      break;
  }

  {
    std::string cnfstr;
    std::istringstream is (line);
    char c;
    c = is.get ();
    if (is.eof () || c != 'p')
      throw CNFError (CNFError::BAD_FORMAT, "Bad format");
    char space = is.get ();
    is >> cnfstr;
    if (cnfstr != "cnf" || space != ' ')
      throw CNFError (CNFError::BAD_FORMAT, "Bad format");
    is >> newvars_n;
    is >> newclauses_n;
    if (is.fail ())
      throw CNFError (CNFError::BAD_FORMAT, "Can't parse formulae size");
    if (newvars_n <= 0)
      throw CNFError (CNFError::BAD_FORMAT,
            "Number of variables must be grater than 0.");
  }

  for (guint32 i = 0; i < newclauses_n; i++)
  {
    LiteralVector clause;
    gint32 v;
    while (1)
    {
      file >> v;

      if (file.eof () && (v != 0 || i+1 < newclauses_n))
	throw CNFError (CNFError::END_OF_FILE, "End of file");

      if (file.fail ())
      {
	if (file.badbit)
	  throw CNFError (CNFError::IO_ERROR, "I/O error");
	else
	  throw CNFError (CNFError::BAD_FORMAT, "Bad format");
      }

      if (v < -newvars_n || v > newvars_n)
        throw CNFError (CNFError::BAD_LITERAL, "Bad format");

      if (v != 0)
        clause.push_back (v);
      else
        break;
    }

    newformulae.push_back (clause.size ());
    newformulae.insert (newformulae.end (), clause.begin (), clause.end ());
  }

  /* replace old CNF by new loaded from file */
  vars_n = newvars_n;
  clauses_n = newclauses_n;
  formulae = newformulae;
}

void
CNF::save_to_file (const std::string& filename) const
{
  std::ofstream file (filename.c_str (), std::ios::out);
  if (!file)
    throw CNFError (CNFError::OPEN_ERROR, "Open error");

  file << "c saved by SATIO"
      "\np cnf " << vars_n << ' ' << clauses_n << '\n';

  LiteralConstIter literal = formulae.begin ();
  for (guint32 i = 0; i < clauses_n; i++)
  {
    gint32 literals_n = *literal++;
    for (gint32 j = 0; j < literals_n; j++)
      file << literal[j] << ' ';
    file << "0\n";
    literal += literals_n;
    if (file.fail ())
      throw CNFError (CNFError::IO_ERROR, "I/O error");
  }
}

gint32
CNF::add_var ()
{
  vars_n++;
  return vars_n;
}

void
CNF::add_vars (gint32 vars_n)
{
  this->vars_n += vars_n;
}

LiteralVector
CNF::add_vars_with_literals (gint32 vars_n)
{
  LiteralVector v = get_literals_from_range (this->vars_n+1, this->vars_n+vars_n+1);
  this->vars_n += vars_n;
  return v;
}

void
CNF::resize (gint32 vars_n, guint32 clauses_n)
{
  this->vars_n = vars_n;
  this->clauses_n = clauses_n;
}

bool
CNF::check_consistency () const
{
  gsize index = 0;
  gsize formulae_size = formulae.size ();
  for (guint32 i = 0; i < clauses_n; i++)
  {
    guint32 clause_size = formulae[index];
    if (index + clause_size >= formulae_size)
      return false;

    for (guint j = 0; j < clause_size; j++)
      if (formulae[index+j+1] < -vars_n || formulae[index+j+1] > vars_n)
        return false;

    index += clause_size + 1;
  }
  return true;
}

/* managing formulae */

void
CNF::clear ()
{
  vars_n = clauses_n = 0;
  formulae.clear ();
}

void
CNF::add_empty_clause ()
{
  formulae.push_back (1);
  formulae.push_back (vars_n);
  formulae.push_back (1);
  formulae.push_back (-vars_n);
  clauses_n += 2;
}

void
CNF::add_clause (guint literals_n, const gint32* literals)
{
  gint32 count = 0;
  for (guint i = 0; i < literals_n; i++)
    if (literals[i] != 0)
      count++;

  if (count != 0)
  {
    formulae.push_back (count);
    for (guint i = 0; i < literals_n; i++)
      if (literals[i] != 0)
	formulae.push_back (literals[i]);
    clauses_n++;
  }
  else
    add_empty_clause ();
}

void
CNF::add_clause (const LiteralVector& literals)
{
  gint32 count = 0;
  for (LiteralConstIter it = literals.begin (); it != literals.end (); ++it)
    if (*it != 0)
      count++;

  if (count != 0)
  {
    formulae.push_back (count);
    for (LiteralConstIter it = literals.begin (); it != literals.end (); ++it)
      if (*it != 0)
	formulae.push_back (*it);
    clauses_n++;
  }
  else
    add_empty_clause ();
}

void
CNF::add_clause (gint32 l1)
{
  if (l1 != 0)
  {
    formulae.push_back ((l1 != 0) ? 1 : 0);
    formulae.push_back (l1);
    clauses_n++;
  }
  else
    add_empty_clause ();
}

void
CNF::add_clause (gint32 l1, gint32 l2)
{
  gint32 count = 0;
  count = (l1 != 0) ? 1 : 0;
  count += (l2 != 0) ? 1 : 0;
  if (count != 0)
  {
    formulae.push_back (count);
    if (l1 != 0)
      formulae.push_back (l1);
    if (l2 != 0)
      formulae.push_back (l2);
    clauses_n++;
  }
  else
    add_empty_clause ();
}

void
CNF::add_clause (gint32 l1, gint32 l2, gint32 l3)
{
  gint32 count = 0;
  count = (l1 != 0) ? 1 : 0;
  count += (l2 != 0) ? 1 : 0;
  count += (l3 != 0) ? 1 : 0;
  if (count != 0)
  {
    formulae.push_back (count);
    if (l1 != 0)
      formulae.push_back (l1);
    if (l2 != 0)
      formulae.push_back (l2);
    if (l3 != 0)
      formulae.push_back (l3);
    clauses_n++;
  }
  else
    add_empty_clause ();
}

void
CNF::add_clause (gint32 l1, gint32 l2, gint32 l3, gint32 l4)
{
  gint32 count = 0;
  count = (l1 != 0) ? 1 : 0;
  count += (l2 != 0) ? 1 : 0;
  count += (l3 != 0) ? 1 : 0;
  count += (l4 != 0) ? 1 : 0;
  if (count != 0)
  {
    formulae.push_back (count);
    if (l1 != 0)
      formulae.push_back (l1);
    if (l2 != 0)
      formulae.push_back (l2);
    if (l3 != 0)
      formulae.push_back (l3);
    if (l4 != 0)
      formulae.push_back (l4);
    clauses_n++;
  }
  else
    add_empty_clause ();
}

bool
CNF::evaluate (const std::vector<bool>& v) const
{
  gsize index = 0;
  for (guint32 i = 0; i < clauses_n; i++)
  {
    guint32 clause_size = formulae[index];
    bool clause_result = false;
    for (guint j = 1; j <= clause_size; j++)
      /* (if literal positive) == v[literal variable] */
      if ((formulae[index+j] > 0) == v[std::abs (formulae[index+j])-1])
      {
        clause_result = true;
        break;
      }
    if (!clause_result) /* if not true */
      return false;
    index += clause_size + 1;
  }
  return true;
}

void
CNF::clauses_indices (std::vector<guint32>& indices) const
{
  guint32 i = 0;
  indices.resize (clauses_n);
  for (guint32 c = 0; c < clauses_n; c++)
  {
    indices[c] = i;
    i += formulae[i];
  }
}
