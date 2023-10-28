/*$Id: d_subckt.h  2016/09/17 $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
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
 * data structures for subcircuits, temporary qucsator version
 */
#ifndef Q_PARAMLIST_H
#define Q_PARAMLIST_H
#include "e_compon.h"
#include "u_paramlist.h"
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class INTERFACE Q_PARAMLIST : public COMMON_COMPONENT {
private:
  explicit Q_PARAMLIST(const Q_PARAMLIST& p)
    :COMMON_COMPONENT(p), _params(p._params) {/*++_count;*/}
public:
  explicit Q_PARAMLIST(int c=0)	:COMMON_COMPONENT(c) {/*++_count;*/}
	   ~Q_PARAMLIST()		{/*--_count;*/}
  bool operator==(const COMMON_COMPONENT&)const override;
  COMMON_COMPONENT* clone()const override{return new Q_PARAMLIST(*this);}
  static int	count()			{untested();return 0;}

  int set_param_by_name(std::string Name, std::string Value) override;
  std::string param_by_name(std::string const& Name)const /*override*/;
  bool		param_is_printable(int)const override;
  std::string	param_name(int)const override;
  std::string	param_name(int,int)const override;
  std::string	param_value(int)const override;
  int param_count()const override
	{return (static_cast<int>(_params.size()) + COMMON_COMPONENT::param_count());}

  void		precalc_first(const CARD_LIST*) override;
  void		precalc_last(const CARD_LIST*) override;
private:
  std::string	name()const override	{untested();return "";}
  static int	_count;
public:
  PARAM_LIST_Q	_params;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#define COMMON_PARAMLIST Q_PARAMLIST
inline bool COMMON_PARAMLIST::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_PARAMLIST* p = dynamic_cast<const COMMON_PARAMLIST*>(&x);
  bool rv = p 
    && _params == p->_params
    && COMMON_COMPONENT::operator==(x);

  trace3("compare Q_PARAMLIST", _params.size(), p->_params.size(), rv);
  return rv;
}
/*--------------------------------------------------------------------------*/
inline bool COMMON_PARAMLIST::param_is_printable(int i)const
{
  assert(i < COMMON_PARAMLIST::param_count());
  if (i >= COMMON_COMPONENT::param_count()) {
    return _params.is_printable(COMMON_PARAMLIST::param_count() - 1 - i);
  }else{
    return COMMON_COMPONENT::param_is_printable(i);
  }
}
/*--------------------------------------------------------------------------*/
inline std::string COMMON_PARAMLIST::param_name(int i)const
{
  assert(i < COMMON_PARAMLIST::param_count());
  if (i >= COMMON_COMPONENT::param_count()) {
    return _params.name(COMMON_PARAMLIST::param_count() - 1 - i);
  }else{itested();
    return COMMON_COMPONENT::param_name(i);
  }
}
/*--------------------------------------------------------------------------*/
inline std::string COMMON_PARAMLIST::param_name(int i, int j)const
{untested();
  assert(i < COMMON_PARAMLIST::param_count());
  if (j == 0) {untested();
    return param_name(i);
  }else if (i >= COMMON_COMPONENT::param_count()) {untested();
    return "";
  }else{untested();
    return COMMON_COMPONENT::param_name(i);
  }
}
/*--------------------------------------------------------------------------*/
inline std::string COMMON_PARAMLIST::param_value(int i)const
{
  assert(i < COMMON_PARAMLIST::param_count());
  if (i >= COMMON_COMPONENT::param_count()) {
    return _params.value(COMMON_PARAMLIST::param_count() - 1 - i);
  }else{itested();
    return COMMON_COMPONENT::param_value(i);
  }
}
/*--------------------------------------------------------------------------*/
inline void COMMON_PARAMLIST::precalc_first(const CARD_LIST* Scope)
{
  assert(Scope);
  COMMON_COMPONENT::precalc_first(Scope);
  _mfactor = _params.deep_lookup("m");
  //BUG//  _mfactor must be in precalc_first
}
/*--------------------------------------------------------------------------*/
inline void COMMON_PARAMLIST::precalc_last(const CARD_LIST* Scope)
{
  assert(Scope);
  COMMON_COMPONENT::precalc_last(Scope);

  for (PARAM_LIST_Q::iterator i = _params.begin(); i != _params.end(); ++i) {
    i->second.e_val(NOT_INPUT,Scope);
  }
}
/*--------------------------------------------------------------------------*/
inline int COMMON_PARAMLIST::set_param_by_name(std::string Name, std::string Value)
{
  std::string old = "not_set";
  auto x = _params.find(Name);
  if(x!=_params.end()){
    old = x->second.string();
  }else{
  }
  trace3("CPL::spbn", Name, Value, old);

  _params.set(Name, Value);
  return 0; // TODO
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#undef Q_PARAMLIST
#endif
// vim:ts=8:sw=2:noet:
