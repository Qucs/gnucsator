/*                       -*- C++ -*-
 * Copyright (C) 2022, 2023 Felix Salfelder
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
 */
#include <e_model.h>
#include <e_subckt.h>
#include <e_node.h>
#include <e_paramlist.h>
#include <globals.h>
#include <u_lang.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
// components with one node are unlikely.
const int node_capacity_floor = 2;
/*--------------------------------------------------------------------------*/
static COMMON_PARAMLIST Default_PARAMSET(CC_STATIC);
/*--------------------------------------------------------------------------*/
// from u_lang.cc, cut down a bit.
static CARD const* find_proto(const std::string& Name, const CARD* Scope)
{
  return OPT::language->find_proto(Name, Scope);

  const CARD* p = NULL;
  if (Scope) { untested();
    try { untested();
      p = Scope->find_looking_out(Name);
    }catch (Exception_Cant_Find& e) { untested();
      assert(!p);
    }
  }else{ untested();
    CARD_LIST::const_iterator i = CARD_LIST::card_list.find_(Name);
    if (i != CARD_LIST::card_list.end()) { untested();
      p = *i;
    }else{ untested();
      assert(!p);
    }
  }

  if (p) { untested();
    return p;
  }else if ((p = device_dispatcher[Name])) { untested();
    return p;
  }else if ((p = model_dispatcher[Name])) { untested();
    return p;
  }else{ untested();
    return NULL;
  }
} // find_proto
/*--------------------------------------------------------------------------*/
class PARAMSET : public BASE_SUBCKT {
  node_t* _n{nullptr};
private: // partly redudant
  PARAMSET const* _parent; // use _dev?
  COMPONENT const* _dev; // owned by paramset instance.
  int _node_capacity;
public:
  PARAMSET();
  PARAMSET(PARAMSET const& p);
  ~PARAMSET(){ delete[] _n; _node_capacity = 0; }
private:
  bool is_device() const override { return true; }

  std::string value_name()const override{ untested();unreachable(); return "";}
  std::string port_name(int)const override;
  node_t& n_(int i)const override {
    assert(_n); assert(i>=0); assert(i<_node_capacity); return _n[i];
  }
  bool print_type_in_spice()const override { untested();unreachable(); return false; }
#ifndef NDEBUG
  int set_port_by_name(std::string& name, std::string& value)override{
    assert(_dev);
    trace4("PARAMSET::spbn", long_label(), name, value, max_nodes());
    return BASE_SUBCKT::set_port_by_name(name, value);
  }
#endif
  void set_port_by_index(int Index, std::string& Value)override{
    grow_nodes(Index);
    BASE_SUBCKT::set_port_by_index(Index, Value);
  }

private:

  CARD* clone() const override;
  CARD* clone_instance() const override;

  bool is_valid() const override;
/*--------------------------------------------------------------------------*/

  int set_param_by_name(std::string Name, std::string Value) override;

//  CARD_LIST* scope() override{ untested();
//    return COMPONENT::scope();
//  }
//  CARD_LIST const* scope() const override { untested();
//    return const_cast<PARAMSET*>(this)->scope();
//  }

