/*
 * satio.cc - SAT I/O module
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <sstream>
#include "satutils.h"

using namespace SatUtils;

/* Module Error class */

ModuleError::ModuleError ()
    : Glib::Error (Glib::Quark ("libsatutils"),
          ModuleError::PARSE_SYNTAX, "Parse syntax error")
{
}

ModuleError::ModuleError (int code, const Glib::ustring& text)
    : Glib::Error (Glib::Quark ("libsatutils"), code, text)
{
}

ModuleError::Code
ModuleError::code () const
{
  return static_cast<ModuleError::Code>(Glib::Error::code ());
}

Glib::ustring
ModuleError::what () const
{
  Glib::ustring out = "SatUtils: ";
  out += Glib::Error::what ();
  return out;
}

/* Module class */

Module::Module ()
{
}

Module::~Module ()
{
}

void
Module::add_param (const std::string& name, const std::string& desc, bool& value)
{
  m_bool_params.push_back (ModuleBoolParam (name, desc, value));
}

void
Module::add_param (const std::string& name, const std::string& desc, int& value)
{
  m_int_params.push_back (ModuleIntParam (name, desc, value));
}

void
Module::add_param (const std::string& name, const std::string& desc, guint& value)
{
  m_uint_params.push_back (ModuleUIntParam (name, desc, value));
}

void
Module::add_param (const std::string& name, const std::string& desc, gint64& value)
{
  m_int64_params.push_back (ModuleInt64Param (name, desc, value));
}

void
Module::add_param (const std::string& name, const std::string& desc, guint64& value)
{
  m_uint64_params.push_back (ModuleUInt64Param (name, desc, value));
}

void
Module::add_param (const std::string& name, const std::string& desc, float& value)
{
  m_float_params.push_back (ModuleFloatParam (name, desc, value));
}

void
Module::add_param (const std::string& name, const std::string& desc,
      std::string& value)
{
  m_string_params.push_back (ModuleStringParam (name, desc, value));
}

