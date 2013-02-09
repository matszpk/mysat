/*
 * satgen.cc - generating SAT by using input files
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <clocale>
#include <memory>
#include <fstream>
#include <glibmm.h>
#include <satutils.h>
#include "modules-table.h"

using namespace SatUtils;

/* OptionGroup class */

class SatGenOptionGroup: public Glib::OptionGroup
{
public:
  struct OptionState
  {
    Glib::ustring module_name;
    Glib::ustring module_params;
    Glib::ustring outmap_file;
    bool list_modules;
    bool list_params;
  };
private:
  OptionState option_state;
  Glib::OptionEntry module_name_entry;
  Glib::OptionEntry module_params_entry;
  Glib::OptionEntry outmap_file_entry;
  Glib::OptionEntry list_modules_entry;
  Glib::OptionEntry list_params_entry;
public:
  SatGenOptionGroup ();
  
  const OptionState& get_option_state () const
  { return option_state; }
  
};

SatGenOptionGroup::SatGenOptionGroup ()
    : Glib::OptionGroup ("satgen group", "SatGen Option Group",
          "SatGen Option Group")
{
  option_state.list_modules = false;
  option_state.list_params = false;
  
  module_name_entry.set_short_name ('M');
  module_name_entry.set_long_name ("module");
  module_name_entry.set_description ("select generating module");
  module_name_entry.set_arg_description ("MODULE");
  add_entry (module_name_entry, option_state.module_name);
  
  module_params_entry.set_short_name ('P');
  module_params_entry.set_long_name ("params");
  module_params_entry.set_description ("set module parameters");
  module_params_entry.set_arg_description ("PARAM[=VALUE]:...");
  add_entry (module_params_entry, option_state.module_params);
  
  outmap_file_entry.set_short_name ('O');
  outmap_file_entry.set_long_name ("outmap");
  outmap_file_entry.set_description ("generate outmap file");
  outmap_file_entry.set_arg_description ("OUTMAPFILE");
  add_entry (outmap_file_entry, option_state.outmap_file);
  
  list_modules_entry.set_short_name ('l');
  list_modules_entry.set_long_name ("list-modules");
  list_modules_entry.set_description ("list all modules");
  add_entry (list_modules_entry, option_state.list_modules);
  
  list_params_entry.set_short_name ('k');
  list_params_entry.set_long_name ("list-params");
  list_params_entry.set_description ("list all parameters for module");
  add_entry (list_params_entry, option_state.list_params);
}

/* main function */

int
main (int argc, char** argv)
{
  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "C");

  Glib::init ();
  
  SatGenOptionGroup option_group;
  Glib::OptionContext option_context;
  option_context.set_main_group (option_group);
  
  try
  {
    option_context.parse (argc, argv);
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return 1;
  }
  
  const SatGenOptionGroup::OptionState& option_state =
      option_group.get_option_state ();
  
  if (!option_state.list_modules && !option_state.list_params)
    if (argc < 2)
    {
      std::cout << "Usage: satgen CNFFILE [INPUTFILE]" << std::endl;
      return 0;
    }
  
  try
  {
    if (option_state.list_modules)
    {
      for (guint i = 0; i < modules_info_table_size; i++)
        std::cout << "Module name: " << modules_info_table[i].name << "\n"
            "Description: " << modules_info_table[i].description << "\n";
      std::cout.flush ();
    }
    
    bool with_module = false;
    std::auto_ptr<SatGenModule> module;
    if (option_state.module_name.size () != 0)
    {
      guint module_index;
      for (module_index = 0; module_index < modules_info_table_size;
           ++module_index)
	if (modules_info_table[module_index].name == option_state.module_name)
	{
	  module = std::auto_ptr<SatGenModule>
	     (modules_info_table[module_index].create ());
	  with_module = true;
	  break;
	}
      if (module_index == modules_info_table_size)
      {
        std::cerr << "Module not found." << std::endl;
        return 1;
      }
    }
    
    if (option_state.list_params)
    {
      if (!with_module)
      {
        std::cerr << "Module must be specified." << std::endl;
        return 1;
      }
      std::cout << module->get_module_params_info_string ();
      std::cout.flush ();
      return 0;
    }
    
    if (option_state.module_params.size () != 0)
    {
      if (!with_module)
      {
        std::cerr << "Module must be specified." << std::endl;
        return 1;
      }
      module->parse_params (option_state.module_params);
    }
    
    if (with_module)
    {
      if (argc >= 2)
      {
	std::string input_string;
	if (argc >= 3)
	{
	  std::ifstream file (argv[2], std::ios::in);
	  if (!file)
	  {
	    std::cerr << "Input file: Open error" << std::endl;
	    return 1;
	  }
	  file.exceptions (std::ios_base::badbit);
	  while (1)
	  {
	    char c = file.get ();
	    if (file.eof ())
	      break;
	    input_string.push_back (c);
	  }
	}
	else
	{
	  std::cin.exceptions (std::ios_base::badbit);
	  while (1)
	  {
	    char c = std::cin.get ();
	    if (std::cin.eof ())
	      break;
	    input_string.push_back (c);
	  }
	}
	/* generating */
	{
	  CNF cnf;
	  std::string outmap_string;
	  module->parse_input (input_string);
	  module->generate (cnf, outmap_string,
	           option_state.outmap_file.size () != 0);
	  
	  /* saving result */
	  if (option_state.outmap_file.size ())
	  {
	    std::ofstream ofile (option_state.outmap_file.c_str (),
	         std::ios::out);
	    if (!ofile)
	    {
	      std::cerr << "OutMap file: Open error" << std::endl;
	      return 1;
	    }
	    ofile.exceptions (std::ios_base::badbit);
	    ofile << outmap_string;
	  }
	  cnf.save_to_file (argv[1]);
	}
      }
      else
      {
        std::cerr << "Output file must be specified" << std::endl;
        return 1;
      }
    }
    else if (argc >= 2)
    {
      std::cerr << "Module must be specified" << std::endl;
      return 1;
    }
  }
  catch (Glib::Exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return 1;
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what () << std::endl;
    return 1;
  }
  
  return 0;
}
