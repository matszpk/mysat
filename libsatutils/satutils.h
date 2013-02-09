/*
 * satutils.h - Sat utilities with SAT IO
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __MYSAT_SATUTILS_H__
#define __MYSAT_SATUTILS_H__

#include <string>
#include <vector>
#include <glibmm.h>

namespace SatUtils
{

class CNFError: public Glib::Error
{
public:
  enum Code
  {
    BAD_FORMAT,
    BAD_LITERAL,
    OPEN_ERROR,
    IO_ERROR,
    END_OF_FILE,
  };

  CNFError (); /* default is syntax error */
  CNFError (int code, const Glib::ustring& text);
  Code code () const;
  Glib::ustring what () const;
};

/* definition of CNF */

typedef std::vector<gint32> LiteralVector;
typedef std::vector<gint32>::iterator LiteralIter;
typedef std::vector<gint32>::const_iterator LiteralConstIter;

LiteralVector get_literals_from_range (gint32 a, gint32 b);
void add_literals_from_range (LiteralVector& v, gint32 a, gint32 b);

/*
 * structure of formulae:
 * clause_size [literal [literal]] clause_size [literal [literal]]
 * clause_size [literal [literal]] clause_size [literal [literal]] ...
 */

class CNF
{
private:
  gint32 vars_n;
  guint32 clauses_n;
  LiteralVector formulae;

public:
  CNF ();
  CNF (gint32 input_vars_n, guint32 input_clauses_n);
  explicit CNF (const std::string& filename);

  bool operator== (const CNF& cnf) const;
  bool operator!= (const CNF& cnf) const
  { return !(*this == cnf); }

  void load_from_file (const std::string& filename);
  void save_to_file (const std::string& filename) const;

  gint32 get_vars_n () const
  { return vars_n; }
  gint32 next_var () const
  { return vars_n+1; }
  guint32 get_clauses_n () const
  { return clauses_n; }
  guint32 get_literals_n () const
  { return formulae.size () - clauses_n; }

  gint32 add_var ();
  void add_vars (gint32 vars_n);
  LiteralVector add_vars_with_literals (gint32 vars_n);
  void resize (gint32 vars_n, guint32 clauses_n);
  bool check_consistency () const;

  LiteralIter begin ()
  { return formulae.begin (); }
  LiteralConstIter begin () const
  { return formulae.begin (); }

  LiteralIter end ()
  { return formulae.end (); }
  LiteralConstIter end () const
  { return formulae.end (); }

  LiteralVector& get_formulae ()
  { return formulae; }
  const LiteralVector& get_formulae () const
  { return formulae; }

  void clear ();

  /* add clause with literals: ignore '0' literals */
  void add_empty_clause ();
  void add_clause (guint literals_n, const gint32* literals);
  void add_clause (const LiteralVector& literals);
  void add_clause (gint32 l1);
  void add_clause (gint32 l1, gint32 l2);
  void add_clause (gint32 l1, gint32 l2, gint32 l3);
  void add_clause (gint32 l1, gint32 l2, gint32 l3, gint32 l4);

  bool evaluate (const std::vector<bool>& v) const;

  void clauses_indices (std::vector<guint32>& indices) const;
};

/* definition of Module */

class ModuleError: public Glib::Error
{
public:
  enum Code
  {
    PARSE_SYNTAX,
    BAD_VALUE,
    OTHER_ERROR,
  };

  ModuleError (); /* default is syntax error */
  ModuleError (int code, const Glib::ustring& text);
  Code code () const;
  Glib::ustring what () const;
};

struct ModuleBoolParam
{
  std::string name;
  std::string description;
  bool* value;
  ModuleBoolParam () { }
  ModuleBoolParam (const std::string& n, const std::string& d, bool& v)
      : name (n), description (d), value (&v)
  { }
};

struct ModuleIntParam
{
  std::string name;
  std::string description;
  int* value;
  ModuleIntParam () { }
  ModuleIntParam (const std::string& n, const std::string& d, int& v)
      : name (n), description (d), value (&v)
  { }
};

struct ModuleUIntParam
{
  std::string name;
  std::string description;
  guint* value;
  ModuleUIntParam () { }
  ModuleUIntParam (const std::string& n, const std::string& d, guint& v)
      : name (n), description (d), value (&v)
  { }
};

struct ModuleInt64Param
{
  std::string name;
  std::string description;
  gint64* value;
  ModuleInt64Param () { }
  ModuleInt64Param (const std::string& n, const std::string& d, gint64& v)
      : name (n), description (d), value (&v)
  { }
};

struct ModuleUInt64Param
{
  std::string name;
  std::string description;
  guint64* value;
  ModuleUInt64Param () { }
  ModuleUInt64Param (const std::string& n, const std::string& d, guint64& v)
      : name (n), description (d), value (&v)
  { }
};


struct ModuleFloatParam
{
  std::string name;
  std::string description;
  float* value;
  ModuleFloatParam () { }
  ModuleFloatParam (const std::string& n, const std::string& d, float& v)
      : name (n), description (d), value (&v)
  { }
};

struct ModuleStringParam
{
  std::string name;
  std::string description;
  std::string* value;
  ModuleStringParam () { }
  ModuleStringParam (const std::string& n, const std::string& d, std::string& v)
      : name (n), description (d), value (&v)
  { }
};

typedef std::vector<ModuleBoolParam> ModuleBoolParamVector;
typedef std::vector<ModuleIntParam> ModuleIntParamVector;
typedef std::vector<ModuleUIntParam> ModuleUIntParamVector;
typedef std::vector<ModuleInt64Param> ModuleInt64ParamVector;
typedef std::vector<ModuleUInt64Param> ModuleUInt64ParamVector;
typedef std::vector<ModuleFloatParam> ModuleFloatParamVector;
typedef std::vector<ModuleStringParam> ModuleStringParamVector;

/* Module class */

class Module: public sigc::trackable
{
private:
  ModuleBoolParamVector m_bool_params;
  ModuleIntParamVector m_int_params;
  ModuleUIntParamVector m_uint_params;
  ModuleInt64ParamVector m_int64_params;
  ModuleUInt64ParamVector m_uint64_params;
  ModuleFloatParamVector m_float_params;
  ModuleStringParamVector m_string_params;

protected:
  void add_param (const std::string& name, const std::string& desc, bool& value);
  void add_param (const std::string& name, const std::string& desc, int& value);
  void add_param (const std::string& name, const std::string& desc, guint& value);
  void add_param (const std::string& name, const std::string& desc, gint64& value);
  void add_param (const std::string& name, const std::string& desc, guint64& value);
  void add_param (const std::string& name, const std::string& desc, float& value);
  void add_param (const std::string& name, const std::string& desc,
        std::string& value);

  virtual void post_process_params () = 0;

  Module ();
public:
  virtual ~Module ();

  void parse_params (const std::string& input);

  const ModuleBoolParamVector& get_bool_params () const
  { return m_bool_params; }
  const ModuleIntParamVector& get_int_params () const
  { return m_int_params; }
  const ModuleUIntParamVector& get_uint_params () const
  { return m_uint_params; }
  const ModuleInt64ParamVector& get_int64_params () const
  { return m_int64_params; }
  const ModuleUInt64ParamVector& get_uint64_params () const
  { return m_uint64_params; }
  const ModuleFloatParamVector& get_float_params () const
  { return m_float_params; }
  const ModuleStringParamVector& get_string_params () const
  { return m_string_params; }

  std::string get_module_params_info_string () const;
};

};

#endif /* __MYSAT_SATUTILS_H__ */
