/*
 * module.h - module of satgen
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#ifndef __SATGEN_MODULE_H__
#define __SATGEN_MODULE_H__

#include <satutils.h>

using namespace SatUtils;

class InputSyntaxError: public Glib::Error
{
public:
  enum Code { SYNTAX_ERROR };

  InputSyntaxError (); /* default is syntax error */
  InputSyntaxError (const Glib::ustring& text);
  InputSyntaxError (guint lineno, const Glib::ustring& text);
  Code code () const;
  Glib::ustring what () const;
};

class SatGenModule: public Module
{
protected:
  SatGenModule ();
public:
  virtual ~SatGenModule ();
  
  virtual void parse_input (const std::string& input) = 0;
  virtual void generate (CNF& cnf, std::string& outmap_string,
        bool with_outmap) const = 0;
};

#endif /* __SATGEN_MODULE_H__ */