  int net_nodes()const override {
    if(_dev){
      // trace3("PARAMSET::net_nodes", _dev->long_label(), _dev->net_nodes(), _dev->max_nodes());
      return _net_nodes; // dev->net_nodes(); // problem with chain
      return _dev->net_nodes(); // problem with bjt
    }else{
      return BASE_SUBCKT::net_nodes();
    }
  }
  int min_nodes()const override{
    if(_dev){
      return _dev->min_nodes();
    }else{ untested();
      assert(_parent);
      return _parent->min_nodes();
    }
  }
  int max_nodes()const override{
    if(_dev){
      return _dev->max_nodes();
    }else{ untested();
      assert(_parent);
      return _parent->max_nodes();
    }
  }

private:
  void precalc_first() override;
  void build_sckt(COMPONENT*);
  void expand()override;
  CARD* deflate()override;
  void map_nodes()override { untested(); /*no-op*/ }
private: // no ops for top level
#if 1
  void precalc_last() override { untested(); assert(!is_device());}
  void tr_begin() override{ untested(); assert(!is_device());}
  void ac_begin() override{ untested(); assert(!is_device());}
  void tr_load() override{ untested(); assert(!is_device());}
  bool tr_needs_eval()const override{ untested(); assert(!is_device()); return false;}
  void tr_queue_eval()override{ untested(); assert(!is_device());}
  void tr_accept() override{ untested(); assert(!is_device());}
  double tr_probe_num(const std::string&)const override{untested(); return NOT_VALID;}
  void tr_advance() override{ untested(); assert(!is_device());}
  void dc_advance() override{ untested(); assert(!is_device());}
  bool do_tr() override{ untested(); assert(!is_device()); return true;}
  void do_ac() override{ untested(); assert(!is_device());}
  void ac_load() override{ untested(); assert(!is_device());}
#else
  void tr_begin() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_begin();
    }else{ untested();
    }
  }
  void tr_load() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_load();
    }else{ untested();
    }
  }
  void tr_advance() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_advance();
    }else{ untested();
    }
  }
  void tr_restore() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_restore();
    }else{ untested();
    }
  }
  void tr_regress() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_regress();
    }else{ untested();
    }
  }
  void dc_advance() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::dc_advance();
    }else{ untested();
    }
  }
  bool do_tr() override{ untested();
    if(owner()){ untested();
      return BASE_SUBCKT::do_tr();
    }else{ untested();
      return true;
    }
  }
  bool do_tr_last() override{ untested();
    if(owner()){ untested();
      return BASE_SUBCKT::do_tr_last();
    }else{ untested();
      return true;
    }
  }
  void do_ac() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::do_ac();
    }else{ untested();
    }
  }
  void tr_accept() override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_accept();
    }else{ untested();
    }
  }
  bool tr_needs_eval()const override{ untested();
    if(owner()){ untested();
      return BASE_SUBCKT::tr_needs_eval();
    }else{ untested();
      return false;
    }
  }
  void tr_queue_eval()override{ untested();
    if(owner()){ untested();
      BASE_SUBCKT::tr_queue_eval();
    }else{ untested();
    }
  }
#endif
private:
  COMPONENT const* prepare_dev(CARD const* proto);
