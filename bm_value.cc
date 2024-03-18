/*$Id: bm_value.cc,v 26.137 2010/04/10 02:37:33 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *           (C) 2015 Felix Salfelder
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
 * behavioral modeling simple value
 * used with tc, etc, and conditionals
 *
 * QUCS variation
 */
#include "globals.h"
#include "bm.h"

#include <map>              // parameter dictionaries
#include "boost/assign.hpp" // initialization templates
/*--------------------------------------------------------------------------*/
using std::string;
using std::map;
namespace{
/*--------------------------------------------------------------------------*/
class EVAL_BM_VALUE : public EVAL_BM_ACTION_BASE {
private:
  explicit	EVAL_BM_VALUE(const EVAL_BM_VALUE& p):EVAL_BM_ACTION_BASE(p) {}
  static std::map<string, const char*> _param_dict;
public:
  explicit      EVAL_BM_VALUE(int c=0) :EVAL_BM_ACTION_BASE(c) {
    trace0("EVAL_BM_VALUE(c)");
  }
		~EVAL_BM_VALUE()	{ trace0("~EVAL_BM_VALUE");}
private: // override virtual
  bool		operator==(const COMMON_COMPONENT&)const override;
  COMMON_COMPONENT* clone()const override { untested(); return new EVAL_BM_VALUE(*this);}
  void		print_common_obsolete_callback(OMSTREAM&, LANGUAGE*)const override;
  bool		is_trivial()const override;

  void		precalc_first(const CARD_LIST*)override;
  void		tr_eval(ELEMENT*)const override;
  std::string	name()const override {return "value";}
  bool		ac_too()const override {return false;}
  bool		parse_numlist(CS&) override;
//  bool  	parse_params_obsolete_callback(CS&);
  bool is_constant()const{return true;}
  int set_param_by_name(string Name, string Value)override;
  // doesnt make sense. set value through device
  // void   set_param_by_name(string Name, string Value);
};
static EVAL_BM_VALUE p1(CC_STATIC);
static DISPATCHER<COMMON_COMPONENT>::INSTALL d1(&bm_dispatcher, "qucs_value", &p1);
/*--------------------------------------------------------------------------*/
bool EVAL_BM_VALUE::operator==(const COMMON_COMPONENT& x)const
{ untested();
  const EVAL_BM_VALUE* p = dynamic_cast<const EVAL_BM_VALUE*>(&x);
  return  p && EVAL_BM_ACTION_BASE::operator==(x);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_VALUE::print_common_obsolete_callback(OMSTREAM& o, LANGUAGE*)const
{ untested();
  o << " INCOMPLETE. do not have spice representation (yet).";
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_VALUE::is_trivial()const
{ untested();
  return  !(_bandwidth.has_hard_value()
	    || _delay.has_hard_value()
	    || _phase.has_hard_value()
	    || _ooffset.has_hard_value()
	    || _ioffset.has_hard_value()
	    || _scale.has_hard_value()
	    || _tc1.has_hard_value()
	    || _tc2.has_hard_value()
	    || _ic.has_hard_value()
	    || _tnom_c.has_hard_value()
	    || _dtemp.has_hard_value()
	    || _temp_c.has_hard_value());
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_VALUE::precalc_first(const CARD_LIST* Scope)
{ untested();
  trace2("QUCS_VALUE, precalc_first", modelname(), _value);
  if (modelname() != "") { untested();
    // no. we use set_param stuff.
  //  _value = modelname();
  }else{ untested();
  }
  EVAL_BM_ACTION_BASE::precalc_first(Scope);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_VALUE::tr_eval(ELEMENT* d)const
{ untested();
  tr_finish_tdv(d, _value);
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_VALUE::parse_numlist(CS& cmd)
{ untested();
  size_t here = cmd.cursor();
  PARAMETER<double> new_value(NOT_VALID);
  cmd >> new_value;
  if (cmd.gotit(here)) { untested();
    _value = new_value;
    return true;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
// from EVAL_BM_ACTION_BASE
// changed to qucs names.
map<string, const char*> EVAL_BM_VALUE::_param_dict =
  boost::assign::map_list_of
  ("Tc1", "tc1")
  ("Tc2", "tc2")
  ("Temp", "temp")
  ("ic", "ic");
/*--------------------------------------------------------------------------*/
int EVAL_BM_VALUE::set_param_by_name(string Name, string Value)
{ untested();
  const char* n = _param_dict[Name];
  if(n) { untested();
    Name = n;
  }else{ untested();
  }
  trace2("value wrapper", Name, Value);
  return EVAL_BM_ACTION_BASE::set_param_by_name(Name, Value);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
// vim:ts=8:sw=2:noet:
