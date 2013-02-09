/*
 * mysat.cc - my SAT solver
 * Author: Mateusz Szpakowski
 * License: LGPL v2.0
 */

#include <iostream>
#include <algorithm>
#include <clocale>
#include <cstring>
#include <signal.h>
#include <memory>
#include <glibmm.h>
#include <satutils.h>
#include "modules-table.h"

using namespace SatUtils;

class MySatOptionGroup: public Glib::OptionGroup
{
public:
  struct OptionState
  {
    Glib::ustring module_name;
    Glib::ustring module_params;
    bool progress;
    bool simplify;
    int timeout;
    bool check_model;
    bool no_model;
    bool verbose;
    bool list_modules;
    bool list_params;
  };
private:
  OptionState option_state;
  Glib::OptionEntry module_name_entry;
  Glib::OptionEntry module_params_entry;
  Glib::OptionEntry progress_entry;
  Glib::OptionEntry timeout_entry;
  Glib::OptionEntry simplify_entry;
  Glib::OptionEntry check_model_entry;
  Glib::OptionEntry no_model_entry;
  Glib::OptionEntry verbose_entry;
  Glib::OptionEntry list_modules_entry;
  Glib::OptionEntry list_params_entry;
public:
  MySatOptionGroup ();

  const OptionState& get_option_state () const
  { return option_state; }
};

MySatOptionGroup::MySatOptionGroup ()
    : Glib::OptionGroup ("mysat group", "MySat Option Group",
          "MySat Option Group")
{
  option_state.list_modules = false;
  option_state.list_params = false;
  option_state.progress = false;
  option_state.simplify = false;
  option_state.check_model = false;
  option_state.no_model = false;
  option_state.timeout = -1;

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

  list_modules_entry.set_short_name ('l');
  list_modules_entry.set_long_name ("list-modules");
  list_modules_entry.set_description ("list all modules");
  add_entry (list_modules_entry, option_state.list_modules);

  list_params_entry.set_short_name ('k');
  list_params_entry.set_long_name ("list-params");
  list_params_entry.set_description ("list all parameters for module");
  add_entry (list_params_entry, option_state.list_params);

  timeout_entry.set_short_name ('t');
  timeout_entry.set_long_name ("timeout");
  timeout_entry.set_description ("set maximum time to solve problem");
  timeout_entry.set_arg_description ("SECONDS");
  add_entry (timeout_entry, option_state.timeout);

  simplify_entry.set_short_name ('s');
  simplify_entry.set_long_name ("simplify");
  simplify_entry.set_description ("simplify input formulae");
  add_entry (simplify_entry, option_state.simplify);

  progress_entry.set_short_name ('p');
  progress_entry.set_long_name ("progress");
  progress_entry.set_description ("print progress informations");
  add_entry (progress_entry, option_state.progress);

  check_model_entry.set_short_name ('c');
  check_model_entry.set_long_name ("check-model");
  check_model_entry.set_description ("evaluate formulae for model");
  add_entry (check_model_entry, option_state.check_model);

  no_model_entry.set_short_name ('n');
  no_model_entry.set_long_name ("no-model");
  no_model_entry.set_description ("dont print model");
  add_entry (no_model_entry, option_state.no_model);

  verbose_entry.set_short_name ('v');
  verbose_entry.set_long_name ("verbose");
  verbose_entry.set_description ("verbose mode");
  add_entry (verbose_entry, option_state.verbose);
}

/* progress handling */

static void
main_on_progress (const std::string& comment)
{
  std::cout << "c " << comment << std::endl;
}

static void
main_on_timeout (int signo)
{
  std::cout.flush ();
  std::cout << "c TIMEOUT\ns UNKNOWN" << std::endl;
  _exit (0);
}

static void
main_on_signal (int signo)
{
  std::cout.flush ();
  std::cout << "c\nc Terminated by " << strsignal (signo) <<
      "\ns UNKNOWN" << std::endl;
  exit (0);
}

/* prepare CNF */

