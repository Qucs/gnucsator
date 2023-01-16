/*                     -*- C++ -*-
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
 */
#include "u_nodemap.h"
#include "e_node.h"
#include "globals.h"
#include "e_paramlist.h"
#include "e_subckt.h"
#include "e_model.h"
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static COMMON_PARAMLIST Default_SUBCKT(CC_STATIC);
/*--------------------------------------------------------------------------*/
// looks like INSTANCE from d_subckt.cc, but isnt.
// this one is never part of a simulation, because of deflation.
// TODO: cleanup/rename?
class DEV_INSTANCE_PROTO;
class INSTANCE : public BASE_SUBCKT {
  friend class DEV_INSTANCE_PROTO; // has to do with _parent.
private:
protected: // HACK
  std::vector<node_t> _nodes;
  static int	_count;

protected: // stub stuff
  INSTANCE const* _cloned_from; // use common/mutable_common instead?
  const COMPONENT* _parent;
  mutable DEV_INSTANCE_PROTO* _proto; // use common->proto?
  std::vector<std::pair<std::string, std::string>> _params;
  std::vector<std::string> _port_names;
protected:
  explicit	INSTANCE(const INSTANCE&);
public:
  explicit	INSTANCE();
		~INSTANCE()		{
		  --_count;
		  delete (CARD*)_proto;
		}
  CARD*		clone()const		{
    INSTANCE* new_instance = new INSTANCE(*this);

    // BUG?
    new_instance->_parent = (COMPONENT const*)_proto;

    return new_instance;
  }
private: // override virtual
  char		id_letter()const	{return 'X';}
  bool		print_type_in_spice()const {return true;}
  std::string   value_name()const	{return "#";}
protected:
  int		max_nodes()const	{
    // INT_MAX results in arithmetic overflow in lang_spice
    return INT_MAX-2;
  }
public: // ?
  void build_proto() const;
private:

protected:
  void set_port_by_index(int Index, std::string& Value);

