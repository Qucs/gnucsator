/*                -*- C++ -*-
 * Copyright (C) 2018 Felix Salfelder
 * Author: Felix Salfelder <felix@salfelder.org>
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 * top level module with readline
 */
#include <globals.h>
#include <u_prblst.h>
#include <u_sim_data.h>
#include <e_cardlist.h>
#include <u_lang.h>
#include <ap.h>
#include <patchlev.h>
#include <c_comand.h>
#include <declare.h>	/* plclose */
#ifdef HAVE_LIBREADLINE
# include <readline/readline.h>
# include <readline/history.h>
# include <fstream>
#endif
#include <libgen.h> // basename, dirname
#ifdef HAVE_CONFIG
# include "config.h"
#endif
/*--------------------------------------------------------------------------*/
#ifdef HAVE_LIBREADLINE
// static bool use_readline=true;
#else
// static bool use_readline=false;
#endif
/*--------------------------------------------------------------------------*/
extern std::string gnucap_includepath;
/*--------------------------------------------------------------------------*/
#if 1
CS& CS::operator=(const CS& p)
{untested();
  assert(&p != this);
  _name = p._name;
  _file = p._file;
  _cmd = p._cmd;
  _cnt = p._cnt;
  _ok = p._ok;
  _length = p._length;
  return *this;
}
#endif
/*--------------------------------------------------------------------------*/
struct JMP_BUF{
  sigjmp_buf p;
} env;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void prepare_env()
{
  static const char* plugpath="PLUGPATH=" GNUCAP_PLUGPATH
                              "\0         (reserved space)                 ";

  std::string ldlpath = OS::getenv("LD_LIBRARY_PATH");
  if (ldlpath != "") {untested();
    ldlpath += ":";
  }else{
  }
  assert(strlen("PLUGPATH=") == 9);
  trace1("plugpath", ldlpath + (plugpath+9));
  OS::setenv("GNUCAP_PLUGPATH", ldlpath + (plugpath+9), false);
}
/*--------------------------------------------------------------------------*/
static void read_startup_files(char *const* argv)
{
  {
    // TODO: look in $HOME/.gnucap/config:/etc/gnucap/config
    std::string name = findfile(SYSTEMSTARTFILE, SYSTEMSTARTPATH, R_OK);
    if (name != "") { untested();
      trace2("", name, &CARD_LIST::card_list);
      CMD::command("include " + name, &CARD_LIST::card_list);
    }else{ untested();
      CMD::command(std::string("load " DEFAULT_PLUGINS), &CARD_LIST::card_list);
    }
  }
  {
    // TODO: also scan parent directories
    std::string name = findfile(USERSTARTFILE, USERSTARTPATH, R_OK);
    if (name != "") {untested();
      CMD::command("include " + name, &CARD_LIST::card_list);
    }else{
    }
  }
  //CMD::command("clear", &CARD_LIST::card_list);
  if (!OPT::language) {
    OPT::language = language_dispatcher[DEFAULT_LANGUAGE];

    for(DISPATCHER<LANGUAGE>::const_iterator i=language_dispatcher.begin();
        !OPT::language && i!=language_dispatcher.end(); ++i) {untested();
      OPT::language = prechecked_cast<LANGUAGE*>(i->second);
    }
  }else{untested();
    // already have a language specified in a startup file
  }
  if (OPT::language) {
    OPT::case_insensitive = OPT::language->case_insensitive();
    OPT::units            = OPT::language->units();
  }else{ untested();
    OPT::case_insensitive = false;
    OPT::units            = uSI;
  }
}
/*--------------------------------------------------------------------------*/
/* sig_abrt: trap asserts
*/
extern "C" {
  static void sig_abrt(SIGNALARGS)
  {untested();
    signal(SIGABRT,sig_abrt);
    static int count = 100;
    if (--count > 0 && ENV::run_mode != rBATCH) {untested();
      IO::error << '\n';
      siglongjmp(env.p,1);
    }else{untested();
      exit(1);
    }
  }
}
/*--------------------------------------------------------------------------*/
/* sig_int: what to do on receipt of interrupt signal (SIGINT)
 * cancel batch files, then back to command mode.
 * (actually, control-c trap)
 */
extern "C" {
  static void sig_int(SIGNALARGS)
  {itested();
    signal(SIGINT,sig_int);
    if (ENV::run_mode == rBATCH) {untested();
      exit(1);
    }else{itested();
      IO::error << '\n';
      siglongjmp(env.p,1);
    }
  }
}
/*--------------------------------------------------------------------------*/
extern "C" {
  static void sig_fpe(SIGNALARGS)
  {untested();
    signal(SIGFPE,sig_fpe);
    error(bDANGER, "floating point error\n");
  }
}
/*--------------------------------------------------------------------------*/
static void setup_traps(void)
{
  signal(SIGFPE,sig_fpe);
  signal(SIGINT,sig_int);
  //signal(SIGABRT,sig_abrt);
}
/*--------------------------------------------------------------------------*/
/* finish: clean up after a command
 * deallocates space, closes plot windows, resets i/o redirection, etc.
 * This is done separately for exception handling.
 * If a command aborts, clean-up is still done, leaving a consistent state.
 * //BUG// It is a function to call as a remnant of old C code.
 * Should be in a destructor, so it doesn't need to be explicitly called.
 */