private: // base class?
  void grow_nodes(int);
}ps;
DISPATCHER<CARD>::INSTALL ds(&device_dispatcher, "paramset", &ps);
/*--------------------------------------------------------------------------*/
PARAMSET::PARAMSET()
  :BASE_SUBCKT()
  ,_parent(NULL)
  ,_dev(NULL)
{
  attach_common(&Default_PARAMSET);
}
/*--------------------------------------------------------------------------*/
PARAMSET::PARAMSET(PARAMSET const& p)
  :BASE_SUBCKT(p)
  ,_parent(p._parent)
  ,_dev(p._dev)
{
  assert(has_common());
  _node_capacity = net_nodes();
  if(_node_capacity){
    _n = new node_t[_node_capacity];
  }else{
    assert(_n == NULL);
  }
  for (int ii = 0;  ii < net_nodes();  ++ii) {
    _n[ii] = p._n[ii];
  }
  new_subckt();

  // if(scope()){ untested();
  // }else{ untested();
  // }
}
/*--------------------------------------------------------------------------*/
CARD* PARAMSET::clone_instance() const
{
  PARAMSET* n = new PARAMSET(*this);
  assert(n->has_common());
  assert(has_common());
  auto c = prechecked_cast<COMMON_PARAMLIST*>(common()->clone());
  assert(c);
  c->_params = PARAM_LIST();
  n->attach_common(c);
  n->_parent = this;
  return n;
}
/*--------------------------------------------------------------------------*/
CARD* PARAMSET::clone() const
{
  PARAMSET* n = new PARAMSET(*this);
  assert(n->has_common());
  assert(has_common());
  if(this == &ps) {
    auto c = prechecked_cast<COMMON_PARAMLIST*>(common()->clone());
    assert(c);
    c->_params = PARAM_LIST();
    n->attach_common(c);
    n->_parent = this;
  }else{
    // TODO: hand out raw device if already precalcd?
  }
  return n;
}
/*--------------------------------------------------------------------------*/
bool PARAMSET::is_valid() const
{
  // assert(scope());
  assert(_parent);
  trace1("PARAMSET::is_valid", long_label());
  if(_parent->subckt()){
    PARAM_LIST const* params = _parent->subckt()->params();
    PARAM_INSTANCE v = params->deep_lookup("_..is_valid");
    Base const* x = v.e_val(nullptr, subckt());
    Integer c;
    Integer* res = c.assign(x);
    if(!res) {
      return true;
    }else{
      assert(x);
      trace3("PARAMSET::valid?", long_label(), v.string(), res->value());
      int a = res->value();
      delete res;
      return a;
    }
  }else{
    trace1("PARAMSET::invalid?", long_label());
    return false;
  }
}
/*--------------------------------------------------------------------------*/
void PARAMSET::grow_nodes(int Index)
{
  if(Index<_node_capacity){
  }else{
    int new_capacity = std::max(_node_capacity, node_capacity_floor);
    while(new_capacity <= Index) {
      assert(new_capacity < new_capacity * 2);
      new_capacity *= 2;
    }
    node_t* new_nodes = new node_t[new_capacity];
    for(int i=0; i<_node_capacity; ++i){
      new_nodes[i] = _n[i];
    }
    delete[] _n;
    _n = new_nodes;
    _node_capacity = new_capacity;
  }
}
/*--------------------------------------------------------------------------*/
int PARAMSET::set_param_by_name(std::string Name, std::string Value)
{
//  assert(_parent);

  if(Name=="$mfactor"){
    return BASE_SUBCKT::set_param_by_name(Name, Value);
  }else if(Name==""){ untested();
    throw Exception_No_Match("invalid parameter: " + Name);
  }else if(_parent && _parent->subckt()){
    trace2("PARAMSET::spbn2", long_label(), _parent->long_label());
    PARAM_LIST const* p = _parent->subckt()->params();

    if(p->find(Name) == p->end()){ untested();
      throw Exception_No_Match("invalid parameter: " + Name);
    }else{
    }
    return BASE_SUBCKT::set_param_by_name(Name, Value);
  }else{
    return BASE_SUBCKT::set_param_by_name(Name, Value);
  }

}
/*--------------------------------------------------------------------------*/
COMPONENT const* PARAMSET::prepare_dev(CARD const* proto)
{
  auto dev = prechecked_cast<COMPONENT const*>(proto);

#if 0
  // this does not work, proto could be a paramset.
  CARD* cl = proto->clone_instance(); // BUG: clone
  auto dev = prechecked_cast<COMPONENT const*>(proto);
  subckt()->push_back(dev);
  dev->set_owner(this);
  for(int i=0; i<dev->max_nodes(); ++i){ untested();
    std::string pn = dev->port_name(i);
    trace3("PARAMSET::prepare_dev", long_label(), i, pn);

    dev->set_port_by_index(i, pn);
  }
#endif

  if(dev){
  }else if(auto m = dynamic_cast<MODEL_CARD const*>(proto)) {
    dev = prechecked_cast<COMPONENT const*>(m->component_proto());
    assert(dev);
  }
  return dev;
}
/*--------------------------------------------------------------------------*/
void PARAMSET::precalc_first()
{
 // assert(scope());
  trace5("PARAMSET::pf", long_label(), owner(), _dev, _sim->is_first_expand(), !is_device());
  std::string base_name = dev_type();

  if(_dev){
    // assert(_dev == find_proto(base_name, NULL)); TODO.
  }else{
    CARD const* p = find_proto(base_name, NULL); //what if there are 2?
    if(!p){
      throw Exception_No_Match(base_name); // cmd.warn(bDANGER, here, "paramset: no match");
    }else{
    }
    _dev = prepare_dev(p);
  }

  if(!_dev){ untested();
    throw Exception_No_Match(base_name); // cmd.warn(bDANGER, here, "paramset: no match");
  }else{
  }

 // if(!owner()){
 //   // this does not work.
 // }else
  if(_parent && _parent->subckt()) {
    COMPONENT::precalc_first();
    assert(subckt());
    auto c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);

    {
      PARAM_LIST const* pl = _parent->subckt()->params();
      assert(pl);
      c->_params.set_try_again(const_cast<PARAM_LIST*>(pl));
    }

    subckt()->attach_params(&(c->_params), scope());
    if(_parent){
      assert(_parent->subckt());
      subckt()->params()->set_try_again(_parent->subckt()->params());
    }else{ untested();
      unreachable(); 
      subckt()->params()->set_try_again(scope()->params());
    }

    subckt()->precalc_first();
  }else{
    // auto c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    // assert(c);
    // PARAM_LIST const* pl = subckt()->params();
    // assert(pl);
    // c->_params.set_try_again(pl);
  }

  trace3("PARAMSET::pf done", long_label(), is_valid(), my_mfactor());

  assert(!is_constant()); /* because I have more work to do */
} // precalc_first
/*--------------------------------------------------------------------------*/
// THIS IS A HACK. variables with "_." prefix survive e_val. remove prefix.
static void demangle(std::string& value)
{
  assert(value.size());
  std::string::iterator i=value.begin();
  std::string::iterator j=i;
  ++j;
  for(; j!=value.end(); ){
    if(*i!='_'){
    }else if(*j!='.'){ untested();
    }else{
      // match
      *i = *j = ' ';
      ++j;
    }
    i = j;
    ++j;
  }
}
/*--------------------------------------------------------------------------*/
void resolve_copy(CARD_LIST* t, PARAM_LIST const& p, const CARD_LIST*)
{
  assert(t);
  PARAM_LIST& out = *t->params();

  for (PARAM_LIST::const_iterator i = p.begin(); i != p.end(); ++i) {
    if (i->second.has_hard_value()) {
      CS cmd(CS::_STRING, i->second.string());
      Expression f(cmd);
      CARD_LIST empty;
      empty.set_verilog_math();
      Expression e(f, &empty);
      Expression E;
	
      bool skip1 = false;
      for(auto ii=e.begin(); ii!=e.end(); ++ii){
	trace2("EXP", (*ii)->name(), dynamic_cast<Token_SYMBOL*>(*ii));
	trace2("EXP", (*ii)->name(), dynamic_cast<Token_CONSTANT*>(*ii));
	if(dynamic_cast<Token_PARLIST*>(*ii)){
	  skip1 = true;
	  E.push_back(*ii);
	}else if(skip1){
	  assert(dynamic_cast<Token_SYMBOL*>(*ii));
	  E.push_back(*ii);
	  skip1 = false;
	}else if(dynamic_cast<Token_CONSTANT*>(*ii)
	    && !dynamic_cast<const String*>((*ii)->data())
	    && !dynamic_cast<const Float*>((*ii)->data())
	    && !dynamic_cast<const Integer*>((*ii)->data())) { untested();
	  incomplete(); // probably;
		unreachable(); // unfixed m_expression_reduce gets us here.
	  E.push_back(new Token_SYMBOL("_." + (*ii)->name()));
	  delete(*ii);
	}else if(dynamic_cast<Token_SYMBOL*>(*ii)) {
	  assert(!dynamic_cast<const Float*>((*ii)->data()));
	  assert(!dynamic_cast<const Integer*>((*ii)->data()));
	  E.push_back(new Token_SYMBOL("_." + (*ii)->name()));
	  delete(*ii);
	}else{
	  E.push_back(*ii);
	}
      }
      while(e.size()){
	e.pop_back();
      }

      std::stringstream s;
      E.dump(s);
      out.set(i->first, s.str());
      trace2("resolve copy1", i->first, s.str());

    }else{ untested();
    }
  }
}
/*--------------------------------------------------------------------------*/
// BUG: this needs work
// - what is resolve_copy?
CARD* PARAMSET::deflate()
{
  trace2("PARAMSET::deflate", long_label(), subckt()->size());
  assert(is_valid());
  assert(subckt());
  subckt()->set_verilog_math();
  assert(subckt()->size()==1);
  CARD_LIST::iterator i = subckt()->begin();
  assert(*i);
  COMPONENT* dev = prechecked_cast<COMPONENT*>(*i);
  assert(dev);
  assert(dev->owner()==this);

  auto c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  auto pc = prechecked_cast<COMMON_PARAMLIST const*>(_parent->common());

  *subckt()->params() = PARAM_LIST();
  // c->_params set_try_again ...
  subckt()->params()->set_try_again(_parent->subckt()->params());
  trace0("PARAMSET::resolve?");
  resolve_copy(subckt(), c->_params, NULL);
  for(auto const& x : c->_params){
    trace1("debugp", x.first);
  }

  trace4("PARAMSET::deflate args fwd", dev->long_label(), dev->dev_type(), long_label(), dev_type());
  trace2("PARAMSET::deflate args fwd", dev->long_label(), my_mfactor());
  for(auto pi=pc->_params.begin(); pi!=pc->_params.end(); ++pi){
    trace3("PARAMSET::deflate args fwd2", dev->long_label(), pi->first, pi->second.string());
    CS cmd(CS::_STRING, pi->second.string());
    Expression e(cmd);
    Expression r(e, subckt());
    std::stringstream s;
    r.dump(s);

    std::string value = s.str();
    demangle(value);
    trace3("PARAMSET::deflate args fix", long_label(), pi->first, value);
    assert(pi->first!="");
    assert(pi->first!="$mfactor");
    // BUG? already set?
    dev->set_param_by_name(pi->first, "");
    dev->set_param_by_name(pi->first, value);
  }

  *i = NULL;
  dev->set_owner(NULL);
  dev->set_owner(owner());
  dev->set_label(short_label());

  CARD* deflated = dev->deflate();
  if(deflated == dev){
  }else if(dynamic_cast<PARAMSET const*>(dev)){
    // nested paramsets
    delete (CARD*) dev;
  }else{ untested();
    // what is it?
  }

  deflated->set_param_by_name("$mfactor", ""); // to string?
  deflated->set_param_by_name("$mfactor", to_string(my_mfactor())); // to string?
  auto dd = prechecked_cast<COMPONENT const*>(deflated);
  if(dd->common()){
  }else{
  }

  deflated->precalc_first();

  return deflated;
} // PARAMSET::deflate
/*--------------------------------------------------------------------------*/
void PARAMSET::expand()
{
  if(!is_device()){ untested();
    // not a device, not expandable.
  }else{
    BASE_SUBCKT::expand();
    trace2("PARAMSET::expand", long_label(), net_nodes());
    assert(common());
    auto proto = prechecked_cast<COMPONENT const*>(_parent);
    // assert(proto->owner() == nullptr);
    assert(proto);
    assert(proto->scope());
    assert(proto->subckt());
    assert(_parent->_dev);
    // assert(net_nodes() == proto->net_nodes());
    // assert(net_nodes() == _parent->_dev->net_nodes());
    assert(subckt());
    assert(scope()==owner()->subckt());

    auto c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());

    // here?
    PARAM_LIST const* pl = proto->subckt()->params();
    c->_params.set_try_again(const_cast<PARAM_LIST*>(pl));

    subckt()->attach_params(&(c->_params), scope());

    // renew_subckt(_parent, ... ) dev=sckt()->..?
    CARD* d = _parent->_dev->clone_instance();
    assert(d);
    auto dev = prechecked_cast<COMPONENT*>(d);
    assert(dev);

#if 1
    dev->set_owner(this);
    if(net_nodes() == dev->net_nodes()){
    }else if(net_nodes()){
      static std::string dummy;
      trace2("dummynode", long_label(), net_nodes());

      // make sure the nodes are allocated within dev.
      dev->set_port_by_index(net_nodes()-1, dummy);
    }else{ untested();
    }
    assert(net_nodes() == dev->net_nodes());
#endif

    trace4("PARAMSET::expand sp0", long_label(), net_nodes(), dev->net_nodes(), typeid(*dev).name());

    dev->set_parameters("_", this, dev->mutable_common(),
		     /*Value*/ 0., /*states*/ 0, NULL,
		     net_nodes(), _n);
    subckt()->push_back(d);
    assert(dev->owner() == this);

    {
      auto cp = prechecked_cast<COMMON_PARAMLIST const*>(proto->common());
      for(auto i=cp->_params.begin(); i!=cp->_params.end(); ++i){
      }
      for(auto i=cp->_params.begin(); i!=cp->_params.end(); ++i){
	trace2("PARAMSET::expand sp", i->first, i->second.string());

	if(i->first=="$mfactor"){ untested();
	}else{
	  dev->set_param_by_name(i->first, ""); // again? BUG?
	  dev->set_param_by_name(i->first, i->second.string());
	}
      }
      dev->precalc_first();
    }

    assert(subckt()->size()==1);

    subckt()->expand();
    if(dev->is_valid()){
    }else{
      // TODO: seems to be the wrong place. see mg_bug.1.gc
      throw Exception("invalid prototype for " + long_label());
    }
  }
} // expand
/*--------------------------------------------------------------------------*/
std::string PARAMSET::port_name(int i)const
{
  assert(_dev);
  return _dev->port_name(i);
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
