/*                              -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2022, 2023 Felix Salfelder
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
 * module stuff
 */
#include <u_nodemap.h>
#include <e_node.h>
#include <globals.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#include <io_trace.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
// components with one node are unlikely.
const size_t node_capacity_floor = 2;
/*--------------------------------------------------------------------------*/
static void grow_nodes(size_t Index, node_t*& n, size_t& capacity, size_t capacity_floor)
{
  if(Index < capacity){
  }else{
    size_t new_capacity = std::max(capacity, capacity_floor);
    while(new_capacity <= Index) {
      assert(new_capacity < new_capacity * 2);
      new_capacity *= 2;
    }
    node_t* new_nodes = new node_t[new_capacity];
    for(size_t i=0; i<capacity; ++i){
      new_nodes[i] = n[i];
    }
    delete[] n;
    n = new_nodes;
    capacity = new_capacity;
  }
}
/*--------------------------------------------------------------------------*/
static COMMON_PARAMLIST Default_SUBCKT(CC_STATIC);
/*--------------------------------------------------------------------------*/
class DEV_MODULE : public BASE_SUBCKT {
private:
  const BASE_SUBCKT* _parent;
  size_t _node_capacity{0};
private:
  explicit	DEV_MODULE(const DEV_MODULE&);
public:
  explicit	DEV_MODULE();
		~DEV_MODULE()		{ delete[] _n; _node_capacity = 0; }
  CARD*		clone()const override;
private:
  void		set_port_by_index(int Index, std::string& Value) override;
  // void	set_port_by_name(std::string&, std::string&) override;
private: // override virtual
  bool		is_device()const override	{return _parent;}
  char		id_letter()const override	{return 'X';}
  bool		print_type_in_spice()const override {return true;}
  std::string   value_name()const override	{return "#";}
  int		max_nodes()const override;
  int		min_nodes()const override;
  int		matrix_nodes()const override	{return 0;}
  // int	net_nodes()const override	{return _net_nodes;}
  void		precalc_first()override;
  bool		makes_own_scope()const override  {return !_parent;}
  bool		is_valid()const override;
  CARD_LIST*	   scope()override;
  const CARD_LIST* scope()const	override {return const_cast<DEV_MODULE*>(this)->scope();}

  void		expand() override;

private: // no ops for prototype
  void map_nodes()override	{if(is_device()){ BASE_SUBCKT::map_nodes();}else{} }
  void tr_begin()override	{if(is_device()){ BASE_SUBCKT::tr_begin();}else{} }
  void ac_begin()override	{if(is_device()){ BASE_SUBCKT::ac_begin();}else{} }
  void tr_load()override	{if(is_device()){ BASE_SUBCKT::tr_load();}else{} }
  void tr_accept()override	{if(is_device()){ BASE_SUBCKT::tr_accept();}else{} }
  void tr_regress()override	{if(is_device()){ BASE_SUBCKT::tr_regress();}else{} }
  void tr_advance()override	{if(is_device()){ BASE_SUBCKT::tr_advance();}else{} }
  TIME_PAIR tr_review() override { if(is_device()){return BASE_SUBCKT::tr_review();}else{
                            return TIME_PAIR(NEVER, NEVER);}}
  void dc_advance()override	{if(is_device()){ BASE_SUBCKT::dc_advance();}else{} }
  void do_ac()override		{if(is_device()){ BASE_SUBCKT::do_ac();}else{} }
  void ac_load()override	{if(is_device()){ BASE_SUBCKT::ac_load();}else{} }
  void tr_queue_eval()override{
    if(is_device()){
      BASE_SUBCKT::tr_queue_eval();
    }else{
    }
  }
  bool do_tr() override		{if(is_device()){untested(); return BASE_SUBCKT::do_tr();}else{untested(); return true;} }

