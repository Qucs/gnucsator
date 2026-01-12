/*$Id: lang_verilog.cc $ -*- C++ -*-
 * Copyright (C) 2007 Albert Davis
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
 */
//testing=script 2016.09.10
#include "u_nodemap.h"
#include "globals.h"
#include "c_comand.h"
#include "d_dot.h"
#include "d_coment.h"
#include "e_subckt.h"
#include "e_model.h"
#include "u_lang.h"
#include <stack>
/*--------------------------------------------------------------------------*/
static const std::string IS_VALID = "_..is_valid";
/*--------------------------------------------------------------------------*/
std::stack<CARD*> owner_hack;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class LANG_VERILOG : public LANGUAGE {
  enum MODE {mDEFAULT, mPARAMSET} _mode;
  mutable int arg_count;
  enum {INACTIVE = -1};
public:
  LANG_VERILOG() : arg_count(INACTIVE) {}
  ~LANG_VERILOG() {}
  std::string name()const override {return "verilog";}
  bool case_insensitive()const override {return false;}
  UNITS units()const override {return uSI;}

public: // override virtual, used by callback
  std::string arg_front()const override {untested();
    switch (_mode) {
    case mPARAMSET:untested(); return " .";			    break;
    case mDEFAULT:untested();  return (arg_count++ > 0) ? ", ." : "."; break;
    }
    unreachable();
    return "";
  }
  std::string arg_mid()const override {untested();
    switch (_mode) {
    case mPARAMSET:untested(); return "="; break;
    case mDEFAULT:untested();  return "("; break;
    }
    unreachable();
    return "";
  }
  std::string arg_back()const override {untested();
    switch (_mode) {
    case mPARAMSET:untested(); return ";"; break;
    case mDEFAULT:untested();  return ")"; break;
    }
    unreachable();
    return "";
  }

public: // override virtual, called by commands
  void		parse_top_item(CS&, CARD_LIST*)override;
  DEV_COMMENT*	parse_comment(CS&, DEV_COMMENT*)override;
  DEV_DOT*	parse_command(CS&, DEV_DOT*)override;
  CARD*		parse_paramset(CS&, CARD*) /* override */;
  COMPONENT*	parse_paramset_(CS&, BASE_SUBCKT*);
  CARD*		obsolete_parse_modelcard(CS&, MODEL_CARD*);
  BASE_SUBCKT*  parse_module(CS&, BASE_SUBCKT*)override;
  COMPONENT*	parse_instance(CS&, COMPONENT*)override;
  std::string	find_type_in_string(CS&)override;
private: // local
  void print_attributes(OMSTREAM&, tag_t);
  void skip_attributes(CS& cmd);
  std::string  parse_attributes(CS& cmd);
  void parse_type(CS& cmd, CARD* x);
  void parse_args_paramset(CS& cmd, MODEL_CARD* x);
  void parse_args_instance(CS& cmd, CARD* x);
  void store_attributes(std::string attrib_string, tag_t x);
  void parse_attributes(CS& cmd, tag_t x);
  void parse_args_paramset_(CS& cmd, CARD* x);
//  void parse_args_instance(CS& cmd, CARD* x); 
//  void parse_label(CS& cmd, CARD* x);
  void parse_ports(CS& cmd, COMPONENT* x, bool all_new);

private: // TODO, transition, stale pure virtuals...
  MODEL_CARD*	parse_paramset(CS&, MODEL_CARD*)override
  { untested(); incomplete(); unreachable(); return NULL; }

private: // override virtual, called by print_item
  void print_paramset(OMSTREAM&, const MODEL_CARD*)override;
  void print_module(OMSTREAM&, const BASE_SUBCKT*)override;
  void print_instance(OMSTREAM&, const COMPONENT*)override;
  void print_comment(OMSTREAM&, const DEV_COMMENT*)override;
  void print_command(OMSTREAM& o, const DEV_DOT* c)override;
private: // local
  void print_args(OMSTREAM&, const MODEL_CARD*);
  void print_args(OMSTREAM&, const COMPONENT*);
} lang_verilog;

