/*$Id: d_subckt.cc   $ -*- C++ -*-
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
 * subcircuit stuff
 * base class for other elements using internal subckts
 * netlist syntax:
 * device: Xxxxx <nodelist> <subckt-name> <args>
 * model:  .subckt <subckt-name> <nodelist>
 *	   (device cards)
 *	   .ends <subckt-name>
 * storage note ...
 * the .subckt always has a comment at the hook point, so a for loop works
 * the expansion (attact to the X) has all comments removed
 *	- need to process the entire ring - for doesn't work
 *
 *  *** MODIFIED version. changed parameter resolution ***
 *  *** see DEV_SUBCKT_PROTO::precalc_first            ***
 */
//testing=script 2016.09.16
#include "u_nodemap.h"
#include "e_node.h"
#include "globals.h"
#include "e_paramlist.h"
#include "e_subckt.h"
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static COMMON_PARAMLIST Default_SUBCKT(CC_STATIC);
#define PORTS_PER_SUBCKT 100
//BUG// fixed limit on number of ports
/*--------------------------------------------------------------------------*/
class DEV_SUBCKT : public BASE_SUBCKT {
  friend class DEV_SUBCKT_PROTO;
private:
  explicit	DEV_SUBCKT(const DEV_SUBCKT&);
public:
  explicit	DEV_SUBCKT();
		~DEV_SUBCKT()		{--_count;}
  CARD*		clone()const override	{return new DEV_SUBCKT(*this);}
private: // override virtual
  char		id_letter()const override	{ untested();return 'X';}
  bool		print_type_in_spice()const override {return true;}
  std::string   value_name()const override	{return "#";}
  int		max_nodes()const override	{return PORTS_PER_SUBCKT;}
  int		min_nodes()const override	{return 0;}
  int		matrix_nodes()const override	{return 0;}
  int		net_nodes()const override	{return _net_nodes;}
  void		precalc_first()override;
  bool		makes_own_scope()const override  { untested();return false;}
  bool		is_valid() const override;

  void		expand() override;
private:
  void		precalc_last()override;
  double	tr_probe_num(const std::string&)const override;
  int param_count_dont_print()const override { untested();return common()->COMMON_COMPONENT::param_count();}

