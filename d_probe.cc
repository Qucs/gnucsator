/*                         -*- C++ -*-
 * Copyright (C) 2012 Fabian Vallon
 *               2012-2015 Felix Salfelder
 * Authors: Fabian Vallon
 *          Felix Salfelder <felix@salfelder.org>
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
 * qucs probes
 */
#include <globals.h>
#include <bm.h>
#include <e_elemnt.h>
#include <u_prblst.h>

#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

#include "u_prblst.cc"

using std::string;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_PROBE : public ELEMENT {
protected:
  explicit DEV_PROBE(const DEV_PROBE& p) :ELEMENT(p), _type(p._type), _reg(false) {}
public:
  explicit DEV_PROBE()	:ELEMENT(), _reg(false) {}
protected: // override virtual
  enum probe_t { VOLTAGE, // voltage probe
                 CURRENT  // current probe (short)
               } _type;
  string value_name()const override{return "dummy";}
  string dev_type()const override{return string((_type==VOLTAGE)?"V":"I")+"Probe";}
  uint_t	   max_nodes()const override{return 2;}
  uint_t	   min_nodes()const override{return 2;}
  uint_t	   matrix_nodes()const override{return 2;}
  uint_t	   net_nodes()const override{return 2;}
  void  set_dev_type(const string& new_type) override;
  //bool	   has_iv_probe()const  {return true;}
  bool	   use_obsolete_callback_parse()const override{return false;}
  CARD*	   clone()const override{return new DEV_PROBE(*this);}
//  void     precalc_first();
  void     precalc_last()override;
  COMPLEX  ac_involts()const override{untested();return ac_outvolts();}
  bool is_device() const override {
    return true;
  }
  void tr_iwant_matrix() override;
  void tr_begin() override;
//  bool do_tr();
  void tr_load() override;
  void tr_unload() override;
  double tr_involts()const override {return tr_outvolts();}
  double tr_involts_limited()const override {return tr_outvolts_limited();}
  void ac_iwant_matrix()override;
  void ac_begin()override{incomplete();}
  void do_ac()override{incomplete();}
  void ac_load()override{incomplete();}
  string port_name(uint_t i)const override{
    assert(i != INVALID_NODE);
    assert(i < 2);
    static string names[] = {"p", "n"};
    return names[i];
  }
  void expand() override;
public:
  string iwant_print() const { untested();
    trace3("DEV_PROBE::iwant_print() calledthe right function",
        long_label(), this, _sim->_mode);
    return "v(" + long_label() + ")"; }
private:
  bool _reg;
};

/*--------------------------------------------------------------------------*/
void DEV_PROBE::set_dev_type(const string& new_type)
{
  trace2("DEV_PROBE:probe type", long_label(), new_type);
  if(new_type=="VProbe"){
    _type = VOLTAGE;
  }else if(new_type=="IProbe"){
    _type = CURRENT;
  }else{ unreachable(); incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::precalc_last()
{
  ELEMENT::precalc_last();
  set_constant(true);
  set_converged(true);

 string prb = string((_type==VOLTAGE)?"v":"i");

 if(_reg){
 }else if(owner()){
   // ignore probes in subcircuits, just as qucsator does
   // (good idea?)
 }else{
   trace3("adding probe", prb, _sim->_mode, long_label());
   CS p(CS::_STRING, prb+"("+short_label()+")");
   assert(scope());
   // _probe_lists->print[_sim->_mode].add_list(p, scope());
   _probe_lists->print[s_OP].add_list(p, scope());
   p.reset();
   _probe_lists->print[s_DC].add_list(p, scope());
   p.reset();
   _probe_lists->print[s_TRAN].add_list(p, scope());
   _reg = true;
 }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::expand()
{
  string prb = string((_type==VOLTAGE)?"v":"i");
  if(_sim->is_first_expand()){
  }else{untested();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::tr_iwant_matrix()
{
  switch(_type){
    case VOLTAGE:
      break;
    case CURRENT:
      tr_iwant_matrix_passive();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::tr_begin()
{
  ELEMENT::tr_begin();
  assert(is_constant());

  if(_type==CURRENT) {
    // from d_res
    _y1.f1 = _y[0].f1 = OPT::shortckt;
    _m0.x  = _y[0].x;
    _m0.c1 = 1./_y[0].f1;
    _m0.c0 = 0.;
    _m1 = _m0;
  }
  assert(_loss0 == 0.);
  assert(_loss1 == 0.);
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::ac_iwant_matrix()
{
  switch(_type) {
    case VOLTAGE:
      break;
    case CURRENT:
      ac_iwant_matrix_passive();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::tr_load()
{
  switch(_type) {
    case VOLTAGE:
      break;
    case CURRENT:
      tr_load_passive();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::tr_unload()
{ untested();
  switch(_type) {
    case VOLTAGE: untested();
      break;
    case CURRENT: untested();
      tr_unload_passive();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_PROBE p1;
DISPATCHER<CARD>::INSTALL
  d1(&device_dispatcher, "VProbe|IProbe", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