static void
prepare_cnf (CNF& cnf)
{
  LiteralVector& form = cnf.get_formulae ();

  guint32 idx = 0;
  guint32 newidx = 0;
  for (guint i = 0; i < cnf.get_clauses_n (); i++)
  {
    guint32 clause_size = form[idx];
    std::sort (form.begin () + idx + 1, form.begin () + idx + clause_size + 1);
    LiteralIter it = std::unique_copy(form.begin () + idx + 1,
	form.begin () + idx + clause_size + 1, form.begin () + newidx + 1);
    form[newidx] = it - (form.begin() + newidx + 1);

    idx += clause_size + 1;
    newidx += form[newidx] + 1;
  }
  form.resize (newidx);
}

/* main function */

int
main (int argc, char** argv)
{
  setlocale (LC_ALL, "");
  setlocale (LC_NUMERIC, "C");

  Glib::init ();

  MySatOptionGroup option_group;
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

  const MySatOptionGroup::OptionState& option_state =
      option_group.get_option_state ();

  if (!option_state.list_modules && !option_state.list_params)
    if (argc < 2)
    {
      std::cout << "Usage: mysat CNFFILE" << std::endl;
      return 0;
    }

  int exit_status = 0;

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
    std::auto_ptr<MySatModule> module;
    if (option_state.module_name.size () != 0)
    {
      guint module_index;
      for (module_index = 0; module_index < modules_info_table_size;
           ++module_index)
	if (modules_info_table[module_index].name == option_state.module_name)
	{
	  module = std::auto_ptr<MySatModule>
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
      Result result;
      CNF cnf;
      std::vector<bool> model;

      if (option_state.progress)
        module->signal_progress ().connect (sigc::ptr_fun (&main_on_progress));
      {

        if (option_state.timeout >= 0)
        {
          struct sigaction oldsigact, newsigact;
	  newsigact.sa_handler = main_on_timeout;
	  sigemptyset (&newsigact.sa_mask);
	  newsigact.sa_flags = 0;
	  sigaction (SIGALRM, &newsigact, &oldsigact);
	  alarm (option_state.timeout);
        }
        {
	  struct sigaction oldsigact, newsigact;
	  newsigact.sa_handler = main_on_signal;
	  sigemptyset (&newsigact.sa_mask);
	  newsigact.sa_flags = 0;
	  sigaction (SIGINT, &newsigact, &oldsigact);
        }
        {
          struct sigaction oldsigact, newsigact;
	  newsigact.sa_handler = main_on_signal;
	  sigemptyset (&newsigact.sa_mask);
	  newsigact.sa_flags = 0;
	  sigaction (SIGTERM, &newsigact, &oldsigact);
        }
      }

      Glib::Timer timer;

      cnf.load_from_file (argv[1]);
      prepare_cnf(cnf);
      module->fetch_problem(cnf);
      //cnf.clear(); /* after fetching this data is obsolete */

      timer.start ();

      result = module->solve (model);

      timer.stop ();

      if (result == UNKNOWN)
      {
        exit_status = 0;
        std::cout << "s UNKNOWN" << std::endl;
      }
      else if (result == SATISFIABLE)
      {
        exit_status = 10;
        std::cout << "s SATISFIABLE" << std::endl;
        /* print model */
        if (!option_state.no_model)
        {
	  std::cout << 'v';
	  for (int i = 0; i < int (model.size ()); i++)
	    std::cout << ' ' << ((model[i]) ? (i+1) : (-i-1));
	  std::cout << " 0" << std::endl;
        }

        bool result = (cnf.evaluate (model));
        if (option_state.check_model)
          std::cout << "c result of formulae: " << result << std::endl;
        if (!result)
          exit_status = 40;
      }
      else /*if (result == UNSATISFIABLE)*/
      {
        exit_status = 20;
        std::cout << "s UNSATISFIABLE" << std::endl;
      }
      std::cout << "c Time: " << timer.elapsed () << " sec." << std::endl;
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

  return exit_status;
}