  std::string port_name(int i)const override;
public:
  static int	count()			{untested();return _count;}
protected:
  const COMPONENT* _parent;
private:
  node_t	_nodes[PORTS_PER_SUBCKT];
  static int	_count;
} p1;
int DEV_SUBCKT::_count = -1;
/*--------------------------------------------------------------------------*/
class DEV_SUBCKT_PROTO : public DEV_SUBCKT {
private:
  explicit	DEV_SUBCKT_PROTO(const DEV_SUBCKT_PROTO&p);
public:
  explicit	DEV_SUBCKT_PROTO();
		~DEV_SUBCKT_PROTO(){}
public: // override virtual
  char		id_letter()const override	{untested();return '\0';}
  CARD*		clone_instance()const override;
  bool		print_type_in_spice()const override { untested();unreachable(); return false;}
  std::string   value_name()const override	{untested();incomplete(); return "";}
  std::string   dev_type()const override		{untested(); return "";}
  int		max_nodes()const override	{return PORTS_PER_SUBCKT;}
  int		min_nodes()const override	{return 0;}
  int		matrix_nodes()const override	{untested();return 0;}
  int		net_nodes()const override	{return _net_nodes;}
  CARD*		clone()const override		{return new DEV_SUBCKT_PROTO(*this);}
  bool		is_device()const override	{return false;}
  bool		makes_own_scope()const override	{return true;}
  CARD_LIST*	   scope()override		{return subckt();}
  const CARD_LIST* scope()const override	{return subckt();}
private: // no-ops for prototype
  void precalc_first()override;
  void expand()override {}
  void precalc_last()override {}
  void map_nodes()override {}
  void tr_begin()override {}
  void tr_load()override {}
  TIME_PAIR tr_review() override { return TIME_PAIR(NEVER, NEVER);}
  void tr_accept()override {}
  void tr_advance()override {}
  void tr_restore()override { untested();}
  void tr_regress()override {}
  void dc_final()override {}
  void tr_final()override {}
  void dc_advance()override {}
  void ac_begin()override {}
  void do_ac()override {}
  void ac_load()override {}
  void ac_final()override {}
  bool do_tr()override { untested(); return true;}
  bool tr_needs_eval()const override {untested(); return false;}
  void tr_queue_eval()override {}
  std::string port_name(int)const override{untested();return "";}
} pp;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "X|subckt", &pp);
/*--------------------------------------------------------------------------*/
void DEV_SUBCKT_PROTO::precalc_first()
{
  BASE_SUBCKT::precalc_first();
  CARD_LIST* scope;
  if(owner()){ untested();
    scope = owner()->subckt();
  }else{
    scope = &CARD_LIST::card_list;
  }

  PARAM_LIST* pl = const_cast<PARAM_LIST*>(scope->params());
  subckt()->params()->set_try_again(pl);

  // deal with nested subckt protos.
  for(auto i : *subckt()){
    if(dynamic_cast<DEV_SUBCKT_PROTO*>(i)){
      i->precalc_first();
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO(const DEV_SUBCKT_PROTO& p)
  :DEV_SUBCKT(p)
{
  new_subckt();
}
/*--------------------------------------------------------------------------*/
DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO()
  :DEV_SUBCKT()
{
  new_subckt();
}
/*--------------------------------------------------------------------------*/
CARD* DEV_SUBCKT_PROTO::clone_instance()const
{
  DEV_SUBCKT* new_instance = dynamic_cast<DEV_SUBCKT*>(p1.clone());
  assert(!new_instance->subckt());

  if (this == &pp){ untested();
    // cloning from static, empty model
    // look out for _parent in expand
  }else{
    new_instance->_parent = this;
  }

  assert(new_instance->is_device());
  return new_instance;
}
/*--------------------------------------------------------------------------*/
DEV_SUBCKT::DEV_SUBCKT()
  :BASE_SUBCKT(),
   _parent(NULL)
{
  attach_common(&Default_SUBCKT);
  _n = _nodes;
  ++_count;
}
/*--------------------------------------------------------------------------*/
DEV_SUBCKT::DEV_SUBCKT(const DEV_SUBCKT& p)
  :BASE_SUBCKT(p),
   _parent(p._parent)
{
  //strcpy(modelname, p.modelname); in common
  for (int ii = 0;  ii < max_nodes();  ++ii) {
    _nodes[ii] = p._nodes[ii];
  }
  _n = _nodes;
  assert(!subckt());
  ++_count;
}
/*--------------------------------------------------------------------------*/
std::string DEV_SUBCKT::port_name(int i)const
{
  if (const DEV_SUBCKT* p=dynamic_cast<const DEV_SUBCKT*>(_parent)) {
    if (i<p->net_nodes()){
      return p->port_value(i);
    }else{untested(); 
      return "";
    }
  }else if(_parent){untested(); untested();
    // reachable?
    return "";
  }else{untested();
    return "";
  }
}
/*--------------------------------------------------------------------------*/
void DEV_SUBCKT::expand()
{
  BASE_SUBCKT::expand();
  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);
  if (!_parent) { untested();
    // first time
    assert(c->modelname()!="");
    const CARD* model = find_looking_out(c->modelname());
    if ((_parent = dynamic_cast<const DEV_SUBCKT_PROTO*>(model))) { untested();
      // good
    }else if (dynamic_cast<const BASE_SUBCKT*>(model)) { untested();
      throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt proto");
    }else{ untested();
      throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt");
    }
  }else{
    // reruns
    // bypassed: does not play with hidemodule
    // assert(find_looking_out(c->modelname()) == _parent);
  }
  
  assert(_parent);
  assert(_parent->subckt());
  assert(_parent->subckt()->nodes());
  trace2("",  _parent->net_nodes(),  _parent->subckt()->nodes()->how_many());
  assert(_parent->net_nodes() <= _parent->subckt()->nodes()->how_many());
  assert(_parent->subckt()->params());

  PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
  assert(pl);
  c->_params.set_try_again(pl);

  renew_subckt(_parent, &(c->_params));
  subckt()->expand();
}
/*--------------------------------------------------------------------------*/
bool DEV_SUBCKT::is_valid() const
{
  assert(subckt());
  assert(_parent);
  assert(_parent->subckt());
  PARAM_LIST const* params = _parent->subckt()->params();
  PARAM_INSTANCE v = params->deep_lookup("_..is_valid");
  trace2("DEV_MODULE::is_valid I", long_label(), v.string());
  Base const* x = v.e_val(nullptr, subckt());
  Integer c;
  Integer* res = c.assign(x);
  if(!res) {
    return true;
  }else{
    assert(x);
    trace1("DEV_MODULE::is_valid I", typeid(*x).name());
    int a = res->value();
    delete res;
    return a;
  }
}
/*--------------------------------------------------------------------------*/
void DEV_SUBCKT::precalc_first()
{
  trace2("DEV_SUBCKT::precalc_first1", long_label(), owner());
  BASE_SUBCKT::precalc_first();
  trace2("DEV_SUBCKT::precalc_first2", long_label(), owner());

  if (subckt()) {
  }else{
    new_subckt();
  }

  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);

  if(_parent){
    PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
    assert(pl);
    c->_params.set_try_again(pl);
  }else{ untested();
  }

  for( auto p: c->_params){
    trace2("DEV_SUBCKT::precalc_first att", p.first, p.second.string());
  }

  subckt()->attach_params(&(c->_params), scope());
  trace1("DEV_SUBCKT::precalc_first recurse", long_label());
//  subckt()->precalc_first();
  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
void DEV_SUBCKT::precalc_last()
{
  BASE_SUBCKT::precalc_last();

  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);
  subckt()->attach_params(&(c->_params), scope());
  subckt()->precalc_last();

  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
double DEV_SUBCKT::tr_probe_num(const std::string& x)const
{untested();
  if (Umatch(x, "p ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"P");
    }      
    return power;
  }else if (Umatch(x, "pd ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"PD");
    }      
    return power;
  }else if (Umatch(x, "ps ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"PS");
    }      
    return power;
  }else{untested();
    return COMPONENT::tr_probe_num(x);
  }
  /*NOTREACHED*/
}
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
