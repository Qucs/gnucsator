/*$Id: d_admit.cc,v 1.6 2010-07-15 10:13:57 felix Exp $ -*- C++ -*-
 * vim:ts=8:sw=2:et
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
#include "e_elemnt.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_VPROBE : public ELEMENT {
protected:
  explicit DEV_VPROBE(const DEV_VPROBE& p) :ELEMENT(p) {}
public:
  explicit DEV_VPROBE()	:ELEMENT() {}
protected: // override virtual
  std::string value_name()const {return "dummy";}
  std::string dev_type()const	{return "vprobe";}
  uint_t	   max_nodes()const	{return 2;}
  uint_t	   min_nodes()const	{return 2;}
  uint_t	   matrix_nodes()const	{return 2;}
  uint_t	   net_nodes()const	{return 2;}
  //bool	   has_iv_probe()const  {return true;}
  bool	   use_obsolete_callback_parse()const {return false;}
  CARD*	   clone()const		{return new DEV_VPROBE(*this);}
  void     precalc_last();
  hp_float_t   tr_involts()const	{return tr_outvolts();}
  hp_float_t   tr_involts_limited()const {return tr_outvolts_limited();}
  COMPLEX  ac_involts()const	{untested();return ac_outvolts();}
  virtual void tr_iwant_matrix(){}
  virtual void ac_iwant_matrix(){}
  std::string port_name(uint_t i)const {
    assert(i != INVALID_NODE);
    assert(i < 2);
    static std::string names[] = {"p", "n"};
    return names[i];
  }
public:
  std::string iwant_print() const {
    trace3("DEV_VPROBE::iwant_print() calledthe right function",
        long_label(),hp(this), _sim->_mode); 

return "v(" + long_label() + ")"; }
};

/*--------------------------------------------------------------------------*/
void DEV_VPROBE::precalc_last()
{
  ELEMENT::precalc_last();
  set_constant(true);
  set_converged(true);
}


/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_VPROBE p1;
DISPATCHER<CARD>::INSTALL
  d1(&device_dispatcher, "VProbe", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

// - FABIAN - hier eine unterklasse vprobe hinzufügen
