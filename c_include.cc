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
 * `include command
 */
#include <c_comand.h>
#include <ap.h>
#include <globals.h>
#include <u_lang.h>
#include <u_parameter.h>
#include <libgen.h> // dirname

std::string gnucap_includepath=".";

namespace {
/*--------------------------------------------------------------------------*/
class CMD_SI : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope) {
    std::string optarg;
    cmd >> optarg;
    trace1("CMD_SI", optarg);
    gnucap_includepath = optarg;
  }
}p2;
DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "set_includepath", &p2);
/*--------------------------------------------------------------------------*/
class CMD_AI : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope) {
    std::string optarg;
    cmd >> optarg;
    trace1("CMD_AI::do_it", optarg);
    gnucap_includepath += std::string(":") + optarg;
  }
}p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "add_includepath", &p1);
/*--------------------------------------------------------------------------*/
class CMD_INCLUDE : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope)
  {
    trace0("CMD_INCLUDE::do_it");
    unsigned here = cmd.cursor();
    char* dirtmp=NULL;
    char* basetmp=NULL;
    char buf[PATH_MAX];
    char* cwd;
    cwd = getcwd(buf, PATH_MAX);
    try {
      std::string file_name;
      cmd >> file_name;
      trace1("include", file_name);

#if 0
      if (file_name.c_str()[0]){ untested();
        if (file_name.c_str()[0] == '$'){ untested();
          trace1("have dollar", file_name);
          PARAMETER<std::string> a(file_name.c_str()+1);
          a.e_val("", Scope);
          if(!(a=="")){ untested();
            file_name = a.value();
          }else{ untested();
          }
        }
      }else{ untested();
        trace1("no dollar", file_name);
      }
#endif
      dirtmp = strdup(file_name.c_str());
      basetmp = strdup(file_name.c_str());

      char* dir=dirname(dirtmp);
      char* base=basename(basetmp);
      trace1("chdir", dir);

      std::string full_file_name=findfile(file_name, gnucap_includepath, R_OK);
      trace3("...", full_file_name, gnucap_includepath, base);
      chdir(dir);

      CS file(CS::_INC_FILE, std::string(full_file_name));
      for (;;) {
        trace2(" CMD_INCLUDE::do_it >", file_name , (OPT::language) );
        OPT::language->parse_top_item(file, Scope);
      }
    }catch (Exception_File_Open& e) { itested();
      cmd.warn(bDANGER, here, e.message() + '\n');
    }catch (Exception_End_Of_Input& e) { itested();
      // done
    }
    free(dirtmp);
    free(basetmp);
    chdir(cwd);
  }
} p0;
DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "`include", &p0);
/*--------------------------------------------------------------------------*/
}
// vim:ts=8:sw=2:et