static void finish(void)
{
  plclose();
  outreset();
}
/*--------------------------------------------------------------------------*/
static void process_cmd_line(int argc, char * const*argv)
{
  int opt;
  CS cmd(CS::_STRING, "");

  try{
    while ((opt = getopt(argc, argv, "a:b:c:i:vI:D:U:")) != -1) { untested();
      switch (opt) {
        case 'a': untested();
          CMD::command(std::string("attach ") + optarg, &CARD_LIST::card_list);
          break;
        case 'b': untested();
          incomplete();
          break;
        case 'c': untested();
          cmd = CS(CS::_STRING, optarg);
          CMD::cmdproc(cmd, &CARD_LIST::card_list);
          break;
        case 'v': untested();
          cmd = CS(CS::_STRING, "verilog");
          CMD::cmdproc(cmd, &CARD_LIST::card_list);
          break;
        case 'I': untested();
          CMD::command(std::string("`add_include ") + optarg, &CARD_LIST::card_list);
          break;
        case 'i':
          CMD::command(std::string("`include ") + optarg, &CARD_LIST::card_list);
          break;
        case 'D':
          CMD::command(std::string("`define ") + optarg, &CARD_LIST::card_list);
          break;
        case 'U':
          CMD::command(std::string("`undef ") + optarg, &CARD_LIST::card_list);
          break;
        default:
          printf("options (incomplete):\n"
                 " [-i] <filename>   interactive\n"
                 " -b <filename>     batchmode\n"
                 " -u                this usage summary\n"
                 " -b and -i are mutually exclusive. The last one set wins.\n"
                 );
          exit(1);
      }
    }
  }catch (Exception_Quit& e) { untested();
    throw;
  }catch (Exception& e) {itested();
    // abort command, continue loop
    error(bDANGER, e.message() + '\n');
  }

  trace2("optind", optind, argc);

  if (optind > argc) { untested();
    fprintf(stderr, "Expected argument after options\n");
    exit(EXIT_FAILURE);
  }else if(optind < argc){
    try {
      CMD::command(std::string("include ") + argv[optind++], &CARD_LIST::card_list);
    }catch (Exception& e) {itested();
      error(bDANGER, e.message() + '\n');
      finish();
    }
    if (optind >= argc) {itested();
      //CMD::command("end", &CARD_LIST::card_list);
      throw Exception_Quit("");
    }else{untested();
    }
  }

}
/*--------------------------------------------------------------------------*/
class MAIN{
public:
  explicit MAIN(){
  }
  void uninit(){
    plclose();
    outreset();

    CMD::command("clear", &CARD_LIST::card_list);
    for(auto i: CARD_LIST::card_list){
      unreachable();
      std::cout << i->short_label() << "\n";
    }
    trace1("erase", &CARD_LIST::card_list);
    CARD_LIST::card_list.erase_all();
    CMD::command("delete all", &CARD_LIST::card_list);
    try{
      CMD::command("detach_all", &CARD_LIST::card_list);
    }catch(Exception_CS const& e){
      incomplete();
      std::cerr << e.message();
    }
  }
  ~MAIN() {
    // CARD_LIST::card_list already gone!
    CKT_BASE::_probe_lists = NULL;
    CKT_BASE::_sim = NULL;
  }

  int operator()(int argc, char *const*argv);
private:
  SIM_DATA _sim_data;
  PROBE_LISTS _probe_lists;
};
/*--------------------------------------------------------------------------*/
int main(int argc, char *const* argv)
{
  MAIN main_;
  int r=main_(argc, argv);
  main_.uninit(); // required before ~MAIN
  return r;
}
/*--------------------------------------------------------------------------*/
int MAIN::operator()(int argc, char *const* argv)
{
  prepare_env();
  CKT_BASE::_sim = &_sim_data;
  CKT_BASE::_probe_lists = &_probe_lists;
  try {

    SET_RUN_MODE xx(rBATCH);
    if (!sigsetjmp(env.p, true)) {
      read_startup_files(argv);
      setup_traps();
      try {
        process_cmd_line(argc, argv);
      }catch (Exception_Quit& e) { untested();
        throw;
      }catch (Exception& e) { untested();
        error(bDANGER, e.message() + '\n');
      }
    }else{untested();
      //CMD::command("quit", &CARD_LIST::card_list);
      return 0; // check?
    }

    { untested();
      SET_RUN_MODE xx(rINTERACTIVE);
      CS cmd(CS::_STDIN);
      for (;;) {
        if (!sigsetjmp(env.p, true)) {
          try {
            if (OPT::language) { untested();
              OPT::language->parse_top_item(cmd, &CARD_LIST::card_list);
            }else{untested();
              CMD::cmdproc(cmd.get_line(I_PROMPT), &CARD_LIST::card_list);
            }
          }catch (Exception_End_Of_Input& e) { untested();
            error(bDANGER, e.message() + '\n');
            break;
          }catch (Exception& e) { untested();
            error(bDANGER, e.message() + '\n');
          }
        }else{ untested();
        }
      }
    }
  }catch (Exception_Quit&) { untested();
  }catch (Exception& e) {untested();
    error(bDANGER, e.message() + '\n');
    return 1;
  }
  return 0;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

// vim:ts=8:sw=2:et
