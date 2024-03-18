/*$Id: d_subckt.cc  2016/09/17  $ -*- C++ -*-
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
#include "e_node.h"
#include "globals.h"
#include "e_paramlist.h"
#include "e_subckt.h"
#include "io_trace.h"
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
  CARD*		clone()const override;
//  CARD*		clone_instance()const override;
private: // override virtual
  bool		is_device()const override	{return _parent;}
  char		id_letter()const override	{return 'X';}
  bool		print_type_in_spice()const override {return true;}
  std::string   value_name()const override	{return "#";}
  int		max_nodes()const override	{return PORTS_PER_SUBCKT;}
  int		min_nodes()const override	{return 0;}
  int		matrix_nodes()const override	{return 0;}
  int		net_nodes()const override	{return _net_nodes;}
  void		precalc_first()override;
  bool		makes_own_scope()const override  {return !_parent;}
  bool		is_valid() const override;
  CARD_LIST*	   scope() override;
  const CARD_LIST* scope()const override	{return const_cast<DEV_SUBCKT*>(this)->scope();}

  void		expand() override;

private: // no ops for prototype
  void map_nodes()override	{if(is_device()){ BASE_SUBCKT::map_nodes();}else{} }
  void tr_begin()override	{if(is_device()){ BASE_SUBCKT::tr_begin();}else{} }
  void ac_begin()override	{if(is_device()){ BASE_SUBCKT::ac_begin();}else{} }
  void tr_load()override	{if(is_device()){ BASE_SUBCKT::tr_load();}else{} }
  void tr_accept()override	{if(is_device()){ BASE_SUBCKT::tr_accept();}else{} }
  void tr_advance()override	{if(is_device()){ BASE_SUBCKT::tr_advance();}else{} }
  void dc_advance()override	{if(is_device()){ BASE_SUBCKT::dc_advance();}else{} }
  void do_ac()override		{if(is_device()){ BASE_SUBCKT::do_ac();}else{} }
  void ac_load()override	{if(is_device()){ BASE_SUBCKT::ac_load();}else{} }
  void tr_queue_eval()override{ untested();
    if(is_device()){ untested();
      BASE_SUBCKT::tr_queue_eval();
    }else{ untested();
    }
  }
  bool do_tr() override		{if(is_device()){untested(); return BASE_SUBCKT::do_tr();}else{untested(); return true;} }

  bool tr_needs_eval()const override{ untested();
    if(is_device()){untested();
      return BASE_SUBCKT::tr_needs_eval();
    }else{untested();
      return false;
    }
  }
private:
  void		precalc_last()override;
  double	tr_probe_num(const std::string&)const override;
  int param_count_dont_print()const override{return common()->COMMON_COMPONENT::param_count();}

  std::string port_name(int i)const override;
  int set_param_by_name(std::string Name, std::string Value)override;
public:
  static int	count()			{untested();return _count;}
protected:
  const BASE_SUBCKT* _parent;
private:
  node_t	_nodes[PORTS_PER_SUBCKT];
  static int	_count;
} p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "module", &p1);
int DEV_SUBCKT::_count = -1;
/*--------------------------------------------------------------------------*/
CARD_LIST* DEV_SUBCKT::scope()
{ untested();
  if(_parent){ untested();
    return COMPONENT::scope();
  }else{ untested();
    return subckt();
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_SUBCKT::is_valid() const
{ untested();
  trace1("DEV_SUBCKT::is_valid", long_label());
  assert(subckt());
  assert(_parent);
  assert(_parent->subckt());
  PARAM_LIST const* params = _parent->subckt()->params();
  PARAMETER<double> v = params->deep_lookup("_..is_valid");
  trace1("DEV_SUBCKT::is_valid I", v.string());
  double x = v.e_val(1., subckt());
  return x==1.;
}
/*--------------------------------------------------------------------------*/
CARD* DEV_SUBCKT::clone()const
{ untested();
  DEV_SUBCKT* new_instance = new DEV_SUBCKT(*this);
  assert(!new_instance->subckt());

  if (this == &p1){ untested();
    // cloning from static, empty model
    // look out for _parent in expand
    new_instance->new_subckt(); // from DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO
  }else if(_parent){ untested();
    new_instance->_parent = _parent;
    assert(new_instance->is_device());
  }else{ untested();
    new_instance->_parent = this;
    assert(new_instance->is_device());
  }

  return new_instance;
}
/*--------------------------------------------------------------------------*/
#if 0
CARD* DEV_SUBCKT::clone_instance()const
{ untested();
  DEV_SUBCKT* new_instance = dynamic_cast<DEV_SUBCKT*>(p1.clone());
  //assert(!new_instance->subckt());

  if (this == &p1){ untested();
    // cloning from static, empty model
    // look out for _parent in expand
    new_instance->new_subckt(); // from DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO
  }else if(_parent){ untested();
    new_instance->_parent = _parent;
    assert(new_instance->is_device());
  }else{ untested();
    new_instance->_parent = this;
  }

  assert(new_instance->is_device());
  return new_instance;
}
#endif
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
{ untested();
  //strcpy(modelname, p.modelname); in common
  for (int ii = 0;  ii < max_nodes();  ++ii) { untested();
    _nodes[ii] = p._nodes[ii];
  }
  _n = _nodes;
  assert(!subckt());
  ++_count;
}
/*--------------------------------------------------------------------------*/
int DEV_SUBCKT::set_param_by_name(std::string Name, std::string Value)
{ untested();
  assert(_parent);
  assert(_parent->subckt());

  PARAM_LIST::const_iterator p = _parent->subckt()->params()->find(Name);
  if(p != _parent->subckt()->params()->end()){ untested();
    return BASE_SUBCKT::set_param_by_name(Name,Value);
  }else{ untested();
    throw Exception_No_Match(Name);
  }
}
/*--------------------------------------------------------------------------*/
std::string DEV_SUBCKT::port_name(int i)const
{ untested();
  if (const DEV_SUBCKT* p=dynamic_cast<const DEV_SUBCKT*>(_parent)) { untested();
    if (i<p->net_nodes()){ untested();
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
{ untested();
  if(!_parent){ untested();
    return;
  }else{ untested();
    trace5("DEV_SUBCKT::expand", long_label(), net_nodes(), subckt(), _parent, ((COMPONENT const*)_parent)->net_nodes());
  }
  BASE_SUBCKT::expand();
  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);
  if(c->modelname()==""){ untested();
    assert(!_parent);
    // proto?
    return;
  }else{ untested();
    assert(_parent);
  }
  trace5("DEV_SUBCKT::expand", long_label(), net_nodes(), subckt(), _parent, ((COMPONENT const*)_parent)->net_nodes());
  BASE_SUBCKT::expand();
  if (!_parent) { untested();
    // get here when instanciating X, then set modelname
    assert(c->modelname()!="");
    const CARD* model = find_looking_out(c->modelname());
    if(!dynamic_cast<const BASE_SUBCKT*>(model)) { untested();
      throw Exception_Type_Mismatch(long_label(), c->modelname(), "module");
    }else{ untested();
      _parent = prechecked_cast<const BASE_SUBCKT*>(model);
    }
  }else{ untested();
    // possible after clone_instance.
    //
    // does not work, because prototypes may have been hidden.
    // assert(find_looking_out(c->modelname()) == _parent);
  }
  
  assert(_parent->subckt());
  assert(_parent->subckt()->params());
  PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
  assert(pl);
  c->_params.set_try_again(pl);

  renew_subckt(_parent, &(c->_params));
  subckt()->expand();

  for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ++i){ untested();
    CARD* d = (*i)->deflate();

    if(d == (*i)){ untested();
    }else{ untested();
      assert(d->owner() == this);
      delete *i;
      *i = d;
    }
  }
}
/*--------------------------------------------------------------------------*/
void DEV_SUBCKT::precalc_first()
{ untested();
  trace3("DEV_SUBCKT::precalc_first1", long_label(), owner(), is_device());
  BASE_SUBCKT::precalc_first();
  trace2("DEV_SUBCKT::precalc_first2", long_label(), owner());

  if (subckt()) { untested();
  }else{ untested();
    new_subckt();
  }

  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);

  if(_parent){ untested();
    PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
    assert(pl);
    c->_params.set_try_again(pl);

    for( auto p: c->_params){ untested();
      trace2("DEV_SUBCKT::precalc_first att", p.first, p.second.string());
    }

    subckt()->attach_params(&(c->_params), scope());
    trace1("DEV_SUBCKT::precalc_first recurse", long_label());
//  subckt()->precalc_first();
    assert(!is_constant()); /* because I have more work to do */
  }else{ untested();
  }

  // HACK
  if(1){ untested();
  }else if(!is_device()){ untested();
    PARAM_LIST* pl = const_cast<PARAM_LIST*>(scope()->params());
    assert(subckt());
    subckt()->params()->set_try_again(pl);

    // Qucs hack: deal with nested subckt protos.
    for(auto i : *subckt()){ untested();
      assert(i);
      if(i->is_device()){ untested();
      }else if(dynamic_cast<DEV_SUBCKT*>(i)){ untested();
	i->precalc_first();
      }else{ untested();
      }
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_SUBCKT::precalc_last()
{ untested();
  if(is_device()){ untested();
    BASE_SUBCKT::precalc_last();
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);
    subckt()->attach_params(&(c->_params), scope());
    subckt()->precalc_last();

    assert(!is_constant()); /* because I have more work to do */
  }else{ untested();
    // DEV_SUBCKT_PROTO
  }
}
/*--------------------------------------------------------------------------*/
double DEV_SUBCKT::tr_probe_num(const std::string& x)const
{ untested();
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
  }else{ untested();
    return COMPONENT::tr_probe_num(x);
  }
  /*NOTREACHED*/
}
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
