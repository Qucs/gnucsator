/*                      -*- C++ -*-
 * Copyright (C) 2015 Felix Salfelder
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
 * qucs Eqn evaluator component
 */

#include <e_compon.h>
#include <c_comand.h>
#include <u_lang.h>
#include <u_xprobe.h>
#include <globals.h>
#include <map>

#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

namespace{
using std::string;
using std::vector;

class INTERFACE EQN : public COMPONENT {
public:
  explicit EQN();
  ~EQN() {}
protected:
  explicit EQN(const EQN& p);
private: // a default for most elements
  uint_t max_nodes()const     {return 0;}
  uint_t min_nodes()const     {return 0;}
  uint_t matrix_nodes()const  {return 0;}
  uint_t net_nodes()const     {return 0;}
private:
  string dev_type()const {return "Eqn";}
  string value_name()const{return "";}
  string port_name(uint_t)const{return "";}
public: // override virtual
  bool print_type_in_spice()const {return false;}
  void precalc_last();
  void expand();
  void tr_begin() { q_eval();}
  TIME_PAIR tr_review();
//  void tr_restore();
//  void dc_advance();
  void tr_advance();
//  void tr_regress();
  void tr_accept();
  bool tr_needs_eval()const { return true;}

  CARD* clone()const{return new EQN(*this);}
  //void   map_nodes();
  double   tr_probe_num(const string&)const;
  XPROBE   ac_probe_ext(const string&)const;

protected: // inline, below
  bool conv_check()const;
  bool has_ac_eval()const;
  bool using_ac_eval()const;

public: // step control. maybe later?
//  double   tr_review_trunc_error(const FPOLY1* q);
//  double   tr_review_check_and_convert(double timestep);
//  double error_factor()const	{return OPT::trstepcoef[_trsteporder];}
public:
  PARAM_LIST const& params() const{
    return _params;
  }
private:
  PARAM_LIST _params; // map<string,PARAM>, unordered
//  PARAM_LIST _params_extra; // time, temp etc.
  vector<PARAM_LIST::iterator> _param_order; // in order, for evaluation
  PARAMETER<double>* _time_p;

  int param_count()const {return (unsigned(_param_order.size()) + COMPONENT::param_count());}
  void parm_eval();
protected:
  void set_param_by_name(string Name, string Value);
/*--------------------------------------------------------------------------*/
}e1;
/*--------------------------------------------------------------------------*/
DISPATCHER<CARD>::INSTALL
  d1(&device_dispatcher, "Eqn", &e1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static PARAM_LIST::iterator pick(PARAM_LIST& list, const string& name)
{
#if HAVE_PARAM_PICK
  incomplete();
#else
  //BUG: linear search
  for(PARAM_LIST::iterator i=list.begin(); i!=list.end(); ++i){
    if(i->first == name){
      return i;
    }else{
    }
  }
#endif
  unreachable();
  return list.end();
}
/*--------------------------------------------------------------------------*/
EQN::EQN() : COMPONENT(), _time_p(NULL) {}
/*--------------------------------------------------------------------------*/
EQN::EQN(const EQN& p):
  COMPONENT(p),
  _params(p._params),
  _time_p(NULL)
{
  //this is a bit of a hack. but i do need params in order...
  _param_order.resize(p._param_order.size());
  unsigned cnt = 0;
  for(auto o : p._param_order){ untested();
    auto i = pick(_params, o->first);
    _param_order[cnt++] = i;
  }
}
/*--------------------------------------------------------------------------*/
void EQN::set_param_by_name(string Name, string Value)
{
  if(Name=="Export"){
    if(Value=="0"){
    }else{
      incomplete();
    }
  }else{
    PARAMETER<double> p;
    p = Value;
    CS cs(CS::_STRING, Name+"={"+Value+"}");
    trace2("EQN parse", Name, Value);
    _params.parse(cs);
    assert(pick(_params,Name) != _params.end()); // incomplete?
    _param_order.push_back(pick(_params,Name));
    trace1("EQN parse", _params.size());
  }
}
/*--------------------------------------------------------------------------*/
void EQN::expand()
{
  if (!subckt()) {
    new_subckt();
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void EQN::precalc_last()
{
  PARAM_LIST* dummy = new PARAM_LIST;
  subckt()->attach_params(dummy, scope());
  delete dummy;
  subckt()->params()->set_try_again(&_params);
  _params.set_try_again(scope()->params());

  parm_eval();

  for(auto i=_params.begin(); i!=_params.end(); ++i){
    if(i->first == "time"){
      _time_p = &(i->second);
    }
  }
}
/*--------------------------------------------------------------------------*/
XPROBE EQN::ac_probe_ext(const string&)const
{incomplete();
  return XPROBE(NOT_VALID);
}
/*--------------------------------------------------------------------------*/
double EQN::tr_probe_num(const string& what) const
{
  try{
    PARAMETER<double> x = _params[what];
    return x;
  }catch(Exception const&){untested();
    return NOT_VALID;
  }
}
/*--------------------------------------------------------------------------*/
void EQN::tr_advance()
{
}
/*--------------------------------------------------------------------------*/
void EQN::tr_accept()
{
  parm_eval();
}
/*--------------------------------------------------------------------------*/
void EQN::parm_eval()
{
  assert(subckt());
  trace2("eval", _param_order.size(), _params.size());
  // trace1("eval", _params);
  if(_time_p) {
    *_time_p = _sim->_time0;
  }
  for(auto p : _param_order){
    try{
      p->second.e_val(NOT_INPUT, subckt());
      trace3("parm_eval", p->first, p->second, double(p->second));
    }catch(Exception_No_Match const&){
      incomplete();
      trace0("parm_eval incomplete");
    }
  }
}
/*--------------------------------------------------------------------------*/
TIME_PAIR EQN::tr_review()
{
  q_accept();
  return TIME_PAIR();
}
/*--------------------------------------------------------------------------*/
class CMD_EQN : public CMD {
  void do_it(CS& cmd, CARD_LIST* cl) override{
    assert(cl);
    assert(cl->params());
    auto& pl = *cl->params();
    trace1("Eqn::do_it", cmd.fullstring());

    assert(OPT::language);
    CARD* c = device_dispatcher.clone("Eqn");
    auto e = prechecked_cast<EQN*>(c);
    assert(e);
    OPT::language->parse_instance(cmd, e);
    cl->push_back(e);

    for(auto x : e->params()){ // TODO: ordinary param_access.
      trace2("DBG", x.first, x.second.string());
      auto Name = x.first;
      auto Value = x.second.string();
      CS cs(CS::_STRING, Name+"={"+Value+"}");
      pl.parse(cs);
    }
  }
}p0;
DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "Eqn", &p0);
/*--------------------------------------------------------------------------*/
}
// vim:ts=8:sw=2:noet