void
Module::parse_params (const std::string& input)
{
  std::string::const_iterator it = input.begin ();
  while (it != input.end ())
  {
    std::string::const_iterator namestart = it;
    if (isalpha (*it) || *it == '_')
      while (it != input.end () && (isalnum (*it) || *it == '_')) ++it;
    
    if (it != input.end () && *it != ':' && *it != '=')
      throw ModuleError ();
    
    std::string name (namestart, it);
    
    bool found = false;
    /* finding param */
    {
      ModuleBoolParamVector::iterator param = m_bool_params.begin ();
      for (; param != m_bool_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_bool_params.end ()) /* parse value */
      {
        if (it != input.end () && *it == '=')
        {
          ++it;
          if (it == input.end () ||
              (it != input.end () && *it != '0' && *it != '1'))
            throw ModuleError (ModuleError::PARSE_SYNTAX,
                "Parse error in boolean.");
          /* if '1' then true */
          *(param->value) = (*it == '1');
          ++it;
        }
        else
          *(param->value) = true;
        found = true;
      }
    }
    
    /* prepare string with value */
    std::string value;
    if (!found)
    {
      if (it != input.end () && *it == '=')
      {
	++it;
	std::string::const_iterator valuestart = it;
	/* to separator or end */
	while (it != input.end () && *it != ':') ++it;
	value.assign (valuestart, it);
      }
      else
	throw ModuleError (ModuleError::PARSE_SYNTAX, "\'=\' is required.");
    }
    /* find int param */
    if (!found)
    {
      ModuleIntParamVector::iterator param = m_int_params.begin ();
      for (; param != m_int_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_int_params.end ()) /* parse value */
      {
        std::istringstream is (value);
        is >> *(param->value);
        
        if (is.fail () || !is.eof ())
          throw ModuleError (ModuleError::PARSE_SYNTAX,
                "Parse error in signed integer.");
        found = true;
      }
    }
    /* find uint param */
    if (!found)
    {
      ModuleUIntParamVector::iterator param = m_uint_params.begin ();
      for (; param != m_uint_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_uint_params.end ()) /* parse value */
      {
        std::istringstream is (value);
        is >> *(param->value);
        if (is.fail () || !is.eof ())
          throw ModuleError (ModuleError::PARSE_SYNTAX,
                "Parse error in unsigned integer.");
        found = true;
      }
    }
    /* find int64 param */
    if (!found)
    {
      ModuleInt64ParamVector::iterator param = m_int64_params.begin ();
      for (; param != m_int64_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_int64_params.end ()) /* parse value */
      {
        std::istringstream is (value);
        is >> *(param->value);
        
        if (is.fail () || !is.eof ())
          throw ModuleError (ModuleError::PARSE_SYNTAX,
                "Parse error in 64-bit signed integer.");
        found = true;
      }
    }
    /* find uint64 param */
    if (!found)
    {
      ModuleUInt64ParamVector::iterator param = m_uint64_params.begin ();
      for (; param != m_uint64_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_uint64_params.end ()) /* parse value */
      {
        std::istringstream is (value);
        is >> *(param->value);
        if (is.fail () || !is.eof ())
          throw ModuleError (ModuleError::PARSE_SYNTAX,
                "Parse error in 64-bit unsigned integer.");
        found = true;
      }
    }
    /* find float param */
    if (!found)
    {
      ModuleFloatParamVector::iterator param = m_float_params.begin ();
      for (; param != m_float_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_float_params.end ()) /* parse value */
      {
        std::istringstream is (value);
        is >> *(param->value);
        if (is.fail () || !is.eof ())
          throw ModuleError (ModuleError::PARSE_SYNTAX, "Parse error in float.");
        found = true;
      }
    }
    /* find string param */
    if (!found)
    {
      ModuleStringParamVector::iterator param = m_string_params.begin ();
      for (; param != m_string_params.end (); ++param)
        if (param->name == name)
          break;
      if (param != m_string_params.end ()) /* parse value */
      {
        *(param->value) = value;
        found = true;
      }
    }
    /* if not found */
    if (!found)
      throw ModuleError (ModuleError::PARSE_SYNTAX, "Undefined parameter.");
    /* skip separator */
    if (it != input.end ())
    {
      if (*it != ':')
        throw ModuleError (ModuleError::PARSE_SYNTAX,
                "Bad separator.");
      ++it;
    }
  }
  
  post_process_params ();
}


std::string
Module::get_module_params_info_string () const
{
  std::ostringstream os;
  
  if (m_bool_params.size ())
  {
    os << "Boolean parameters:\n";
    for (ModuleBoolParamVector::const_iterator it = m_bool_params.begin ();
	it != m_bool_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << ((*(it->value)) ? "true" : "false") << ")\n";
  }
  
  if (m_int_params.size ())
  {
    os << "Integer parameters:\n";
    for (ModuleIntParamVector::const_iterator it = m_int_params.begin ();
	it != m_int_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << *(it->value) << ")\n";
  }
  
  if (m_uint_params.size ())
  {
    os << "Unsigned integer parameters:\n";
    for (ModuleUIntParamVector::const_iterator it = m_uint_params.begin ();
	it != m_uint_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << *(it->value) << ")\n";
  }
  
  if (m_int64_params.size ())
  {
    os << "64-bit Integer parameters:\n";
    for (ModuleInt64ParamVector::const_iterator it = m_int64_params.begin ();
	it != m_int64_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << *(it->value) << ")\n";
  }
  
  if (m_uint64_params.size ())
  {
    os << "64-bit unsigned integer parameters:\n";
    for (ModuleUInt64ParamVector::const_iterator it = m_uint64_params.begin ();
	it != m_uint64_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << *(it->value) << ")\n";
  }
  
  if (m_float_params.size ())
  {
    os << "Floating point parameters:\n";
    for (ModuleFloatParamVector::const_iterator it = m_float_params.begin ();
	it != m_float_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << *(it->value) << ")\n";
  }
  
  if (m_string_params.size ())
  {
    os << "String parameters:\n";
    for (ModuleStringParamVector::const_iterator it = m_string_params.begin ();
	it != m_string_params.end (); ++it)
      os << "  " << it->name << " - " << it->description <<
          " (default: " << *(it->value) << ")\n";
  }
  
  return os.str ();
}
