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
#include <globals.h>
#include <io_trace.h>
#include <u_nodemap.h>
#include <e_node.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#include <e_model.h>
#include <e_hsparam.h>
#include <c_comand.h>
#include <set>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
// components with one node are unlikely.
const size_t node_capacity_floor = 2;
/*--------------------------------------------------------------------------*/
static void grow_nodes(int Index, node_t*& n, int& capacity, int capacity_floor)
{
  if(Index < capacity){
  }else{
    int new_capacity = std::max(capacity, capacity_floor);
    while(new_capacity <= Index) {
      assert(new_capacity < new_capacity * 2);
      new_capacity *= 2;
    }
    node_t* new_nodes = new node_t[new_capacity];
    for(int i=0; i<capacity; ++i){
      new_nodes[i] = n[i];
    }
    delete[] n;
    n = new_nodes;
    capacity = new_capacity;
  }
}
/*--------------------------------------------------------------------------*/
class DEV_INSTANCE_PROTO;
class COMMON_INSTANCE : public COMMON_PARAMLIST {
public:
  COMMON_INSTANCE(int x) : COMMON_PARAMLIST(x) {}
public:
  DEV_INSTANCE_PROTO* _proto{nullptr};
};
static COMMON_INSTANCE Default_SUBCKT(CC_STATIC);
/*--------------------------------------------------------------------------*/
// looks like INSTANCE from d_subckt.cc, but isnt.
// this one is never part of a simulation, because of deflation.
// TODO: cleanup/rename?
class INSTANCE : public BASE_SUBCKT {
  friend class DEV_INSTANCE_PROTO; // has to do with _parent.
  node_t* _n{nullptr};
protected: // HACK
  static int	_count;

protected: // stub stuff
  INSTANCE const* _cloned_from; // use common/mutable_common instead?
  const COMPONENT* _parent{nullptr};
  mutable DEV_INSTANCE_PROTO* _proto; // use common->proto?
  std::vector<std::pair<std::string, std::string>> _params;
  std::vector<std::string> _port_names;
  int _node_capacity;
protected:
  explicit	INSTANCE(const INSTANCE&);
public:
  explicit	INSTANCE();
		~INSTANCE();
  CARD*		clone_instance()const override {
    // incomplete();
    return clone();
  }
  CARD*		clone()const override {
    INSTANCE* new_instance = new INSTANCE(*this);

    // BUG?
    new_instance->_parent = (COMPONENT const*)_proto;

    return new_instance;
  }
private: // override virtual
  char		id_letter()const override {return 'X';}
  bool		print_type_in_spice()const override{return true;}
  std::string   value_name()const override {return "#";}
protected:
  int		max_nodes()const override {
    // INT_MAX results in arithmetic overflow in lang_spice
    // (does not seem to work with lang_spice anyway)
    return INT_MAX-2;
  }
public: // ?
  void build_proto() const;
private:

protected:
  void set_port_by_index(int Index, std::string& Value)override;

