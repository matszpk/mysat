/*
 * module.cc - satgen module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <sstream>
#include "module.h"

namespace
{

std::string text_with_lineno (guint lineno, const Glib::ustring& text)
{
  std::ostringstream os;
  os << "Line " << lineno << ": " << text;
  return os.str ();
}

};

/* input syntax error class */

InputSyntaxError::InputSyntaxError ()
    : Glib::Error (Glib::Quark ("Input Syntax"),
          InputSyntaxError::SYNTAX_ERROR, "Parse syntax error")
{
}

InputSyntaxError::InputSyntaxError (const Glib::ustring& text)
    : Glib::Error (Glib::Quark ("Input Syntax"),
          InputSyntaxError::SYNTAX_ERROR, text)
{
}

InputSyntaxError::InputSyntaxError (guint lineno, const Glib::ustring& text)
   : Glib::Error (Glib::Quark ("Input Syntax"),
          InputSyntaxError::SYNTAX_ERROR, text_with_lineno (lineno, text))
{
}

InputSyntaxError::Code
InputSyntaxError::code () const
{
  return static_cast<InputSyntaxError::Code>(Glib::Error::code ());
}

Glib::ustring
InputSyntaxError::what () const
{
  Glib::ustring out = "Input syntax error: ";
  out += Glib::Error::what ();
  return out;
}

/* SatGenModule */

SatGenModule::SatGenModule ()
{
}

SatGenModule::~SatGenModule ()
{
}