  // override. the base class does not know about _parent.
  void set_port_by_name(std::string& name, std::string& ext_name);
  int		min_nodes()const	{return 0;}
  int		ext_nodes()const	{return net_nodes();}
  int		matrix_nodes()const	{return 0;}
protected:
  int		net_nodes()const	{return _net_nodes;}
  void		precalc_first();
private:
  bool		makes_own_scope()const  {return false;}

protected:
  void		expand()override;
  CARD*		deflate()override;
private:
  void		precalc_last()override{
    trace1("INSTANCE::precalc_last", long_label());
    unreachable();
  }
  double	tr_probe_num(const std::string&)const {unreachable(); return 0.;}
  int param_count_dont_print()const {return 0;}
  int param_count() const override {
    return int(_params.size());
  }
private: // overrides
  bool param_is_printable(int)const override {
    return true;
  }
  void set_param_by_name(std::string name, std::string value) override {
    _params.push_back(std::make_pair(name, value));
    // mutable_common()->set_param_by_name(name, value); // ?
  }
  std::string param_name(int i, int) const override { untested();
    return param_name(i, 0);
  }
  std::string param_name(int i) const override {
    assert(i<int(_params.size()));
    return _params[i].first;
  }
  std::string param_value(int i) const override {
    assert(i<int(_params.size()));
    return _params[i].second;
  }
  void set_param_by_index(int i, std::string& value, int) override { untested();
    int idx = i+1;

    // TODO: use common.
    if(int(_params.size()) == idx){ untested();
      _params.push_back(std::make_pair("", value));
    }else{ untested();
      throw Exception(long_label() + ": param assign out of order");
    }
  }

private:
  void collect_overloads(DEV_INSTANCE_PROTO* scope) const;
  void prepare_overload(CARD* proto, std::string modelname, DEV_INSTANCE_PROTO* p) const;

protected:
  std::string port_name(int i)const;
public:
  static int	count()			{untested();return _count;}
protected:
} p1;
int INSTANCE::_count = -1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "instance", &p1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_INSTANCE_PROTO : public INSTANCE {
  explicit	DEV_INSTANCE_PROTO(const DEV_INSTANCE_PROTO&p)
   : INSTANCE(p) { untested();
     new_subckt();
   }
public:
  explicit	DEV_INSTANCE_PROTO() : INSTANCE() {
    new_subckt();
  }
		~DEV_INSTANCE_PROTO(){}
private:
  CARD* clone() const override;
  CARD* clone_instance()const override{ untested(); return clone();}
public:
  void precalc_first() override { unreachable(); }
  void precalc_last() override { unreachable(); }
  CARD_LIST*	   scope()		{ return subckt(); }
  const CARD_LIST* scope()const		{ return subckt(); }

  bool do_tr() override { unreachable(); return true; }

public:
  void set_port_by_index(int Index, std::string& Value){
    assert(!_parent);
    // assert(!_proto);
    if(_proto){ untested();
    }else{
    }

    _nodes.resize(std::max(size_t(Index)+1, _nodes.size())); // TODO
    _n = _nodes.data();
    trace3("resized", long_label(), Index, _nodes.capacity());

    BASE_SUBCKT::set_port_by_index(Index, Value);
  }

  void set_port_by_name(std::string&, std::string&) override { untested();
    unreachable();
  }
  int		max_nodes()const	{ return int(_nodes.size());}

  void set_param_by_name(std::string name, std::string value) override { untested();
    trace3("proto:spbn", long_label(), name, value);
  }


}pp; // DEV_INSTANCE_PROTO
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "instance_proto", &pp);
/*--------------------------------------------------------------------------*/
void INSTANCE::prepare_overload(CARD* model, std::string modelname, DEV_INSTANCE_PROTO* Proto) const
{
  // assert(Proto==this); // for now.
  trace3("prepare_overload", Proto->long_label(), Proto->net_nodes(), _parent);
  assert(Proto);
  // assert(!_parent);
  assert(Proto->subckt());
  assert(Proto->scope()==Proto->subckt());
  assert(model);
  CARD* cl = model->clone_instance();
  COMPONENT* c = prechecked_cast<COMPONENT*>(cl);
  assert(c || !cl);

  // dup in CMD_PARAMSET creation, lang_verilog.cc
  if(!cl){ untested();
    return;
  }else if(!c->common()){
    c->set_dev_type(modelname);
  }else if(auto m=dynamic_cast<MODEL_CARD const*>(model)){ untested();
    // bypass spice-style find_model
    trace3("prepare_overload bypass", Proto->long_label(), Proto->net_nodes(), _parent);
    assert(c->common());
    COMMON_COMPONENT* cc = c->common()->clone();
    cc->attach(m);
    cc->set_modelname("");
    c->attach_common(cc);
  }else{
    c->set_dev_type(modelname);
  }

  std::string label = short_label() + ":p" + std::to_string(Proto->subckt()->size());

  c->set_owner(Proto);
  c->set_label(label);

  try {
    trace3("DEV_INSTANCE_PROTO::po, set port in proto", Proto->long_label(), Proto->net_nodes(), Proto->max_nodes());
    for(int i=0; i<Proto->net_nodes(); ++i){
      std::string v = Proto->port_value(i);
      trace3("DEV_INSTANCE_PROTO::po, set port in proto", Proto->long_label(), i, v);

      if(v[0] == '*'){
	c->set_port_by_index(i, v);
      }else{ untested();
	c->set_port_by_name(v, v);
      }
    }
    if(Proto->net_nodes() < c->min_nodes()){ untested();
      throw Exception("not enough nodes\n");
    }else{
    }

    COMMON_PARAMLIST const* cp = prechecked_cast<COMMON_PARAMLIST const*>(Proto->common());
    assert(cp);
    for(int i=0; i<int(_params.size()); ++i){
      trace4("stub param fwd", c->long_label(), i, _params[i].first, _params[i].second);
      std::string value = _params[i].second;
      if(_params[i].first == ""){ untested();
	int idx = c->param_count() - i - 1;
	c->set_param_by_index(idx, value, 0);
      }else{
	c->set_param_by_name(_params[i].first, value);
      }
    }
    Proto->subckt()->push_front(c);
  }catch(Exception const& e){ untested();
    // TODO: include proto name attribute
    error(bLOG, long_label() + " discarded: " + e.message() + "\n");
    delete (CARD*) c;
  }

  trace3("done overload", Proto->long_label(), modelname, Proto->subckt()->size());
} // prepare_overload
/*--------------------------------------------------------------------------*/
void INSTANCE::collect_overloads(DEV_INSTANCE_PROTO* Proto) const
{
  if(Proto->subckt()->size()){
    incomplete();
    return;
  }else{
  }

  COMMON_PARAMLIST const* c = prechecked_cast<COMMON_PARAMLIST const*>(Proto->common());
  assert(c);
  assert(c->modelname()!="");
  std::string modelname = c->modelname();
  trace3("co", long_label(), c->modelname(), Proto->long_label());

  assert(!_parent);
  assert(Proto->scope()==Proto->subckt());
  assert(!Proto->scope()->size());

  for(auto n : *(Proto->subckt()->nodes())){
    trace1("node", n.first);
  }

  if (modelname == "") { untested();
    throw Exception(Proto->long_label() + ": missing args -- need model name");
  }else if(Proto->subckt()->size()){ untested();
    // how to make reruns safe?
  }else{
    assert(Proto->subckt());
    CARD_LIST const& toplevel = CARD_LIST::card_list;

    CARD_LIST::const_iterator i = toplevel.find_(modelname);
    while(i != toplevel.end()) {
      error(bLOG, long_label() + ": " + modelname + " from top level\n");

      prepare_overload(*i, modelname, Proto);
      i = toplevel.find_again(modelname, ++i);
    }

    MODEL_CARD* m = model_dispatcher[modelname];
    std::string extended_name = modelname;
    int bin_count = 0;
    while(m){ untested();
      error(bLOG, long_label() + ": " + extended_name + " from model_dispatcher\n");
      prepare_overload(m, modelname, Proto);
      extended_name = modelname + ':' + to_string(bin_count++);
      m = model_dispatcher[extended_name];
    }

    CARD* p = device_dispatcher[modelname];
    extended_name = modelname;
    bin_count = 0;
    while(p){
      error(bLOG, long_label() + ": " + extended_name + " from device_dispatcher\n");
      prepare_overload(p, modelname, Proto);
      extended_name = modelname + ':' + to_string(bin_count++);
      p = device_dispatcher[extended_name];
    }

  }

  if(size_t s = Proto->subckt()->size()){
    error(bTRACE, long_label() + ": " + std::to_string(s) + " candidate" + (s>1?"s":"") +
	" found for " +modelname+ "\n");
  }else{ untested();
    error(bDANGER, long_label() + ": no candidates found for " +modelname+ "\n");
    // not in precalc
    // throw Exception(long_label() + ": no candiates found for " + modelname);
  }
} // collect_overloads
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
CARD* INSTANCE::deflate()
{
  trace3("INSTANCE::deflate", long_label(), subckt()->size(), dev_type());
//  return this; // keep it all. for debugging
  CARD_LIST* s = subckt();
  assert(s);
  assert(_parent);
  assert(scope()!=s);
  assert(s->size() && "no candidates in deflate?!");

  if(s->size()){
    auto i = s->begin();
    ++i;
    while(i!=s->end()){ untested();
      auto j = i;
      ++i;
      error(bWARNING, long_label() + " too many candidates.\n");
      s->erase(j);
    }
  }else{ untested();
    // this isnt the correct exception.
    throw Exception_Type_Mismatch("ERROR", long_label(), dev_type());
  }
  assert(s->size()==1); // a random one, for now.

  CARD_LIST::iterator i = s->begin();
  if(i!=s->end()){
    CARD* r = *i;
    { // TODO?: r = s->detach(i);
      *i = NULL;
      s->erase(i);
      r->set_owner(NULL);
      // r->precalc_first(); BUG?
      r->set_owner(owner());
      r->set_label(short_label());
      r->set_dev_type(dev_type()); // really?
    }

    // rewire nodes.
    COMPONENT* c = prechecked_cast<COMPONENT*>(r);
    assert(c);
    int h = _parent->subckt()->nodes()->how_many();
    trace2("rewire", long_label(), h);
    for(int i=0; i<net_nodes(); ++i){
      std::string nn = _n[i].n_()->short_label();
      trace4("rewire", long_label(), i, nn, c->n_(i).n_()->short_label());
      trace4("rewire", long_label(), i, c->n_(i).t_(), n_(i).t_());
      trace4("rewire", long_label(), i, c->n_(i).e_(), n_(i).e_());
    }
    for(int i=0; i<net_nodes(); ++i){
      assert(c->n_(i).t_() == n_(c->n_(i).e_()-1).t_());
      c->n_(i) = n_(c->n_(i).e_()-1);
    }

    assert(r->dev_type()!="");
    CARD* deflated = r->deflate();
    if(deflated == r){
      assert(deflated->owner() == owner());
    }else{ untested();
      assert(deflated->owner() == owner());
      // a paramset?
      delete (CARD*) r;
    }
    trace2("INSTANCE::deflate done", long_label(), subckt()->size());
    trace2("INSTANCE::deflate done", deflated->dev_type(), dev_type());
    assert(deflated->dev_type()==dev_type());
    return deflated;
  }else{ untested();
    throw Exception_No_Match(dev_type());
  }
} // INSTANCE::deflate
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
INSTANCE::INSTANCE()
  :BASE_SUBCKT()
  ,_cloned_from(NULL)
  ,_parent(NULL)
  ,_proto(NULL)
{
  attach_common(&Default_SUBCKT);
  _n = _nodes.data();
  ++_count;
}
/*--------------------------------------------------------------------------*/
INSTANCE::INSTANCE(const INSTANCE& p)
  :BASE_SUBCKT(p)
  ,_cloned_from(&p)
  ,_parent(p._parent)
  ,_proto(NULL)
{
  trace2("INSTANCE::INSTANCE", p.long_label(), p._net_nodes);
  _nodes = p._nodes;
  _net_nodes = p._net_nodes;
  _n = _nodes.data();
  assert(!subckt());
  ++_count;

  set_label(p.short_label());
  set_dev_type(p.dev_type());

  if(_parent){ untested();
  }else if(!common()){ untested();
    unreachable();
  }else if(common()->has_model()){ untested();
    trace2("INSTANCE::INSTANCE has model", p.long_label(), p._net_nodes);
    incomplete();
  }else{
    // TODO:: use dispatcher["instance_proto"]?
    trace2("INSTANCE::INSTANCE no model", p.long_label(), p._net_nodes);
    _proto = new DEV_INSTANCE_PROTO();
    assert(_proto->common());
    assert(_proto->subckt());
    assert(_proto->subckt()->params());
  }
}
/*--------------------------------------------------------------------------*/
std::string INSTANCE::port_name(int i)const
{
  if(size_t(i)<_port_names.size()){ untested();
    return _port_names[i];
  }else{
    return ""; // it has no name.
  }
}
/*--------------------------------------------------------------------------*/
void INSTANCE::expand()
{
  BASE_SUBCKT::expand();
  auto c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);

  assert(_parent);
  assert(_parent->subckt());
  assert(_parent->subckt()->nodes());
  trace3("INSTANCE::expand", long_label(), _parent->net_nodes(),  _parent->subckt()->nodes()->how_many());
  assert(_parent->net_nodes() <= _parent->subckt()->nodes()->how_many());
  assert(_parent->subckt()->params());

  {
    // some of these are used in debug mode in map_subckt_nodes
    // make valgrind happy.. (is a bug, fixed in sckt_ports branch)
    _nodes.resize(prechecked_cast<CARD const*>(_parent)->net_nodes());
    _n = _nodes.data();
  }

  trace4("expand I", long_label(), _parent->long_label(), _parent, _parent->net_nodes());
  trace2("expand I: renew", _parent->scope()->nodes(), _parent->scope()->nodes()->how_many());
  trace2("expand I: renew", _parent->scope()->size(), common()->has_model());
  trace2("expand I: renew", _parent->subckt()->size(), common()->has_model());
  if(!_parent->scope()->size()){ untested();
    std::string modelname = c->modelname();
    throw Exception(long_label() + ": no valid prototype found for " + modelname);
  }else {
    assert(_sim->is_first_expand());
    PARAM_LIST const* pl = _parent->subckt()->params();
    assert(pl);
    // c->_params.set_try_again(pl);

    renew_subckt(_parent, &(c->_params)); // pass owner?
    assert(scope()!=subckt());
    // subckt()->attach_params(&(c->_params), scope());
  }

  trace3("INSTANCE::expand sckt in", long_label(), subckt()->size(), _sim->is_first_expand());
  // assert(subckt()->size());
  subckt()->set_owner(NULL);
  subckt()->set_owner(owner()); // TODO: renew_subckt with alternative owner?
  subckt()->precalc_first(); // here?

  // sift. move to CARD_LIST::expand?
  for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ){
    CARD const* s = *i;
    COMPONENT const* d = dynamic_cast<COMPONENT const*>(s);
    CARD_LIST::iterator j = i;
      ++i;
    if(!d->is_valid()){
      error(bTRACE, long_label() + " dropped invalid candidate.\n");
      subckt()->erase(j);
    }else{
      // error(bTRACE, long_label() + " found valid candidate.\n");
    }
  }

  if(subckt()->size()==0){ untested();
    // reachable?
    throw Exception(long_label() + ": no candidates " + dev_type());
  }else if(subckt()->size()==1){
    (*subckt()->begin())->set_label(short_label());
  }else{ untested();
    // TODO: include name attributes, once available
    throw Exception(long_label() + ": ambiguous overload: " + dev_type());
  }

  trace2("INSTANCE::expand, pre expand sckt", long_label(), dev_type());
  subckt()->expand(); // here? duplicate precalc_first...

  for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ++i){
    CARD* s = *i;
    assert(s->owner()==owner());
    CARD* d = s->deflate();

    if(d == s){
    }else{ untested();
      assert(d->owner() == owner());
      *i = d;
      delete s;
    //  d->precalc_last(); // needed?
    }
  }
}
/*--------------------------------------------------------------------------*/
// Kludge: build proto in stub, so it only needs to be done once.
void INSTANCE::precalc_first()
{
  assert(common());
  trace3("INSTANCE::precalc_first", long_label(), _parent, common()->modelname());

  if(!owner()){
    build_proto();
    _parent=_proto;
  }else if(_cloned_from){
    _cloned_from->build_proto();
  }else{ untested();
  }

  if(_parent){
    trace2("INSTANCE::precalc_first w/ parent", long_label(), _parent->long_label());
  }else{ untested();
  }
  // a device in a module instance
  BASE_SUBCKT::precalc_first();

  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
void INSTANCE::build_proto() const
{
  if(!_proto){ untested();
    // static instance?
  }else{
    assert(scope());
    assert(scope()->params());
    assert(_proto->subckt());
    assert(_proto->subckt()->params());

    _proto->attach_common(common()->clone());
    // _proto->set_label(short_label());

    collect_overloads(_proto);
  }
}
/*--------------------------------------------------------------------------*/
CARD* DEV_INSTANCE_PROTO::clone()const
{ untested();
  INSTANCE* new_instance = dynamic_cast<INSTANCE*>(p1.clone());
  assert(!new_instance->subckt());
  assert(common());

  if (this == &pp){ untested();
    // cloning from static, empty model
    // look out for _parent in expand
  }else if(common()->has_model()){ untested();
    incomplete(); // ?
  }else{ untested();
    new_instance->_parent = this;
  }

  assert(new_instance->is_device());
  return new_instance;
}
/*--------------------------------------------------------------------------*/
void INSTANCE::set_port_by_index(int Index, std::string& Value)
{
  _nodes.resize(std::max(size_t(Index)+1, _nodes.size())); // TODO
  _n = _nodes.data();
  trace3("resized", long_label(), Index, _nodes.capacity());

  BASE_SUBCKT::set_port_by_index(Index, Value);

  if(!_parent){
    assert(_proto);

    std::string n = "*unnamed_port_" + std::to_string(Index);
    trace4("proto fwd", long_label(), Index, Value, n);
    _proto->set_port_by_index(Index, n);
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void INSTANCE::set_port_by_name(std::string& name, std::string& ext_name)
{ untested();
  trace3("INSTANCE::pbn", long_label(), name, ext_name);

  int i = net_nodes();
  _port_names.resize(net_nodes()+1);
  _port_names[net_nodes()] = name;

  if(subckt()){ untested();
  }else{ untested();
  }
  assert(scope());

  { // INSTANCE::set_port_by_index(i, ext_name);
    _nodes.resize(std::max(size_t(i)+1, _nodes.size())); // TODO
    _n = _nodes.data();
    trace3("resized", long_label(), i, _nodes.capacity());
    BASE_SUBCKT::set_port_by_index(i, ext_name);
  }

  assert(_proto);
  _proto->set_port_by_index(i, name);

  assert(scope()!=subckt());
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
