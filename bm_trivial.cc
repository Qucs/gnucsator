/*$Id: bm_value.cc,v 26.137 2010/04/10 02:37:33 al Exp $ -*- C++ -*-
 * Copyright (C) 2015 Felix Salfelder
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
 * behavioral modeling trivial value
 * used to translate parameter names from QUCS.
 */
#include "globals.h"
#include "bm.h"
#include "e_elemnt.h"

/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
using std::string;
/*--------------------------------------------------------------------------*/
class EVAL_BM_TRIVIAL : public EVAL_BM_BASE {
private:
  const string   _name;
  explicit	EVAL_BM_TRIVIAL(const EVAL_BM_TRIVIAL& p):EVAL_BM_BASE(p), _name(p._name) {}
public:
  explicit      EVAL_BM_TRIVIAL(int c=0, string name="value") :EVAL_BM_BASE(c), _name(name) {
    trace0("EVAL_BM_TRIVIAL(c)");
  }
		~EVAL_BM_TRIVIAL()	{ trace0("~EVAL_BM_TRIVIAL");}
private: // override virtual
  bool		operator==(const COMMON_COMPONENT&)const;
  COMMON_COMPONENT* clone()const	{ untested(); return new EVAL_BM_TRIVIAL(*this);}
  void		print_common_obsolete_callback(OMSTREAM&, LANGUAGE*)const;
  bool		is_trivial()const;

  void		precalc_first(const CARD_LIST*);
  void		tr_eval(ELEMENT*)const;
  string	name()const		{return _name;}
  bool		ac_too()const		{return false;}
  bool		parse_numlist(CS&);
//  bool  	parse_params_obsolete_callback(CS&);
  bool is_constant()const{return true;}
  void  	set_param_by_name(string Name, string Value);
  // doesnt make sense. set value through device
  // void   set_param_by_name(string Name, string Value);
};
static EVAL_BM_TRIVIAL p1(CC_STATIC,"U");
static DISPATCHER<COMMON_COMPONENT>::INSTALL d1(&bm_dispatcher, "qucs_trivial_U", &p1);
/*--------------------------------------------------------------------------*/
bool EVAL_BM_TRIVIAL::operator==(const COMMON_COMPONENT& x)const
{ untested();
  const EVAL_BM_TRIVIAL* p = dynamic_cast<const EVAL_BM_TRIVIAL*>(&x);
  return  p && EVAL_BM_BASE::operator==(x);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_TRIVIAL::print_common_obsolete_callback(OMSTREAM& o, LANGUAGE*)const
{ untested();
  o << " INCOMPLETE. do not have spice representation (yet).";
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_TRIVIAL::is_trivial()const
{ untested();
  return true;
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_TRIVIAL::precalc_first(const CARD_LIST* Scope)
{ untested();
  trace2("QUCS_TRIVIAL, precalc_first", modelname(), _value);
  if (modelname() != "") {
    // no. we use set_param stuff.
  //  _value = modelname();
  }else{
  }
  EVAL_BM_BASE::precalc_first(Scope);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_TRIVIAL::tr_eval(ELEMENT* d)const
{ untested();
  double val = _value;
  d->_y[0] = FPOLY1(CPOLY1(d->_y[0].x, 0., val));
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_TRIVIAL::parse_numlist(CS& cmd)
{ untested();
  unsigned here = cmd.cursor();
  PARAMETER<double> new_value(NOT_VALID);
  cmd >> new_value;
  if (cmd.gotit(here)) {
    _value = new_value;
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_TRIVIAL::set_param_by_name(string Name, string Value)
{ untested();
  if(Umatch(_name,Name)) {
    _value = Value;
  }else{
    EVAL_BM_BASE::set_param_by_name(Name, Value);
  }
  trace2("value wrapper", Name, Value);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
// vim:ts=8:sw=2:noet:
