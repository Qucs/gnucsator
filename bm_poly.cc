/*                             -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
 * HSPICE compatible POLY
 */
//testing=script,complete 2005.10.06
#include "u_lang.h"
#include "e_elemnt.h"
#include "bm.h"
#include <globals.h>
#include <boost/assign.hpp> // initialization templates
/*--------------------------------------------------------------------------*/
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
const double _default_max(BIGBIG);
const double _default_min(-BIGBIG);
const bool   _default_abs(false);
const bool   _default_degree(0);
/*--------------------------------------------------------------------------*/
class EVAL_BM_POLY : public EVAL_BM_ACTION_BASE {
private:
  PARAMETER<double> _min;
  PARAMETER<double> _max;
  PARAMETER<bool>   _abs;
  static std::map<std::string, PARA_BASE EVAL_BM_POLY::*> param_dict;
  std::vector<PARAMETER<double> > _c;
  PARAMETER<unsigned> _degree;
  explicit	EVAL_BM_POLY(const EVAL_BM_POLY& p);
public:
  explicit      EVAL_BM_POLY(int c=0);
		~EVAL_BM_POLY()		{}
private: // override vitrual
  bool		operator==(const COMMON_COMPONENT&)const override;
  COMMON_COMPONENT* clone()const override {return new EVAL_BM_POLY(*this);}
  void		print_common_obsolete_callback(OMSTREAM&, LANGUAGE*)const override;
// not yet  bool use_obsolete_callback_print()const {return false;}

  void		precalc_last(const CARD_LIST*)override;
  void		tr_eval(ELEMENT*)const override;
  std::string	name()const override {return "poly";}
  bool		ac_too()const override {untested();return false;}
  bool		parse_numlist(CS&)override;
  int		set_param_by_name(std::string Name, std::string Value)override;
  bool		parse_params_obsolete_callback(CS&)override;
  void		skip_type_tail(CS& cmd)const override{cmd.umatch("(1)");}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
EVAL_BM_POLY::EVAL_BM_POLY(int c)
  :EVAL_BM_ACTION_BASE(c),
   _min(_default_min),
   _max(_default_max),
   _abs(_default_abs),
   _degree(_default_degree)
{
}
/*--------------------------------------------------------------------------*/
EVAL_BM_POLY::EVAL_BM_POLY(const EVAL_BM_POLY& p)
  :EVAL_BM_ACTION_BASE(p),
   _min(p._min),
   _max(p._max),
   _abs(p._abs),
   _c(p._c),
   _degree(p._degree)
{
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_POLY::operator==(const COMMON_COMPONENT& x)const
{
  const EVAL_BM_POLY* p = dynamic_cast<const EVAL_BM_POLY*>(&x);
  bool rv = p
    && _min == p->_min
    && _max == p->_max
    && _abs == p->_abs
    && _c == p->_c
    && _degree == p->_degree
    && EVAL_BM_ACTION_BASE::operator==(x);
  return rv;
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_POLY::print_common_obsolete_callback(OMSTREAM& o, LANGUAGE* lang)const
{
  assert(lang);
  o << ' ' << name() << '(';
  for (std::vector<PARAMETER<double> >::const_iterator
	 p = _c.begin();  p != _c.end();  ++p) {
    o << *p << ' ';
  }
  o << ')';
  print_pair(o, lang, "min", _min, _min.has_hard_value());
  print_pair(o, lang, "max", _max, _max.has_hard_value());
  print_pair(o, lang, "abs", _abs, _abs.has_hard_value());
  EVAL_BM_ACTION_BASE::print_common_obsolete_callback(o, lang);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_POLY::precalc_last(const CARD_LIST* Scope)
{
  assert(Scope);
  EVAL_BM_ACTION_BASE::precalc_last(Scope);

  for (std::vector<PARAMETER<double> >::const_iterator
	 p = _c.begin();  p != _c.end();  ++p) {
    (*p).e_val(0, Scope);
  }
  _min.e_val(_default_min, Scope);
  _max.e_val(_default_max, Scope);
  _abs.e_val(_default_abs, Scope);
}
/*--------------------------------------------------------------------------*/
void EVAL_BM_POLY::tr_eval(ELEMENT* d)const
{
  double x = ioffset(d->_y[0].x);
  double f0 = 0.;
  double f1 = 0.;
  assert(_c.size());
  for (size_t i=_c.size()-1; i>0; --i) {
    f0 += _c[i];
    f0 *= x;
    f1 *= x;
    f1 += _c[i]*int(i);
  }
  f0 += _c[0];

  if (_abs && f0 < 0) {
    f0 = -f0;
    f1 = -f1;
  }

  if (f0 > _max) {
    f0 = _max;
    f1 = 0;
  }else if (f0 < _min) {
    f0 = _min;
    f1 = 0;
  }

  d->_y[0] = FPOLY1(x, f0, f1);
  tr_final_adjust(&(d->_y[0]), d->f_is_value());
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_POLY::parse_numlist(CS& cmd)
{
  size_t start = cmd.cursor();
  size_t here = cmd.cursor();
  for (;;) {
    size_t old_here = here;
    PARAMETER<double> val;
    cmd >> val;
    if (cmd.stuck(&here)) {
      // no more, graceful finish
      break;
    }else{
      if (cmd.match1('=')) {
	// got one that doesn't belong, back up
	cmd.reset(old_here);
	break;
      }else{
	_c.push_back(val);
      }
    }
  }
  if (cmd.gotit(start)) {
  }else{ untested();
  }
  return cmd.gotit(start);
}
/*--------------------------------------------------------------------------*/
std::map<std::string, PARA_BASE EVAL_BM_POLY::*> EVAL_BM_POLY::param_dict = 
  boost::assign::map_list_of
    ("min",  (PARA_BASE EVAL_BM_POLY::*) &EVAL_BM_POLY::_min)
    ("max",  (PARA_BASE EVAL_BM_POLY::*) &EVAL_BM_POLY::_max)
    ("abs",  (PARA_BASE EVAL_BM_POLY::*) &EVAL_BM_POLY::_abs);
/*--------------------------------------------------------------------------*/
int EVAL_BM_POLY::set_param_by_name(std::string Name, std::string Value)
{
  PARA_BASE EVAL_BM_POLY::* x = (param_dict[Name]);
  if(x) {
    PARA_BASE* p = &(this->*x);
    *p = Value;
    return 0; // TODO
  } else {
    return EVAL_BM_ACTION_BASE::set_param_by_name(Name, Value);
  }
}
/*--------------------------------------------------------------------------*/
bool EVAL_BM_POLY::parse_params_obsolete_callback(CS& cmd)
{
  return ONE_OF
    || Get(cmd, "min", &_min)
    || Get(cmd, "max", &_max)
    || Get(cmd, "abs", &_abs)
    || EVAL_BM_ACTION_BASE::parse_params_obsolete_callback(cmd)
    ;
}
/*--------------------------------------------------------------------------*/
#if 0
void EVAL_BM_POLY::parse_type_tail(CS& cmd) {
  untested();
  if(cmd.umatch("(1)")){
    _degree = 1;
  }else{
    untested();
  }
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
EVAL_BM_POLY p1(CC_STATIC);
DISPATCHER<COMMON_COMPONENT>::INSTALL d1(&bm_dispatcher, "poly", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