  // override. the base class does not know about _parent.
  int set_port_by_name(std::string& name, std::string& ext_name)override;
  int		min_nodes()const override	{return 0;}
  int		ext_nodes()const override	{return net_nodes();}
  int		matrix_nodes()const override	{ untested();return 0;}
protected:
  int		net_nodes()const override	{return _net_nodes;}
  void		precalc_first()override;
private:
  bool		makes_own_scope()const override { untested();return false;}

protected:
  void		expand()override;
  CARD*		deflate()override;
private:
  void		precalc_last()override{ untested();
    trace1("INSTANCE::precalc_last", long_label());
    // unreachable();
  }
  double	tr_probe_num(const std::string&)const override { untested();unreachable(); return 0.;}
  int param_count_dont_print()const override {return 0;}
  int param_count() const override {
    return int(_params.size());
  }
private: // overrides
  void set_parameters(const std::string& Label, CARD* Parent,
			      COMMON_COMPONENT* Common, double Value,
			      int state_count, double state[],
			      int node_count, const node_t nodes[]) override{
    if(node_count){
      grow_nodes(node_count-1, _n, _node_capacity, node_capacity_floor);
      _net_nodes = short(node_count);
    }else{ untested();
    }
    COMPONENT::set_parameters(Label, Parent, Common, Value, state_count, state,
	node_count, nodes);
  }
  bool param_is_printable(int)const override {
    return true;
  }
  int set_param_by_name(std::string name, std::string value) override {
    trace3("instance spbn", long_label(), name, value);
    _params.push_back(std::make_pair(name, value));
    // mutable_common()->set_param_by_name(name, value); // ?
    return int(_params.size())-1; // incomplete.
  }
  std::string param_name(int i, int j) const override { untested();
    if(j==0){ untested();
      return param_name(i);
    }else{ untested();
      return "";
    }
  }
  std::string param_name(int i) const override {
    assert(i<int(_params.size()));
    return _params[i].first;
  }
  std::string param_value(int i) const override {
    assert(i<int(_params.size()));
    return _params[i].second;
  }
  void set_param_by_index(int I, std::string& value, int) override {
    assert(I>=0);
    trace2("spbi", I, value);

    // TODO: use common.
    if(int(_params.size()) == I){
      _params.push_back(std::make_pair("", value));
    }else{ untested();
      assert(0);
      throw Exception(long_label() + ": param assign out of order");
    }
  }

private:
  void collect_overloads(DEV_INSTANCE_PROTO* scope) const;
  void prepare_overload(CARD* proto, std::string modelname, DEV_INSTANCE_PROTO* p) const;

