/*$Id: d_admit.cc,v 1.6 2010-07-15 10:13:57 felix Exp $ -*- C++ -*-
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
 * admittance devices:
 *	self-admittance (old Y device)
 *		y.x = volts, y.f0 = amps, ev  = y.f1 = mhos.
 *		m.x = volts, m.c0 = amps, acg = m.c1 = mhos.
 *	trans-admittance (VCCS, G device)
 *	voltage controlled admittance
 *		y.x  = volts(control), y.f0 = mhos, ev = y.f1 = mhos/volt
 *		m.x  = volts(control), m.c0 = 0,    acg = m.c1 = mhos
 *		_loss0 == 1/R. (mhos)
 */
//testing=script 2006.07.17
#include "bm.h"
#include "e_elemnt.h"
#include "u_prblst.h"

using std::string;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_PROBE : public ELEMENT {
protected:
  explicit DEV_PROBE(const DEV_PROBE& p) :ELEMENT(p), _reg(false) {}
public:
  explicit DEV_PROBE()	:ELEMENT(), _reg(false) {}
protected: // override virtual
  enum probe_t {VOLTAGE, CURRENT};
  probe_t _type;	/* current or voltage controlled */
  string value_name()const {return "dummy";}
  string dev_type()const	{return string((_type==VOLTAGE)?"V":"I")+"probe";}
  uint_t	   max_nodes()const	{return 2;}
  uint_t	   min_nodes()const	{return 2;}
  uint_t	   matrix_nodes()const	{return 2;}
  uint_t	   net_nodes()const	{return 2;}
  void  set_dev_type(const string& new_type);
  //bool	   has_iv_probe()const  {return true;}
  bool	   use_obsolete_callback_parse()const {return false;}
  CARD*	   clone()const		{return new DEV_PROBE(*this);}
  void     precalc_first();
  void     precalc_last();
  hp_float_t   tr_involts()const	{return tr_outvolts();}
  hp_float_t   tr_involts_limited()const {return tr_outvolts_limited();}
  COMPLEX  ac_involts()const	{untested();return ac_outvolts();}
  virtual void tr_iwant_matrix(){}
  virtual void ac_iwant_matrix(){}
  string port_name(uint_t i)const {
    assert(i != INVALID_NODE);
    assert(i < 2);
    static string names[] = {"p", "n"};
    return names[i];
  }
  virtual void expand();
public:
  string iwant_print() const {
    trace3("DEV_PROBE::iwant_print() calledthe right function",
        long_label(),hp(this), _sim->_mode); 
    return "v(" + long_label() + ")"; }
private:
  bool _reg;
};

/*--------------------------------------------------------------------------*/
void DEV_PROBE::set_dev_type(const string& new_type)
{
  trace2("DEV_PROBE:probe type", long_label(), new_type);
  if(new_type=="VProbe"){ untested();
  }else{ incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::precalc_first()
{
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::precalc_last()
{
  ELEMENT::precalc_last();
  set_constant(true);
  set_converged(true);

  string prb = string((_type==VOLTAGE)?"v":"i");

  if(_reg){untested();
  }else{
    trace1("adding probe", long_label());
    CS p(CS::_STRING, prb+"("+long_label()+")");
    PROBE_LISTS::print[_sim->_mode].add_list(p);
    _reg = true;
  }
}
/*--------------------------------------------------------------------------*/
void DEV_PROBE::expand()
{
  if(_sim->is_first_expand()){ untested();
  }else{untested();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_PROBE p1;
DISPATCHER<CARD>::INSTALL
  d1(&device_dispatcher, "VProbe", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
