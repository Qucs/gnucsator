/*                             -*- C++ -*-
 * Copyright (C) 2005 Al Davis
 *               2022 Felix Salfelder
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
 */
#include "c_comand.h"
#include "u_parameter.h"
#include "globals.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static const std::string IS_VALID = "_..is_valid";
/*--------------------------------------------------------------------------*/
class PARAM_PARSE : public PARAMETER<double> {
public:
  explicit PARAM_PARSE() : PARAMETER<double>() { }
public:
  void	operator=(const std::string& s) { untested();
    PARAMETER<double>::operator=(s);
  }
public:
  void parse(CS& cmd){
  double new_val;
  cmd >> new_val;
  if (cmd) {
    PARAMETER<double>::operator=(new_val);
    assert(_s == "#");
  }else{
    std::string name;
    //cmd >> name;
    name = cmd.ctos(",=();", "'{\"", "'}\"");
    if (cmd) {
      if (cmd.match1('(')) { untested();
	_s = name + '(' + cmd.ctos("", "(", ")") + ')';
      }else{
	_s = name;
      }
      if (name == "NA") {untested();
        _s = "";
      }else{
      }
    }else{ untested();
    }
  }
  }
};
/*--------------------------------------------------------------------------*/
class CMD_PARAM : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope) override {
    PARAM_LIST* pl = Scope->params();
    assert(pl);
    if (cmd.is_end()) { untested();
      pl->print(IO::mstdout, OPT::language);
      IO::mstdout << '\n';
    }else{
      trace1("parameterparse", cmd.tail());
      parse(cmd, pl);
    }
  }

  void parse(CS& cmd, PARAM_LIST* pl) {
    (cmd >> "real |integer "); // ignore type
    size_t here = cmd.cursor();
    for (;;) {
      if (!(cmd.more() && (cmd.is_alpha() || cmd.match1('_')))) {
	break;
      }else{
      }
      std::string Name;
      PARAM_PARSE Value;
      std::string range_type;
      cmd >> Name;
      if(cmd >> "="){
	Value.parse(cmd);
      }else{
      }

      std::string range_expr = "1";

      for(;;){
	if(cmd >> "from "){ untested();
	  range_expr = range_expr + "*(";
	}else if(cmd>> "exclude ") { untested();
	  range_expr = range_expr + "*(1-";
	}else{
	  break;
	}

	range_type = cmd.trimmed_last_match();
	trace2("found1", Value, range_type);
	std::string what;
	std::string lb, ub, uo, lo;
	bool range = true;
	if(cmd.skip1('[')){ untested();
	  lo = "<=";
	}else if(cmd.skip1('(')){ untested();
	  lo = "<";
	}else{ untested();
	  range = false;
	}

	if(range){ untested();
	  lb = cmd.ctos(":");
	  cmd.skip1(":");
	  ub = cmd.ctos("])");
	  trace2("bounds", lb, ub);

	  if(cmd.skip1(']')){ untested();
	    uo = "<=";
	  }else if(cmd.skip1(')')){ untested();
	    uo = "<";
	  }else{ untested();
	  }
	  what = "(" + lb + lo + Name + ")*(" + Name + uo + ub + ")";
	}else{ untested();
	  cmd >> what;
	  what = "(" + Name + "==" + what + ")";
	}
	trace3("found1", Value, range_type, what);
	range_expr = range_expr + what + ")";
	if(cmd.skip1(',')){ untested();
	  break;
	}else{ untested();
	}
      }
//      range_expr = range_expr + "}";

      if (cmd.stuck(&here)) {untested();
	break;
      }else{
      }

      trace4("set", pl, Name, Value, range_expr);
      _range = range_expr;
      pl->set(Name, Value.string());

      PARAMETER<double> v = pl->deep_lookup(IS_VALID);
      if(v.has_hard_value()){
	range_expr = v.string() + "*" + range_expr;
	pl->set(IS_VALID, range_expr);
      }else{
	pl->set(IS_VALID, range_expr);
      }
      trace3("valid?", Name, Value, range_expr);
    }
    cmd.check(bDANGER, "syntax error");
  }
  std::string const& range() const {return _range;}
private:
  std::string _range;
} p;
DISPATCHER<CMD>::INSTALL d(&command_dispatcher, "param|parameter", &p);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