  bool tr_needs_eval()const override{
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
} p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "module", &p1);
/*--------------------------------------------------------------------------*/
void DEV_MODULE::set_port_by_index(int Index, std::string& Value)
{
  grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
  BASE_SUBCKT::set_port_by_index(Index, Value);
}
/*--------------------------------------------------------------------------*/
int DEV_MODULE::max_nodes() const
{
  if(_parent){
    return ((CARD const*)_parent)->net_nodes();
  }else{
    // allow one more, building a prototype.
    return net_nodes()+1;
  }
}
/*--------------------------------------------------------------------------*/
int DEV_MODULE::min_nodes() const
{
  if(_parent){
    return ((CARD const*)_parent)->net_nodes();
  }else{
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
CARD_LIST* DEV_MODULE::scope()
{
  if(is_device()){
    return COMPONENT::scope();
  }else{
    return subckt();
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_MODULE::is_valid() const
{
  trace1("DEV_MODULE::is_valid", long_label());
  assert(subckt());
  assert(_parent);
  assert(_parent->subckt());
  PARAM_LIST const* params = _parent->subckt()->params();
  PARAMETER<double> v = params->deep_lookup("_..is_valid");
  trace1("DEV_MODULE::is_valid I", v.string());
  double x = v.e_val(1., subckt());
  return x==1.;
}
/*--------------------------------------------------------------------------*/
CARD* DEV_MODULE::clone()const
{
  DEV_MODULE* new_instance = new DEV_MODULE(*this);
  assert(!new_instance->subckt());

  if (this == &p1){
    // cloning from static, empty model
    // has no parent.
    new_instance->new_subckt(); // from DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO
  }else if(_parent){
    new_instance->_parent = _parent;
    assert(new_instance->is_device());
  }else{
    new_instance->_parent = this;
    assert(new_instance->is_device());
  }

  return new_instance;
}
/*--------------------------------------------------------------------------*/
DEV_MODULE::DEV_MODULE()
  :BASE_SUBCKT(),
   _parent(NULL), _node_capacity(0)
{
  attach_common(&Default_SUBCKT);
  assert(_n == NULL);
}
/*--------------------------------------------------------------------------*/
DEV_MODULE::DEV_MODULE(const DEV_MODULE& p)
  :BASE_SUBCKT(p),
   _parent(p._parent)
{
  trace2("DEV_MODULE::DEV_MODULE", long_label(), net_nodes());
  _node_capacity = net_nodes();
  if(_node_capacity){
    _n = new node_t[_node_capacity];
  }else{
    assert(_n == NULL);
  }
  if(p.is_device()){
    for (int ii = 0;  ii < net_nodes();  ++ii) {
      _n[ii] = p._n[ii];
    }
  }else{
    for (int ii = 0;  ii < net_nodes();  ++ii) {
      assert(!_n[ii].n_());
    }
  }
  assert(!subckt());
}
/*--------------------------------------------------------------------------*/
int DEV_MODULE::set_param_by_name(std::string Name, std::string Value)
{
  assert(_parent);
  assert(_parent->subckt());

  PARAM_LIST::const_iterator p = _parent->subckt()->params()->find(Name);
  if(p != _parent->subckt()->params()->end()){
    return BASE_SUBCKT::set_param_by_name(Name,Value);
  }else{
    throw Exception_No_Match(Name);
  }
}
/*--------------------------------------------------------------------------*/
std::string DEV_MODULE::port_name(int i)const
{
  if (const DEV_MODULE* p=dynamic_cast<const DEV_MODULE*>(_parent)) {
    if (i<p->net_nodes()){
      return p->port_value(i);
    }else{untested();
      return "";
    }
  }else if(_parent) { untested(); untested();
    // reachable?
    return "";
  }else if(i<net_nodes()) {
    return port_value(i);
  }else{
    return "";
  }
}
/*--------------------------------------------------------------------------*/
void DEV_MODULE::expand()
{
  if(!_parent){
    // not a device. probably a prototype
  }else{
    BASE_SUBCKT::expand();
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);
    BASE_SUBCKT::expand();
    assert(_parent);
    assert(_parent->subckt());
    assert(_parent->subckt()->params());
    PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
    assert(pl);
    c->_params.set_try_again(pl);

    renew_subckt(_parent, &(c->_params));
    subckt()->expand();

#if 1 // move to CARD_LIST::expand?
    for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ++i){
      CARD* d = (*i)->deflate();

      if(d == (*i)){
      }else{
	assert(d->owner() == this);
	delete *i;
	*i = d;
      }
    }
#endif
  }
}
/*--------------------------------------------------------------------------*/
void DEV_MODULE::precalc_first()
{
  BASE_SUBCKT::precalc_first();

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

    subckt()->attach_params(&(c->_params), scope());
    assert(!is_constant()); /* because I have more work to do */
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_MODULE::precalc_last()
{
  if(is_device()){
    BASE_SUBCKT::precalc_last();
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);
    subckt()->attach_params(&(c->_params), scope());
    subckt()->precalc_last();

    assert(!is_constant()); /* because I have more work to do */
  }else{
    // its a proto.
  }
}
/*--------------------------------------------------------------------------*/
double DEV_MODULE::tr_probe_num(const std::string& x)const
{
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
  }else{
    return COMPONENT::tr_probe_num(x);
  }
  /*NOTREACHED*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