  node_t& n_(int i)const override {
    assert(_n); assert(i>=0); assert(i<_node_capacity); return _n[i];
  }
protected:
  std::string port_name(int i)const override;
public:
  static int	count()			{untested();return _count;}
protected:
} p1;
int INSTANCE::_count = -1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "instance|device_stub", &p1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_INSTANCE_PROTO : public INSTANCE {
  static std::set<DEV_INSTANCE_PROTO*> & protos(){
    static std::set<DEV_INSTANCE_PROTO*> p;
    return p;
  }
  explicit	DEV_INSTANCE_PROTO(const DEV_INSTANCE_PROTO&p)
   : INSTANCE(p) { untested();
     new_subckt();
     subckt()->set_verilog_math();
   }
public:
  explicit	DEV_INSTANCE_PROTO() : INSTANCE() {
    new_subckt();
    subckt()->set_verilog_math();
    protos().insert(this);
  }
  ~DEV_INSTANCE_PROTO(){
    protos().erase(this);
  }
private:
  CARD* clone() const override;
  CARD* clone_instance()const override{ untested(); return clone();}
public:
  void precalc_first() override { untested(); unreachable(); }
  void precalc_last() override { untested(); unreachable(); }
  CARD_LIST*	   scope()override { return subckt(); }
  const CARD_LIST* scope()const	override { return subckt(); }

  bool do_tr()override { untested(); unreachable(); return true; }

  void ac_final()override {untested();}
  void dc_final()override {untested();}
  void tr_final()override {untested();}

public:
  void set_port_by_index(int Index, std::string& Value)override {
    grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
    BASE_SUBCKT::set_port_by_index(Index, Value);
  }

  int set_port_by_name(std::string&name, std::string&ext_name)override {
    trace3("BUG?", long_label(), name, ext_name);
    // return INSTANCE::set_port_by_name(name, value);
    int i = net_nodes();
    _port_names.push_back(name);

    if(subckt()){
    }else{
    }
    assert(scope());


    if(_proto){
      { // INSTANCE::set_port_by_index(i, ext_name);
	grow_nodes(i, _n, _node_capacity, node_capacity_floor);
	BASE_SUBCKT::set_port_by_index(i, ext_name);
      }
      _proto->set_port_by_index(i, name);
      assert(scope()!=subckt());
    }else{
      trace4("INSTANCE::pbn proto", long_label(), net_nodes(), name, ext_name);
      set_port_by_index(i, ext_name);
    }

    return i; // TODO: test.
  }
//  int		max_nodes()const	{ untested(); return int(_nodes.size());}

  int set_param_by_name(std::string name, std::string value)override { untested();
    trace3("proto:spbn", long_label(), name, value);
    return 0.; // incomplete.
  }

  static void cleanup();

}pp; // DEV_INSTANCE_PROTO
DISPATCHER<CARD>::INSTALL dd(&device_dispatcher, "instance_proto", &pp);
/*--------------------------------------------------------------------------*/
class attributes : public CKT_BASE{
public:
  ATTRIB_LIST_p& set(tag_t t){
    return set_attributes(t);
  }
}attr;
/*--------------------------------------------------------------------------*/
void INSTANCE::prepare_overload(CARD* model, std::string modelname, DEV_INSTANCE_PROTO* Proto) const
{
  assert(Proto);
  assert(Proto->subckt());
  assert(Proto->scope()==Proto->subckt());
  assert(model);
  CARD* cl = model->clone_instance();
  COMPONENT* c = prechecked_cast<COMPONENT*>(cl);
  assert(c || !cl);

  if(cl && has_attributes(model->id_tag())) {
    trace2("INSTANCE::prepare_overload attr?", modelname, attributes(model->id_tag())->string(tag_t()));
    attr.set(cl->id_tag()) = attributes(model->id_tag());
  }else{
    trace1("INSTANCE::prepare_overload no attr", modelname);
  }

  if(!cl){
    return;
  }else if(!c->common()){
    c->set_dev_type(modelname);
  }else if(auto m=dynamic_cast<MODEL_CARD const*>(model)){
    // bypass spice-style find_model
    trace3("prepare_overload bypass", Proto->short_label(), Proto->net_nodes(), _parent);
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
  assert(c->is_device());

  try {
    trace3("DEV_INSTANCE_PROTO::po, set port in proto", Proto->short_label(), Proto->net_nodes(), Proto->max_nodes());
    trace2("DEV_INSTANCE_PROTO::po, set port in proto", net_nodes(), Proto->subckt()->nodes()->size());
    for(int i=0; i<Proto->net_nodes(); ++i){
      std::string nn = Proto->port_name(i);
      std::string v = Proto->port_value(i);

      if(nn[0] == '*'){
	trace4("DEV_INSTANCE_PROTO::po by idx", c->long_label(), i, v, nn);
	c->set_port_by_index(i, v);
      }else{
	trace2("DEV_INSTANCE_PROTO::po by name", v, v);
	c->set_port_by_name(v, v);
      }

      if(i<c->net_nodes()){
	// OK
      }else if(i<c->net_nodes()+c->num_current_ports()){
	trace2("DEV_INSTANCE_PROTO::po current port?", i, v);
	std::string branch_name = Proto->port_value(i); // v.substr(1);
	trace1("DEV_INSTANCE_PROTO::po current port?", branch_name);
	c->set_port_by_index(i, branch_name);
	// assert(0); // later
      }else{ untested();
	trace2("DEV_INSTANCE_PROTO::po sth wrong", i, v);
      }
    }
    if(Proto->net_nodes() < c->min_nodes()){ untested();
      throw Exception("not enough nodes, have "
	    + std::to_string(Proto->net_nodes()) + " need "
	    + std::to_string(c->min_nodes()) +"\n");
    }else{
    }

//    COMMON_PARAMLIST const* cp = prechecked_cast<COMMON_PARAMLIST const*>(Proto->common());
//    assert(cp);
    for(int i=0; i<int(_params.size()); ++i){
      trace4("stub param fwd1", c->short_label(), i, _params[i].first, _params[i].second);
      std::string name = _params[i].first;
      std::string value = _params[i].second;
      if(name == ""){
	c->set_param_by_index(i, value, 0);
      }else{
	try{
	  c->set_param_by_name(name, value);
	}catch(Exception_Clash const& e){
	  throw(Exception_Clash("parameter " + name + " already set"));
	}
      }
    }
    Proto->subckt()->push_back(c);
//    c->precalc_first(); // latch mfactor.??
  }catch(Exception const& e){
    trace1("discard", long_label());
    // TODO: include proto name attribute
    error(bLOG, long_label() + " discarded: " + e.message() + "\n");
    delete (CARD*) c;
  }
} // prepare_overload
/*--------------------------------------------------------------------------*/
void INSTANCE::collect_overloads(DEV_INSTANCE_PROTO* Proto) const
{
  if(Proto->subckt()->size()){
    // device has multiple instances
    // BUG: cleanup cache if is_first_expand()
    // incomplete();
    return;
  }else{
  }

  COMMON_PARAMLIST const* c = prechecked_cast<COMMON_PARAMLIST const*>(Proto->common());
  assert(c);
  assert(c->modelname()!="");
  std::string modelname = c->modelname();

  assert(Proto->scope()==Proto->subckt());
  assert(!Proto->scope()->size());

#ifdef DO_TRACE
  for(auto n : *(Proto->subckt()->nodes())){
    trace1("node", n.first);
  }
#endif

  if (_parent){
    // getting here in modelgen...?
  }else if (modelname == "") { untested();
    throw Exception(Proto->long_label() + ": missing args -- need model name");
  }else if(Proto->subckt()->size()){ untested();
    // how to make reruns safe?
  }else{
    assert(Proto->subckt());
    CARD_LIST const& toplevel = CARD_LIST::card_list;

    CARD_LIST::const_iterator i = toplevel.find_(modelname);
    while(i != toplevel.end()) {
      std::string desc;
      if(has_attributes((*i)->id_tag())) {
	auto const& a = attributes((*i)->id_tag());
	if(a){
	  desc = a->operator[](std::string("desc"));
	  if(desc == "0") { untested();
	    desc = "";
	  }else{
	    desc = ": " + desc;
	  }
	}else{ untested();
	}
      }else{
      }
      error(bLOG, long_label() + ": " + modelname + " from top level" + desc + "\n");

      prepare_overload(*i, modelname, Proto);
      i = toplevel.find_again(modelname, ++i);
    }

    MODEL_CARD* m = model_dispatcher[modelname];
    std::string extended_name = modelname;
    int bin_count = 0;
    while(m){
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
  }else{
    error(bDANGER, long_label() + ": no candidates found for " +modelname+ "\n");
    // not in precalc
    // throw Exception(long_label() + ": no candiates found for " + modelname);
  }
} // collect_overloads
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
CARD* INSTANCE::deflate()
{
//  return this; // keep it all. for debugging
  trace1("INSTANCE::deflate", long_label());
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
      *i = nullptr;
      s->erase(i);
      r->set_owner(nullptr);
      // r->precalc_first(); BUG?
      r->set_owner(owner());
      r->set_label(short_label());
      r->set_dev_type(dev_type()); // really?
    }

    // rewire nodes.
    COMPONENT* c = prechecked_cast<COMPONENT*>(r);
    assert(c);
    int h = _parent->subckt()->nodes()->how_many();
    for(int a = 0; a < _parent->net_nodes(); ++a) {
      trace3("rewire p", a, _parent->port_name(a), _parent->port_value(a));
    }
    trace3("rewire", long_label(), h, net_nodes());
#if 0
    for(int ii=0; ii<net_nodes(); ++ii){ untested();
      std::string nn = _n[ii].n_()->short_label();
      trace4("rewire", long_label(), ii, nn, c->n_(ii).n_()->short_label());
      trace4("rewire", long_label(), ii, c->n_(ii).e_(), n_(ii).e_());
    }
#endif
    for(int ii=0; ii<c->net_nodes(); ++ii){
      if(ii < c->net_nodes()) {
	trace3("rewire do", ii, c->n_(ii).e_(), _parent->port_name(ii));
	if( _parent->port_name(ii)[0] == '*'){
	  c->n_(ii) = n_(ii); // why?
	}else if(c->n_(ii).e_() != -1){
	  c->n_(ii) = n_(c->n_(ii).e_());
	}else{
	}
      }else if(ii < c->net_nodes()+c->num_current_ports()){
      }else{ untested();
      }
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
    // assert(deflated->dev_type()==dev_type()); ?
    return deflated;
  }else{ untested();
    throw Exception_No_Match(dev_type());
  }
} // INSTANCE::deflate
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
INSTANCE::INSTANCE()
  :BASE_SUBCKT()
  ,_cloned_from(nullptr)
  ,_parent(nullptr)
  ,_proto(nullptr)
  ,_node_capacity(0)
{
  attach_common(&Default_SUBCKT);
  assert(_n == nullptr);
  ++_count;
}
/*--------------------------------------------------------------------------*/
INSTANCE::INSTANCE(const INSTANCE& p)
  :BASE_SUBCKT(p)
  ,_cloned_from(&p)
  ,_parent(p._parent)
  ,_proto(nullptr)
  ,_port_names(p._port_names)
  ,_node_capacity(0)
{
  trace2("INSTANCE::INSTANCE", p.short_label(), p._net_nodes);
  assert(_net_nodes == p._net_nodes);
  _node_capacity = net_nodes();
  if(_node_capacity){
    _n = new node_t[_node_capacity];
  }else{
    assert(_n == nullptr);
  }
  if(p.is_device()){
    for (int ii = 0;  ii < net_nodes();  ++ii) {
      _n[ii] = p._n[ii];
    }
  }else{ untested();
    for (int ii = 0;  ii < net_nodes();  ++ii) { untested();
      assert(!_n[ii].n_());
    }
  }

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
    trace2("INSTANCE::INSTANCE no model", p.short_label(), p._net_nodes);
    _proto = new DEV_INSTANCE_PROTO();
   // _proto->set_owner(nullptr); // reset
   // assert(!_proto->owner());
    assert(_proto->common());
    assert(_proto->subckt());
    assert(_proto->subckt()->params());
  }
}
/*--------------------------------------------------------------------------*/
INSTANCE::~INSTANCE()
{
  --_count;
  delete _proto;
}
/*--------------------------------------------------------------------------*/
std::string INSTANCE::port_name(int i)const
{
  if(size_t(i)<_port_names.size()){
    return _port_names[i];
  }else{
    return ""; // it has no name.
  }
}
/*--------------------------------------------------------------------------*/
static int eff_param_count(CARD const* x)
{
  auto c = prechecked_cast<COMPONENT const*>(x);
  assert(c);
  HS_PARAM const* h = c->hsparam();
  return c->param_count() - (h?h->param_count():0);
}
/*--------------------------------------------------------------------------*/
static std::string param_count_string(CARD const* c)
{
  return to_string(eff_param_count(c));
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
  if(_parent->net_nodes() <= _parent->subckt()->nodes()->how_many()){
    // module
  }else{
    // modelgen
  }
  assert(_parent->subckt()->params());

#if 0
  { untested();
    // some of these are used in debug mode in map_subckt_nodes
    // make valgrind happy.. (is a bug, fixed in sckt_ports branch)
    _nodes.resize(prechecked_cast<CARD const*>(_parent)->net_nodes());
    _n = _nodes.data();
  }
#endif

  trace4("expand I", short_label(), _parent->short_label(), _parent, _parent->net_nodes());
  trace2("expand I: renew", _parent->scope()->nodes(), _parent->scope()->nodes()->how_many());
  trace2("expand I: renew", _parent->scope()->size(), common()->has_model());
  trace2("expand I: renew", _parent->subckt()->size(), common()->has_model());
  if(!_parent->scope()->size()){
    std::string modelname = c->modelname();
    throw Exception(long_label() + ": no valid prototype found for " + modelname);
  }else {
    assert(_sim->is_first_expand());
    // PARAM_LIST const* pl = _parent->subckt()->params();
    // assert(pl);
    // c->_params.set_try_again(pl);

    // here: candidate has too many ports.
    renew_subckt(_parent, &(c->_params)); // pass owner?
    assert(scope()!=subckt());
    // subckt()->attach_params(&(c->_params), scope());
  }

  trace3("INSTANCE::expand sckt in", long_label(), subckt()->size(), _sim->is_first_expand());
  // assert(subckt()->size());
  subckt()->set_owner(nullptr);
  subckt()->set_verilog_math();
  subckt()->set_owner(owner()); // TODO: renew_subckt with alternative owner?
  subckt()->precalc_first(); // here?

  // sift. move to CARD_LIST::expand?
  //
  // tie break rules
  // - The paramset with the fewest number of un-overridden parameters shall be selected.
  // - The paramset with the greatest number of local parameters with specified ranges shall be selected.
  // - The paramset with the fewest ports not connected in the instance line shall be selected.
  COMPONENT* gotit = nullptr;
  for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ){
    CARD const* s = *i;
    COMPONENT const* d = dynamic_cast<COMPONENT const*>(s);
    assert(d);
    CARD_LIST::iterator j = i;
    ++i;

    std::string desc;
    if(has_attributes(s->id_tag())) {
      auto const& a = attributes(s->id_tag());
      if(a){
	desc = a->operator[](std::string("desc"));
	if(desc == "0"){ untested();
	  desc = "";
	}else{
	  desc = ": " + desc;
	  error(bTRACE, long_label() + " .. candidate"+desc+", params: "+param_count_string(s)+"\n");
	}
      }else{ untested();
	// error(bTRACE, long_label() + " .. anonymous candidate.\n");
      }
    }else{
      // error(bTRACE, long_label() + s->dev_type() + " .. no attr candidate.\n");
      // error(bTRACE, long_label() + " .. params " + to_string(d->param_count()) + "\n");
    }

    if(!d->is_valid()){
      error(bTRACE, long_label() + " dropped invalid candidate"+desc+".\n");
    }else if(!gotit){
//      error(bTRACE, long_label() + " found valid candidate.\n");
      gotit = prechecked_cast<COMPONENT*>(*j);
      assert(gotit);
      *j = nullptr;
    }else if(eff_param_count(d) > eff_param_count(gotit)){
      if(desc.size()){ untested();
	error(bTRACE, long_label() + " rejecting candidate, more params"+desc+".\n");
      }else{
	error(bDEBUG, long_label() + " tie break: " + param_count_string(gotit) + " vs. " +
	    param_count_string(d) + "\n");
      }
    }else if(d->param_count() < gotit->param_count()){
      if(desc.size()){
	error(bTRACE, long_label() + " found fewer params"+desc+".\n");
      }else{
	error(bDEBUG, long_label() + " tie break: " + param_count_string(gotit) + " vs. " +
	    param_count_string(d) + "\n");
      }
      delete (CARD*) gotit;
      gotit = prechecked_cast<COMPONENT*>(*j);
      assert(gotit);
      *j = nullptr;
    }else if(d->max_nodes() > gotit->max_nodes()){ untested();
      error(bDEBUG, long_label() + " port tie break: " + to_string(gotit->max_nodes()) + " vs. " +
	  to_string(d->max_nodes()) + "\n");
    }else if(d->max_nodes() < gotit->max_nodes()){
      error(bDEBUG, long_label() + " port tie break: " + to_string(gotit->max_nodes()) + " vs. " +
	  to_string(d->max_nodes()) + "\n");
      delete (CARD*) gotit;
      gotit = prechecked_cast<COMPONENT*>(*j);
      assert(gotit);
      *j = nullptr;
    }else{
      error(bWARNING, long_label() + " ambiguous overload in " + dev_type() + "\n");
    }
    subckt()->erase(j);
  }
  if(gotit){
   // error(bDEBUG, long_label() + " got one: " + to_string(gotit->param_count()) + "\n");
    subckt()->push_back(gotit);
  }else{
  }

  if(subckt()->size()==0){
    // reachable?
    throw Exception(long_label() + ": no candidates " + dev_type());
  }else if(subckt()->size()==1){
    COMPONENT* d = dynamic_cast<COMPONENT*>(*subckt()->begin());
    assert(d);
    assert(d->is_valid());
    d->set_label(short_label());
    d->set_dev_type(dev_type()); // make spice happier..
  }else{ untested();
    // TODO: include name attributes, once available
    throw Exception(long_label() + ": ambiguous overload: " + dev_type());
  }

  trace2("INSTANCE::expand, pre expand sckt", long_label(), dev_type());

  COMPONENT* dev = dynamic_cast<COMPONENT*>(*subckt()->begin());
  if(0){
    // cannot expand_last
    subckt()->expand();
  }else{
    dev->precalc_first();
    dev->expand_first();
    dev->expand();
    if(0){
    }else{
      COMPONENT* ddd = dynamic_cast<COMPONENT*>(dev->deflate());
      if(ddd!=dev){
	*subckt()->begin() = ddd;
	delete (CARD*)dev;
	dev = ddd;
      }else{
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
// Kludge: build proto in stub, so it only needs doing once.
void INSTANCE::precalc_first()
{
  assert(common());
  trace3("INSTANCE::precalc_first", short_label(), _parent, common()->modelname());
  trace1("INSTANCE::precalc_first", _sim->is_first_expand());

  if(!owner()){
    build_proto();
    _parent = _proto; // common->proto?
  }else if(_cloned_from){
    _cloned_from->build_proto();
  }else{ untested();
  }

  if(_parent){
    trace2("INSTANCE::precalc_first w/ parent", short_label(), _parent->short_label());
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
  trace3("instance spbi", long_label(), Index, Value);
  grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
  BASE_SUBCKT::set_port_by_index(Index, Value);

  if(!_parent){
    assert(_proto);

    std::string n = "*unnamed_port_" + std::to_string(Index);
    trace3("proto fwd", long_label(), n, Value);
    _proto->set_port_by_name(n, Value);
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
int INSTANCE::set_port_by_name(std::string& name, std::string& ext_name)
{

  int i = net_nodes();
  _port_names.resize(net_nodes()+1);
  _port_names[net_nodes()] = name;

  if(subckt()){ untested();
  }else{
  }
  assert(scope());

  { // INSTANCE::set_port_by_index(i, ext_name);
    grow_nodes(i, _n, _node_capacity, node_capacity_floor);
    BASE_SUBCKT::set_port_by_index(i, ext_name);
  }

  if(_proto){
    _proto->set_port_by_index(i, name);
    assert(scope()!=subckt());
  }else{
    trace4("INSTANCE::pbn proto", long_label(), net_nodes(), name, ext_name);
    set_port_by_index(i, ext_name);
  }

  return i; // TODO: test.
}
/*--------------------------------------------------------------------------*/
void DEV_INSTANCE_PROTO::cleanup()
{
  for(auto i : protos() ){
    assert(i);
    assert(i->subckt());
    i->subckt()->erase_all();
  }
}
/*--------------------------------------------------------------------------*/
// TODO: need a better stash and mechanism
class CLEANUP : public CMD {
  void do_it(CS&, CARD_LIST* Scope)override {
    DEV_INSTANCE_PROTO::cleanup(); // TODO: more generic approach
    switch (ENV::run_mode) {
    case rPRE_MAIN:
      // unreachable(); // call from DETACH_HACK
      exit(0);
      break;
    case rINTERACTIVE:
	// fall through
    case rSCRIPT:
	// fall through
    case rBATCH:        command("clear", Scope); exit(0); break;
    case rPRESET:       untested(); /*nothing*/ break;
    }
  }
}p3;
DISPATCHER<CMD>::INSTALL d3(&command_dispatcher, "quit|exit", &p3);
/*--------------------------------------------------------------------------*/
// need this, because "quit" is not called from main.cc
// this effectively breaks "detach_all". don't use it for now.
class DETACH_HACK : public CMD {
  void do_it(CS&, CARD_LIST* Scope)override {
    // CMD::command("detach_all:0", Scope); segfault, currently executed code
    CMD::command("quit", Scope);
  }
}p3b;
DISPATCHER<CMD>::INSTALL d3_hack(&command_dispatcher, "detach_all", &p3b);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