DISPATCHER<LANGUAGE>::INSTALL
	d(&language_dispatcher, lang_verilog.name(), &lang_verilog);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::skip_attributes(CS& cmd)
{
  while (cmd >> "(*") { untested();
    cmd.skipto1('*') && (cmd >> "*)");
  }
}
/*--------------------------------------------------------------------------*/
std::string LANG_VERILOG::parse_attributes(CS& cmd)
{
  std::string attrib_string = "";
  std::string comma = "";
  while (cmd >> "(*") { untested();
    attrib_string += comma;
    while(cmd.ns_more() && !(cmd >> "*)")) { untested();
      attrib_string += cmd.ctoc();
    }
    comma = ", ";
  }
  return attrib_string;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::store_attributes(std::string attrib_string, tag_t x)
{
  assert(x);
  if(attrib_string!=""){ untested();
    set_attributes(x).add_to(attrib_string, x);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_attributes(CS& cmd, tag_t x)
{
  assert(x);
  store_attributes(parse_attributes(cmd), x);
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_type(CS& cmd, CARD* x)
{
  assert(x);
  std::string new_type;
  cmd >> new_type;
  x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_args_paramset_(CS& cmd, CARD* x)
{ untested();
  assert(x);

  while (cmd >> '.') { untested();
    size_t here = cmd.cursor();
    std::string Name, value;
    try{ untested();
      cmd >> Name >> '=' >> value >> ';';
      x->set_param_by_name(Name, value);
    }catch (Exception_No_Match&) {untested();
      cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + Name + " ignored");
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_args_paramset(CS& cmd, MODEL_CARD* x)
{ untested();
  assert(x);

  while (cmd >> '.') { untested();
    size_t here = cmd.cursor();
    std::string Name, value;
    try{ untested();
      cmd >> Name >> '=' >> value >> ';';
      x->set_param_by_name(Name, value);
    }catch (Exception_No_Match&) {untested();
      cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + Name + " ignored");
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_args_instance(CS& cmd, CARD* x)
{
  assert(x);

  if (cmd >> "#(") {
    std::string attribs = parse_attributes(cmd);
    size_t here = cmd.cursor();
    
    if (cmd.match1('.')) {
      // by name
      while (cmd >> '.') {
	std::string Name  = cmd.ctos("(", "", "");
	std::string value = cmd.ctos(",)", "(", ")");
	cmd >> ',';
	try{
	  int Index = x->set_param_by_name(Name, value);
	  trace3("pai", Index, Name, value);
	  store_attributes(attribs,  x->param_id_tag(Index));
	}catch (Exception_No_Match&) {
	  cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + Name + " ignored");
	}catch (Exception_Clash&) {
	  cmd.warn(bDANGER, here, x->long_label() + ": already set " + Name + ", ignored");
	}
	attribs = parse_attributes(cmd);
	here = cmd.cursor();
      }
    }else{
      // by order
      for (int Index = 0;  cmd.is_alnum() || cmd.match1("+-.");  ++Index) {
	try{
	  std::string value = cmd.ctos(",)", "", "");
	  x->set_param_by_index(Index, value, 0/*offset*/);
	  store_attributes(attribs,  x->param_id_tag(Index));
	}catch (Exception_Too_Many& e) {untested();
	  cmd.warn(bDANGER, here, e.message());
	}catch (Exception_Clash&) {untested();
	  unreachable();
	  cmd.warn(bDANGER, here, x->long_label() + ": already set, ignored");
	}
	attribs = parse_attributes(cmd);
	here = cmd.cursor();
      }
    }
    cmd >> ')';
  }else if (cmd >> "#") {
    std::string arg;
    cmd >> arg;
    x->set_param_by_index(0, arg, 0);
  }else{
    // no args
  }
}
/*--------------------------------------------------------------------------*/
static void parse_label(CS& cmd, CARD* x)
{
  assert(x);
  std::string my_name;
  if (cmd >> my_name) {
    x->set_label(my_name);
  }else{ untested();
    x->set_label(x->id_letter() + std::string("_unnamed")); //BUG// not unique
    cmd.warn(bDANGER, "label required");
  }
}
/*--------------------------------------------------------------------------*/
// map to verilog representation
std::string mangle(std::string const& name)
{
  if(isdigit(name[0])) {
    return '\\' + name + " ";
  }else if(name[0] == '\\') { untested();
    return name + " ";
  }else{
    // ok, for now.
    // probably need '\\' ... ' ' whenever special characters are used.
    return name;
  }
}
/*--------------------------------------------------------------------------*/
// get identifier and turn into internal representation
// "\1 " -> "1"     -- so it also works with spice
// "\a " -> "a"     -- identical, use simple form
// "\$ " -> "\$"   -- not sure
// "\a* " -> "\a*" -- keep escaped string
// "\\\ " -> "\\\" -- keep escaped string
std::string get_identifier(CS& cmd, std::string const& term)
{
  cmd.skipbl();
  std::string id;

  if(cmd.is_digit()) { untested();
    cmd.warn(bDANGER, "invalid identifier");
  }else{
  }

  if(cmd >> "\\") {
    id = cmd.get_to(" \t\f");
    trace1("got to", cmd.peek());
    cmd.skip();

    {
      bool plain = true;
      for(size_t i = 0; plain && i<id.size() ; ++i) {
	if (isalnum(id[i])) {
	}else if (id[i] == '$') { untested();
	  plain = false;
	}else{
	  plain = false;
	}
      }

      if(plain) {
	// don't touch, for now.
      }else{
	// store escaped string.
	id = "\\" + id;
      }
    }
  }else{
    id = cmd.ctos(term, "", "");
  }

  trace1("identifier", id);
  return id;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_ports(CS& cmd, COMPONENT* x, bool all_new)
{
  assert(x);

  if (cmd >> '(') {
    std::string attribs = parse_attributes(cmd);
    size_t here = cmd.cursor();
    
    if (cmd.match1('.')) {
      // by name
      while (cmd >> '.') {
	std::string Name = get_identifier(cmd, "(");
	int paren = cmd.skip1b('(');
	std::string value = get_identifier(cmd, ")");
	if (!paren){ untested();
	  //?
	}else if( cmd.skip1b(')')) {
	}else{untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": need ')'");
	}
	cmd >> ',';
	try{
	  int Index = x->set_port_by_name(Name, value);
	  store_attributes(attribs,  x->port_id_tag(Index));
	}catch (Exception_No_Match&) { untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": mismatch " + Name + " ignored");
	}catch (Exception_Clash&) {untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": already set " + Name + ", ignored");
	}
	attribs = parse_attributes(cmd);
	here = cmd.cursor();
      }
      for (int Index = 0;  Index < x->min_nodes();  ++Index) {
	//BUG// This may be a bad idea.
	//BUG// It's definitely wrong with all_new.
	//BUG// What should we do with unconnected ports?
	if (!(x->node_is_connected(Index))) {untested();
	  cmd.warn(bDANGER, x->port_name(Index) + ": port unconnected, grounding");
	  x->set_port_to_ground(Index);
	}else{
	}
      }
    }else{
      // by order
      int Index;
      for (Index = 0;  cmd.is_alnum() || cmd.peek() == '\\';  ++Index) {
	try{
	  std::string value = get_identifier(cmd, ",)");
	  cmd >> ',';
	  x->set_port_by_index(Index, value);
	  store_attributes(attribs,  x->port_id_tag(Index));
	  if (all_new) {
	    if (x->node_is_grounded(Index)) { untested();
	      cmd.warn(bDANGER, here, "node 0 not allowed here");
	      --Index;
	    }else if (x->subckt() && x->subckt()->nodes()->how_many() != Index+1) { untested();
	      cmd.warn(bDANGER, here, "duplicate port name, skipping");
	      --Index;
	    }else{
	    }
	  }else{
	  }
	}catch (Exception_Too_Many& e) { untested();
	  cmd.warn(bDANGER, here, e.message());
	}catch (Exception_Clash&) {untested();
	  unreachable();
	  cmd.warn(bDANGER, here, x->long_label() + ": already set, ignored");
	}
	attribs = parse_attributes(cmd);
	here = cmd.cursor();
      }
      if (Index < x->min_nodes()) { untested();
	//BUG// This may be a bad idea.
	//BUG// It's definitely wrong with all_new.
	//BUG// What should we do with unconnected ports?
	if (all_new) {untested();
	}else{ untested();
	}
	cmd.warn(bDANGER, "need " + to_string(x->min_nodes()-Index) +" more nodes, grounding");
	for (  ;  Index < x->min_nodes();  ++Index) { untested();
	  x->set_port_to_ground(Index);
	}
      }else{
      }
    }
    cmd >> ')';
  }else{ untested();
    cmd.warn(bDANGER, "'(' required (parse ports) (grounding)");
    for (int Index = 0;  Index < x->min_nodes();  ++Index) { untested();
      if (!(x->node_is_connected(Index))) { untested();
	if (all_new) {untested();
	}else{ untested();
	}
	cmd.warn(bDANGER, x->port_name(Index) + ": port unconnected, grounding");
	x->set_port_to_ground(Index);
      }else{untested();
	unreachable();
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_COMMENT* LANG_VERILOG::parse_comment(CS& cmd, DEV_COMMENT* x)
{
  assert(x);
  x->set(cmd.fullstring());
  return x;
}
/*--------------------------------------------------------------------------*/
DEV_DOT* LANG_VERILOG::parse_command(CS& cmd, DEV_DOT* x)
{
  assert(x);
  // x has an owner. it gets lost here.
  x->set(cmd.fullstring());

  // owner is a BASE_SUBCKT....
  // how can i set _subckt in a BASE_SUBCKT?
  CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;
  cmd.reset();
  cmd.skipbl();
  if(cmd.peek() == '`'){
  }else{
    // "module" etc gets here.
  }
  parse_attributes(cmd, x->id_tag());

  // if cmd is a `preprocessor directive, then x->owner will be useful...

  if(cmd.peek()=='`'){
    trace1("PUSH SCOPE", cmd.fullstring());
    owner_hack.push(x->owner());
    CMD::cmdproc(cmd, scope);
    trace1("POP SCOPE", cmd.fullstring());
    owner_hack.pop();
  }else{
    CMD::cmdproc(cmd, scope);
  }

  delete x;
  return NULL;
} // parse_command
/*--------------------------------------------------------------------------*/
/* "paramset" <my_name> <base_name> ";"
 *    <paramset_item_declaration>*
 *    <paramset_statement>*
 *  "endparamset"
 */
CARD* LANG_VERILOG::parse_paramset(CS& cmd, CARD* x)
{ untested();
  if(auto c = dynamic_cast<BASE_SUBCKT*>(x)) { untested();
    return parse_paramset_(cmd, c);
  }else if(auto m = dynamic_cast<MODEL_CARD*>(x)) { untested();
    //BUG// no paramset_item_declaration, falls back to spice mode
    return obsolete_parse_modelcard(cmd, m);
  }else{ untested();
    unreachable();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
#if 0
MODEL_CARD* LANG_VERILOG::parse_paramset(CS& cmd, MODEL_CARD* x)
{ untested();
  assert(x);
  cmd.reset();
  cmd >> "paramset ";
  parse_label(cmd, x);
  parse_type(cmd, x);
  cmd >> ';';

  for (;;) { untested();
    parse_args_paramset(cmd, x);
    if (cmd >> "endparamset ") { untested();
      break;
    }else if (!cmd.more()) { untested();
      cmd.get_line("verilog-paramset>");
    }else{untested();
      cmd.check(bWARNING, "what's this?");
      break;
    }
  }
  return x;
}
#endif
/*--------------------------------------------------------------------------*/
CARD* LANG_VERILOG::obsolete_parse_modelcard(CS& cmd, MODEL_CARD* x)
{ untested();
  assert(x);
  cmd.reset();
  cmd >> "paramset ";
  parse_label(cmd, x);
  parse_type(cmd, x);
  cmd >> ';';

  for (;;) { untested();
    parse_args_paramset(cmd, x);
    if (cmd >> "endparamset ") { untested();
      break;
    }else if (!cmd.more()) { untested();
      cmd.get_line("verilog-paramset>");
    }else{ untested();
      cmd.check(bWARNING, "what's this?");
      break;
    }
  }
  return x;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CMD_PARAM : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    assert(Scope);
    PARAM_LIST* pl = Scope->params();
    if (cmd.is_end()) { untested();
      pl->print(IO::mstdout, OPT::language);
      IO::mstdout << '\n';
    }else{
      parse(cmd, pl);
      // DEV_DOT* dd = new DEV_DOT();
      // assert(dd);
      // dd->set(cmd.fullstring());
      // Scope->push_back(dd);
    }
  }
private:
  void parse(CS& cmd, PARAM_LIST*)const;
  void parse_def(CS& cmd, PARAM_INSTANCE& par)const;
  void parse_range(CS& cmd, PARAM_LIST* Scope, std::string Name)const;
} module_param;
/*--------------------------------------------------------------------------*/
// essentially PARAM_INSTANCE::PARAM_NONE, untyped parameter
// but resolve to verilog types.
class PARAM_ANY : public PARA_BASE {
  mutable Base* _value{nullptr};
public:
  explicit PARAM_ANY() : PARA_BASE () {}
  explicit PARAM_ANY(PARAM_ANY const&p) : PARA_BASE (p) {
    if(p._value){
      _value = p._value->clone();
    }else{
    }
  }
  ~PARAM_ANY() { delete _value; _value=nullptr;}
  PARA_BASE* clone()const override{ untested();return new PARAM_ANY(*this);}
  PARA_BASE* pclone(void*p)const override{return new(p) PARAM_ANY(*this);}
  bool operator==(const PARA_BASE& v)const {
    auto p = dynamic_cast<PARAM_ANY const*>(&v);
    // return (p && _v == p->_v  &&  _s == p->_s);
    Base* eq = nullptr;
    bool ret = false;
    if(!p || _s != p->_s) {
      return false;
    }else if(value()==nullptr && v.value()==nullptr) {
      return true;
    }else if(auto f = dynamic_cast<Float const*>(v.value())){
      eq = f->equal(_value);
    }else if(auto i = dynamic_cast<Integer const*>(v.value())){
      eq = i->equal(_value);
    }else if(v.value() == nullptr) { untested();
      ret = _value == nullptr;
    }else{ untested();
      incomplete();
    }

    if(auto ii=dynamic_cast<Integer const*>(eq)){
      ret = ii->value();
    }else{ untested();
    }
    delete eq;

    //if (_value) { untested();
    //  Integer* eq = _value->equal(v.value());
    //  if(eq){ untested();
    //    bool eq = v->_value;
    //    delete eq;
    //    return eq;
    //  }else{ untested();
    //  }
    //}else{ untested();
    //  return !v._value || !has_hard_value();
    //}
    if(ret){
    }else{ untested();
    }
    return ret;
  }

  void parse(CS&) override { untested(); unreachable(); }
  PARA_BASE& operator=(const std::string&s) override{ _s = s; return *this;}
  PARA_BASE& operator=(Base const* v)override {
    delete _value;
    _value = nullptr;
    if(dynamic_cast<Float const*>(v)){
      vReal n;
      _value = n.assign(v);
      trace3("now real", _s, v->val_string(), _value->val_string());
    }else if(dynamic_cast<Integer const*>(v)){
      vInteger n;
      _value = n.assign(v);
      trace3("now integer", _s, v->val_string(), _value->val_string());
    }else{ untested();
      _value = v->assign(v);
      assert(_value);
      trace3("don't know", _s, v->val_string(), _value->val_string());
    }
    assert(_value);
    _s = "#";
    return *this;
  }
  std::string string()const override {
    if (_s == "#") {
      if (_value) {
	return _value->val_string();
      }else{ untested();
	return "";
      }
   // }else if (_s == "") { untested();
   //   return "NA(" + _value->val_string() + ")";
    }else{
      return _s;
    }
  }

  Base const* value()const override {
    if(_value){
      trace1("value", typeid(*_value).name());
    }else{
    }
    return _value;
  }
  bool has_good_value()const override { untested();unreachable(); return false;}
  Base const* e_val_(const Base* def, const PARAM_LIST* s, int)const override { untested();
    // def does not seem to carry type info...
    // see s_dc.vcvs1{a,b,c}.gc
    error(bDEBUG, "assuming double in " + _s + "\n");
    PARAMETER<double> pp;
    PARAM_INSTANCE pi;
    pi = pp;
    pi = _s;
    Base const* v = pi.e_val(def, s);
    assert(v);
    _value = v->clone(); // v belongs to pi.
    return _value;
  }
}param_any;
/*--------------------------------------------------------------------------*/
void CMD_PARAM::parse_def(CS& cmd, PARAM_INSTANCE& par) const
{
   // BUG // need to tokenize right here. strings may contain separators etc.
  Expression e(cmd);
  std::stringstream s;
  e.dump(s);

  trace2("got default expression", s.str(), cmd.tail());
  par = s.str();
}
/*--------------------------------------------------------------------------*/
void CMD_PARAM::parse(CS& cmd, PARAM_LIST* pl) const
{
  PARAM_INSTANCE par;
  if(cmd >> "real"){
    par = PARAMETER<vReal>();
  }else if(cmd >> "integer"){ untested();
    par = PARAMETER<vInteger>();
  }else if(cmd >> "string"){
    par = PARAMETER<vString>();
  }else{
    // TODO: realtime | time
    par = PARAM_ANY();
  }
  size_t here = cmd.cursor();
  for (;;) {
    if (!(cmd.more() && (cmd.is_alpha() || cmd.match1('_')))) { untested();
      break;
    }else{
    }
    std::string Name;
    cmd >> Name;
    trace1("CMD_PARAM::parse", Name);
    par = "";
    if(cmd.skip1('=')) {
      parse_def(cmd, par);
    }else{
    }
    pl->set(Name, par);
    trace3("parsed", Name, par.string(), cmd.tail());

    if(cmd >> ';') {
      assert(cmd);
      break;
    }else if(cmd >> ',') {
    }else{
      parse_range(cmd, pl, Name);
    }

    if(cmd >> ';') {
      break;
    }else if(cmd >> ',') { untested();
    }

    if (cmd.stuck(&here)) { untested();
      break;
    }else{
    }
  }
  if(!cmd){ untested();
    cmd.warn(bDANGER, "syntax error");
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void CMD_PARAM::parse_range(CS& cmd, PARAM_LIST* Scope, std::string Name) const
{
  assert(Scope);
  Scope->set_verilog();
  PARAM_LIST* pl = Scope;
  {
    std::string range_expr = "1";
    std::string range_type;
    size_t here = cmd.cursor();

    for(;;){
      if(cmd >> "from "){
	range_expr = range_expr + "*(";
      }else if(cmd>> "exclude ") { untested();
	range_expr = range_expr + "*(1-";
      }else{
	break;
      }

      range_type = cmd.trimmed_last_match();
      std::string what;
      std::string lb, ub, uo, lo;
      enum { none = 0, interval = 1, set = 2 } range = none;
      if(cmd.skip1('[')){
	lo = "<=";
	range=interval;
      }else if(cmd.skip1('(')){ untested();
	lo = "<";
	range=interval;
      }else if(cmd.peek() == '\''){ untested();
	range=set;
      }else{ untested();
      }

      if(range == set){ untested();
	Expression L;
	cmd >> L;
	Expression LL(L, Scope); // reduce_copy.
	if(LL.size()>2){ untested();
	  incomplete();
	  auto t = LL.begin();
	  ++t;
	  what = "!(1";
	  std::string n = (*t)->name();
	  while (++t != LL.end()){ untested();
	    what += "*(" + n + "!=" + Name + ")";
	    n = (*t)->name();
	  }
	  what += ")";
	}else{ untested();
	  incomplete();
	}

      }else if(range){
	{
	  Expression L;
	  trace1("lb?", cmd.tail());
	  cmd >> L;
	  // lb = cmd.ctos(":");
	  Expression LL(L, Scope); // reduce_copy.
	  Base const* lb_ = LL.value();
	  assert(lb_);
	  if(!lb_){ untested();
	    incomplete();
	  }else if(auto ii = dynamic_cast<Integer const*>(lb_)){
	    incomplete();
	    lb = to_string(ii->value());
	  }else if(auto ff = dynamic_cast<Float const*>(lb_)){
	    incomplete();
	    lb = to_string(ff->value());
	  }else{ untested();
	    incomplete();
	  }
	}

	cmd.skip1(":");
	//	  ub = cmd.ctos("])");
	{
	  Expression L;
	  cmd >> L;
	  // lb = cmd.ctos(":");
	  Expression LL(L, Scope); // reduce_copy.
	  Base const* ub_ = LL.value();
	  if(!ub_){ untested();
	    incomplete();
	  }else if(auto ii = dynamic_cast<Integer const*>(ub_)){
	    ub = to_string(ii->value());
	  }else if(auto ff = dynamic_cast<Float const*>(ub_)){ untested();
	    ub = to_string(ff->value());
	  }else{ untested();
	    incomplete();
	  }
	}
	trace2("bounds", lb, ub);

	if(cmd.skip1(']')){
	  uo = "<=";
	}else if(cmd.skip1(')')){ untested();
	  uo = "<";
	}else{ untested();
	}
	what = "(" + lb + lo + Name + ")*(" + Name + uo + ub + ")";
      }else{ untested();
	cmd >> what;
	what = "(" + Name + "==" + what + ")";
      }
      trace1("condition", what);
      range_expr = range_expr + what + ")";
      if(cmd.skip1(',')){ untested();
	break;
      }else{
      }
    } // from/exclude loop

    if (cmd.stuck(&here)) { untested();
      incomplete();
      trace2("c_param stuck", cmd.tail(), range_expr);
      return;
    }else{
      trace2("c_param more", cmd.tail(), range_expr);
    }

    //  if(Value.string() == "NA( NA)"){ untested();
    //    pl->set(Name, "");
    //  }else{ untested();
    //    pl->set(Name, Value.string());
    //  }
    // trace3("set check", Name, (*pl)[Name], (*pl)[Name].string());

    PARAM_INSTANCE v = pl->deep_lookup(IS_VALID);
    if(range_expr=="1"){ untested();
    }else if(v.has_hard_value()){ untested();
      range_expr = v.string() + "*" + range_expr;
      trace2("c_param hv", IS_VALID, range_expr);
      pl->set(IS_VALID, range_expr);
    }else{
      v = PARAMETER<vInteger>();
      trace2("c_param", IS_VALID, range_expr);
      pl->set(IS_VALID, range_expr);
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_VERILOG::parse_paramset_(CS& cmd, BASE_SUBCKT* x)
{ untested();
  assert(x);

  trace1("parse_paramset_", cmd.fullstring());
  trace1("parse_paramset_", cmd.tail());

  // move_attributes(tag_t(&cmd), tag_t(x)); incomplete.
  cmd >> "paramset ";
  parse_label(cmd, x);
  parse_type(cmd, x);
  cmd >> ';';

  x->subckt()->set_verilog_math();

  for (;;) { untested();
    size_t here = cmd.cursor();
    if (cmd >> "parameter ") { untested();
      module_param.do_it(cmd, x->subckt());
      trace1("done parameter", cmd.tail());
    }else if (cmd >> "//") { untested();
      cmd.reset(here);
      // new__instance(cmd, x, x->subckt()); // BUG
      cmd.get_line("verilog-paramset>");
    }else if (!cmd.more()) { untested();
      cmd.get_line("verilog-paramset>");
    }else{ untested();
      break;
    }
  }

  for (;;) { untested();
    parse_args_paramset_(cmd, x);
    size_t here = cmd.cursor();
    if (cmd >> "endparamset ") { untested();
      break;
    }else if (cmd >> "// ") { untested();
      cmd.reset(here);
      // new__instance(cmd, x, x->subckt()); // BUG
      cmd.get_line("verilog-paramset>");
    }else if (!cmd.more()) { untested();
      cmd.get_line("verilog-paramset>");
    }else{ untested();
      cmd.check(bWARNING, "what's this?");
      break;
    }
  }
  trace2("LANG_VERILOG::parse_paramset_ done", x->long_label(), ((CARD*)x)->dev_type());
  return x;
}
/*--------------------------------------------------------------------------*/
/* "module" <name> "(" <ports> ")" ";"
 *    <declarations>
 *    <netlist>
 * "endmodule"
 */
//BUG// strictly one device per line

BASE_SUBCKT* LANG_VERILOG::parse_module(CS& cmd, BASE_SUBCKT* x)
{
  assert(x->subckt());
  x->subckt()->set_verilog_math();

  // header
  cmd.reset();
  parse_attributes(cmd, x->id_tag());
  (cmd >> "module |macromodule ");
  parse_label(cmd, x);
  parse_ports(cmd, x, true/*all new*/);
  cmd >> ';';

  bool have_instance = false;

  // body
  for (;;) {

    cmd.get_line("verilog-module>");
    if (cmd >> "endmodule ") {
      break;
    }else if (!have_instance && (cmd >> "parameter ")) {
      module_param.do_it(cmd, x->subckt());
    }else if (cmd >> "//") {
      cmd.reset();
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "ground ") {
      cmd.reset();
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "paramset ") { untested();
      cmd.reset();
      cmd.check(bDANGER, "ERROR: This will not work. Need top level.");
      new__instance(cmd, x, x->subckt());
    }else{
      have_instance = true;
      BASE_SUBCKT* new_instance = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("instance"));
      assert(new_instance);
      CARD_LIST* Scope = x->subckt();
      trace3("parse_module instance", cmd.fullstring(), Scope, Scope->nodes());
      assert(Scope);

      new_instance->set_owner(x);
      parse_instance(cmd, new_instance);

      Scope->push_back(new_instance);
    }
  }
  return x;
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_VERILOG::parse_instance(CS& cmd, COMPONENT* x)
{
  assert(x);
  cmd.reset();
  parse_attributes(cmd, x->id_tag());
  parse_type(cmd, x);
  parse_args_instance(cmd, x);
  parse_label(cmd, x);
  parse_ports(cmd, x, false/*allow dups*/);
  cmd >> ';';
  cmd.check(bWARNING, "what's this?");
  return x;
}
/*--------------------------------------------------------------------------*/
std::string LANG_VERILOG::find_type_in_string(CS& cmd)
{
  skip_attributes(cmd);
  size_t here = cmd.cursor();
  std::string type;
  if ((cmd >> "//")) {
    //assert(here == 0);
    type = "dev_comment";
  }else{
    cmd >> type;
  }
  cmd.reset(here); // where the type is.
  return type;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
  cmd.get_line("gnucap-verilog>");
  new__instance(cmd, NULL, Scope);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_attributes(OMSTREAM& o, tag_t x)
{
  assert(x);

  if (has_attributes(x)) {
    std::string s = attributes(x)->string(x);
    if(s.size()) {
      o << "(* " << s << " *) ";
    }else{
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const MODEL_CARD* x)
{
  assert(x);
  if (x->use_obsolete_callback_print()) { untested();
    x->print_args_obsolete_callback(o, this);  //BUG//callback//
  }else{
    for (int ii = 0; ii < x->param_count(); ++ii) {
      if (x->param_is_printable(ii)) {
	std::string arg = " ." + x->param_name(ii) + '=' + x->param_value(ii) + ';';
	o << arg;
      }else{
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << " #(";
  if (x->use_obsolete_callback_print()) {
    arg_count = 0;
    x->print_args_obsolete_callback(o, this);  //BUG//callback//
    arg_count = INACTIVE;
  }else{
    std::string sep = "";
    //for (int ii = x->param_count() - 1;  ii >= 0;  --ii)
    for (int ii = 0; ii < x->param_count(); ++ii) {
      if (x->param_is_printable(ii)) {
	o << sep;
	print_attributes(o, x->param_id_tag(ii));
	o << '.' << x->param_name(ii) << '(' << x->param_value(ii) << ')';
	sep = ',';
      }else{
      }
    }
  }
  o << ") ";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void print_type(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << x->dev_type();
}
/*--------------------------------------------------------------------------*/
static void print_label(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << x->short_label();
}
/*--------------------------------------------------------------------------*/
static void print_ports_long(OMSTREAM& o, const COMPONENT* x)
{
  // print in long form ...    .name(value)
  assert(x);

  o << " (";
  std::string sep = "";
  for (int ii = 0;  x->port_exists(ii);  ++ii) {
    o << sep;
    if(x->port_name(ii) != ""){
      o << '.' << mangle(x->port_name(ii)) << '(' << mangle(x->port_value(ii)) << ')';
    }else{
      o << mangle(x->port_value(ii));
    }
    sep = ",";
  }
  o << ")";
}
/*--------------------------------------------------------------------------*/
static void print_ports_short(OMSTREAM& o, const COMPONENT* x)
{
  // print in short form ...   value only
  assert(x);

  o << " (";
  std::string sep = "";
  for (int ii = 0;  x->port_exists(ii);  ++ii) {
    o << sep << x->port_value(ii);
    sep = ",";
  }
  o << ")";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_paramset(OMSTREAM& o, const MODEL_CARD* x)
{
  assert(x);
  _mode = mPARAMSET;
  o << "paramset " << x->short_label() << ' ' << x->dev_type() << ";\\\n";
  print_args(o, x);
  o << "\\\n"
    "endparmset\n\n";
  _mode = mDEFAULT;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_module(OMSTREAM& o, const BASE_SUBCKT* x)
{
  assert(x);
  assert(x->subckt());
  assert(x->subckt()->params());

  o << "module " <<  x->short_label();
  print_ports_short(o, x);
  o << ";\n";
  PARAM_LIST const& pl = *x->subckt()->params();
  for(int i=0; i<pl.size(); ++i){
    o << "parameter " << pl.name(i) << "=" << pl[i] << ";\n";
  }

  for (CARD_LIST::const_iterator 
	 ci = x->subckt()->begin(); ci != x->subckt()->end(); ++ci) {
    print_item(o, *ci);
  }
  
  o << "endmodule // " << x->short_label() << "\n\n";
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_instance(OMSTREAM& o, const COMPONENT* x)
{
  print_type(o, x);
  print_args(o, x);
  print_label(o, x);
  print_ports_long(o, x);
  o << ";\n";
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_comment(OMSTREAM& o, const DEV_COMMENT* x)
{
  assert(x);
  if ((x->comment().compare(0, 2, "//")) != 0) {
    o << "//";
  }else{
  }
  o << x->comment() << '\n';
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_command(OMSTREAM& o, const DEV_DOT* x)
{
  assert(x);
  if(x->s().size()){
    o << x->s() << '\n';
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CMD_PARAMSET : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope) override { untested();
    if(Scope == &CARD_LIST::card_list){ untested();
    }else{ untested();
      // allowed?
    }
    // already got "paramset"
    std::string my_name, base_name;
    size_t here = cmd.cursor();
    cmd >> my_name >> base_name;
    // cmd >> model_name;

    assert(OPT::language);
    const CARD* proto = OPT::language->find_proto(base_name, NULL);
    CARD* paramset = NULL;

    if(auto model=dynamic_cast<MODEL_CARD const*>(proto)){ untested();
      trace1("CMD_PARAMSET::do_it MODEL_CARD", base_name);
      // Spice compatibility mode. fix later.
      // // BUG also hits paramset //

      paramset = model->clone();
      auto mc = prechecked_cast<MODEL_CARD*>(paramset);
      assert(mc);
      OPT::language->parse_paramset(cmd, mc);
    }else{ untested();
      paramset = device_dispatcher.clone("paramset");
      assert(paramset);
      auto dev = prechecked_cast<BASE_SUBCKT*>(paramset);
      assert(dev);
      //assert(dev->scope());

      // OPT::language->parse_paramset(cmd, dev);
      cmd.reset(here); // really?
      lang_verilog.parse_paramset(cmd, dev);
    }
    trace3("CMD_PARAMSET", paramset->long_label(), paramset->dev_type(), paramset);
    Scope->push_back(paramset);
  }
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "paramset", &p1);
/*--------------------------------------------------------------------------*/
class CMD_MODULE : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    BASE_SUBCKT* new_module = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("module"));
    assert(new_module);
    // assert(!new_module->owner());
    new_module->set_owner(nullptr);
    assert(new_module->subckt());
    assert(new_module->subckt()->is_empty());
    assert(!new_module->is_device());
    try {
      lang_verilog.parse_module(cmd, new_module);
      Scope->push_back(new_module);
    }catch(Exception const& e) { untested();
      cmd.warn(bDANGER, e.message());
      for (;;) { untested();
	cmd.get_line("verilog-module>");

	if (cmd >> "endmodule ") { untested();
	  break;
	}else{ untested();
	}
      }
      delete new_module;
    //  cmd.warn(bDANGER, e.message());
    }
  }
} p2;
DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "module|macromodule", &p2);
/*--------------------------------------------------------------------------*/
class CMD_VERILOG : public CMD {
public:
  void do_it(CS&, CARD_LIST* Scope)override {
    command("options lang=verilog", Scope);
  }
} p8;
DISPATCHER<CMD>::INSTALL d8(&command_dispatcher, "verilog", &p8);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
