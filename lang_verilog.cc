/*$Id: lang_verilog.cc $ -*- C++ -*-
 * Copyright (C) 2007 Albert Davis
 *               2022-24 Felix Salfelder
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
#include <u_nodemap.h>
#include <globals.h>
#include <c_comand.h>
#include <d_dot.h>
#include <d_coment.h>
#include <e_subckt.h>
#include <e_model.h>
#include <u_lang.h>
/*--------------------------------------------------------------------------*/
static const std::string IS_VALID = "__is_valid";
static const std::string HOW_VALID = "__how_valid";
static bool instanciate_unused = false;
static int print_nest;
static std::string toplevel_stub = "__stub";
static std::string subdevice_stub = "__stub";
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class LANG_VERILOG : public LANGUAGE {
public:
  typedef enum MODE {mDEFAULT, mPARAMSET} mode;
private:
  mode _mode;
  mutable int arg_count;
  enum {INACTIVE = -1};
public:
  LANG_VERILOG() : arg_count(INACTIVE) {}
  ~LANG_VERILOG() {}
  std::string name()const override {return "verilog";}
  bool case_insensitive()const override {return false;}
  UNITS units()const override {return uSI;}

public: // override virtual, used by callback
  std::string arg_front()const override {
    switch (_mode) {
    case mPARAMSET:untested(); return " .";			    break;
    case mDEFAULT:  return (arg_count++ > 0) ? ", ." : "."; break;
    }
    unreachable();
    return "";
  }
  std::string arg_mid()const override {
    switch (_mode) {
    case mPARAMSET:untested(); return "="; break;
    case mDEFAULT:  return "("; break;
    }
    unreachable();
    return "";
  }
  std::string arg_back()const override {
    switch (_mode) {
    case mPARAMSET:untested(); return ";"; break;
    case mDEFAULT:  return ")"; break;
    }
    unreachable();
    return "";
  }

public: // override virtual, called by commands
  void		parse_top_item(CS&, CARD_LIST*)override;
  DEV_COMMENT*	parse_comment(CS&, DEV_COMMENT*)override;
  DEV_DOT*	parse_command(CS&, DEV_DOT*)override;
  MODEL_CARD*	parse_paramset(CS&, MODEL_CARD*)override;
  COMPONENT*	parse_paramset_(CS&, BASE_SUBCKT*);
  CARD*		obsolete_parse_modelcard(CS&, MODEL_CARD*);
  BASE_SUBCKT*  parse_module(CS&, BASE_SUBCKT*)override;
  COMPONENT*	parse_instance(CS&, COMPONENT*)override;
  std::string	find_type_in_string(CS&)override;
  void move_attributes(tag_t from, tag_t to);
private: // local
  void skip_attributes(CS& cmd);
  std::string  parse_attributes(CS& cmd);
  void store_attributes(std::string attrib_string, tag_t x);
  CS& parse_attributes(CS& cmd, tag_t x);
  void parse_type(CS& cmd, CARD* x);
  void parse_args_paramset(CS& cmd, /* MODEL_*/ CARD* x);
  void parse_args_instance(CS& cmd, CARD* x);
  void parse_label(CS& cmd, CARD* x);
  void parse_ports(CS& cmd, COMPONENT* x, bool all_new);

private: // override virtual, called by print_item
  void print_paramset(OMSTREAM&, const MODEL_CARD*)override;
  void print_module(OMSTREAM&, const BASE_SUBCKT*)override;
  void print_instance(OMSTREAM&, const COMPONENT*)override;
  void print_comment(OMSTREAM&, const DEV_COMMENT*)override;
  void print_command(OMSTREAM& o, const DEV_DOT*)override;
private: // local
  void print_paramset_(OMSTREAM&, const MODEL_CARD*);
  void new_instance_(CS& cmd, BASE_SUBCKT* Owner, CARD_LIST* Scope);
  void print_attributes(OMSTREAM&, tag_t)const;
  void print_args(OMSTREAM&, const MODEL_CARD*);
  void print_args(OMSTREAM&, const COMPONENT*);
  template<class T>
  void print_args_paramset(OMSTREAM&, const T*);
  void print_items_sckt(OMSTREAM&, const COMPONENT*);
  void print_type(OMSTREAM& o, const COMPONENT* x);
  void print_label(OMSTREAM& o, const COMPONENT* x);
  void print_ports_long(OMSTREAM& o, const COMPONENT* x);
  void print_ports_short(OMSTREAM& o, const COMPONENT* x);
} lang_verilog;

DISPATCHER<LANGUAGE>::INSTALL
	d(&language_dispatcher, lang_verilog.name(), &lang_verilog);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static std::string parse_identifier(CS& cmd, std::string const& term);
/*--------------------------------------------------------------------------*/
#if 0
void LANG_VERILOG::skip_attributes(CS& cmd)
{ untested();
  while (cmd >> "(*") { untested();
    cmd.skipto1('*') && (cmd >> "*)");
  }
}
#endif
/*--------------------------------------------------------------------------*/
std::string LANG_VERILOG::parse_attributes(CS& cmd)
{
  std::string attrib_string = "";
  std::string comma = "";
  while (cmd >> "(*") {
    attrib_string += comma;
    while(cmd.ns_more() && !(cmd >> "*)")) {
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
  if(attrib_string!=""){
    set_attributes(x).add_to(attrib_string, x);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
CS& LANG_VERILOG::parse_attributes(CS& cmd, tag_t x)
{
  assert(x);
  store_attributes(parse_attributes(cmd), x);

  return cmd;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_type(CS& cmd, CARD* x)
{
  assert(x);
  std::string new_type = parse_identifier(cmd, ",=(){};");
  x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_args_paramset(CS& cmd, CARD* x)
{
  assert(x);

  while (cmd >> '.') {
    size_t here = cmd.cursor();
    std::string Name, value;
    try{
      cmd >> Name;
      if(cmd >> '='){
      }else if(cmd.peek() == '('){
	cmd.warn(bDANGER, "nonstandard syntax" );
      }else{ untested();
      }
      Expression e(cmd);
      std::stringstream s;
      e.dump(s);
      trace2("got expression", cmd.fullstring(), cmd.tail());
      if(cmd >> ';') {
      }else{
	cmd.warn(bDANGER, x->long_label() + ": expecting ';'");
      }
      x->set_param_by_name(Name, s.str());
    }catch (Exception_No_Match&) {untested();
      cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + Name + " ignored");
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::move_attributes(tag_t from, tag_t to)
{
  if(has_attributes(to)){
    erase_attributes(to, to+1);
  }else{
  }
  if(has_attributes(from)){
    set_attributes(to).add_to(attributes(from)->string(tag_t(0)), to);
    erase_attributes(from, from+1);
  }else{
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
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_label(CS& cmd, CARD* x)
{
  assert(x);
  std::string my_name = parse_identifier(cmd, ",=(){};");
  if (cmd) {
    x->set_label(my_name);
  }else{
    x->set_label(x->id_letter() + std::string("_unnamed")); //BUG// not unique
    cmd.warn(bDANGER, "label required");
  }
}
/*--------------------------------------------------------------------------*/
void dump_identifier(OMSTREAM& o, std::string const& name)
{
  bool plain = true;

  if(isalpha(name[0])){
  }else if(name[0] == '$'){ untested();
  }else if(name[0] == '_'){ untested();
  }else{
    plain = false;
  }

  for(size_t i=1; plain && i<name.size(); ++i){
    if(isalnum(name[i])){
    }else if(name[i] == '_'){
    }else{
      plain = false;
    }
  }

  if(plain){
    o << name;
  }else{
    o << '\\';
    for(size_t i=0; i<name.size(); ++i){
      if(name[i] == '\\'){
	o << '\\';
      }else{
      }
      o << name[i];
    }
    o << ' ';
  }
}
/*--------------------------------------------------------------------------*/
// get identifier and turn into internal representation
// "\1 " -> "1"         -- so it also works with spice
// "\a " -> "a"         -- identical, use simple form
// "\$ " -> "$"         -- not sure.
// "\a* " -> "a*"       -- store unprotected
// "\\\xyz " -> "\xyz"  -- remove additional escapes
// "\foo\bar"           -- incomplete
static std::string parse_identifier(CS& cmd, std::string const& term)
{
  cmd.skipbl();
  std::string id;

  if(cmd.is_digit()) {
    cmd.warn(bDANGER, "invalid identifier");
  }else{
  }

  bool esc = cmd.skip1('\\');

  while(esc && cmd.more()) {
    if(cmd.skip1('\\')){
      if(cmd.skip1('\\')){
	id += "\\";
      }else{ untested();
	cmd.warn(bDANGER, "invalid escaped char");
      }
    }else{
    }
    id += cmd.get_to(" \t\f\\");

    if(cmd.skip1(" \t\f")){
      break;
    }else{
    }
  }

  if(!esc) {
    id = cmd.ctos(term, "", "");
  }else{
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
	std::string Name = parse_identifier(cmd, "(");
	int paren = cmd.skip1b('(');
	std::string value = parse_identifier(cmd, ")");
	if (!paren){untested();
	  //?
	}else if( cmd.skip1b(')')) {
	}else{untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": need ')'");
	}
	cmd >> ',';
	try{
	  int Index = x->set_port_by_name(Name, value);
	  store_attributes(attribs,  x->port_id_tag(Index));
	}catch (Exception_No_Match&) {untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": mismatch " + Name + " ignored");
	}catch (Exception_Clash&) {untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": already set " + Name + ", ignored");
	}
	attribs = parse_attributes(cmd);
	here = cmd.cursor();
      }
      for (int Index = 0;  Index < x->min_nodes();  ++Index) {
	if (!(x->node_is_connected(Index))) {untested();
	  cmd.warn(bDANGER, x->port_name(Index) + ": port unconnected");
	}else{
	}
      }
    }else{
      // by order
      int Index;
      for (Index = 0;  cmd.is_alnum() || cmd.peek() == '\\';  ++Index) {
	try{
	  std::string value = parse_identifier(cmd, ",)");
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
  }else{
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
  x->set(cmd.fullstring());
  CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;
//  cmd.reset();
//  parse_attributes(cmd, tag_t(x));
  if(auto cc=dynamic_cast<CMD*>(x)){
    std::string s;
    cmd >> s;
    cc->cmdproc(cmd);
    return x;
  }else{
    CMD::cmdproc(cmd, scope);
    x->purge();
    delete x;
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
/* "paramset" <my_name> <base_name> ";"
 *    <paramset_item_declaration>*
 *    <paramset_statement>*
 *  "endparamset"
 */
MODEL_CARD* LANG_VERILOG::parse_paramset(CS&, MODEL_CARD* x)
{ untested();
  if(dynamic_cast<BASE_SUBCKT*>(x)) { untested();
    incomplete();
    return NULL;
   // return parse_paramset_(cmd, c);
  }else if(auto m = dynamic_cast<MODEL_CARD*>(x)) { untested();
    //BUG// no paramset_item_declaration, falls back to spice mode
    unreachable();
    return m;
    // return obsolete_parse_modelcard(cmd, m);
  }else{ untested();
    unreachable();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
class CMD_MODULE_PARAM : public CMD {
public:
  explicit CMD_MODULE_PARAM() {}
private:
  explicit CMD_MODULE_PARAM(CMD_MODULE_PARAM const& p) : CMD(p) {}
public:
  CMD_MODULE_PARAM* clone()const override {
    return new CMD_MODULE_PARAM(*this);
  }
public:
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    CARD* Owner = owner();
    PARAM_LIST* pl = Scope->params();
    assert(!Owner || Scope == Owner->subckt());
    std::string what = cmd.last_match();
    if (cmd.is_end()) { untested();
      pl->print(IO::mstdout, OPT::language);
      IO::mstdout << '\n';
    }else{
      std::string tail = cmd.tail();
      parse(cmd, Owner, what[0]);
      lang_verilog.move_attributes(tag_t(&cmd), id_tag());
      set(what + tail);
    }
  }
private:
  void parse(CS& cmd, CARD* Owner, char what)const;
  void parse_def(CS& cmd, PARAM_INSTANCE& par)const;
  bool parse_range(CS& cmd, PARAM_LIST* Scope, std::string Name)const;
} module_param;
/*--------------------------------------------------------------------------*/
class CMD_NET_DECL : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    assert(Scope);
    assert(cmd.last_match().size()>2);
    DEV_DOT* dot = new DEV_DOT();
    dot->set(cmd.last_match() + cmd.tail());
    Scope->push_back(dot);
    assert(Scope->nodes());
    NODE_MAP& nm = *Scope->nodes();

    std::string name;
    while(cmd.more() && !(cmd >> ';')){
      name = parse_identifier(cmd, ",;");
      trace2("net_decl", cmd.last_match(), name);
      NODE* n = nm.new_node(name);
      assert(n);
      node_t& nn = n->n_(0);
      (void) nn;
      switch(cmd.last_match()[2]) {
      case 'p': // inPut
	// nn.set_input();
	break;
      case 't': // ouTput
	// nn.set_output();
	break;
      case 'o': // inOut
	// nn.set_output();
	// nn.set_input();
	break;
      case 'e': // elEctrical
      case 'r': // wiRe
	// not yet.
      default:
	break;
      }
    }
  }
} net_decl;
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
    assert(operator==(p));
    assert(p.operator==(*this));
  }
  ~PARAM_ANY() { delete _value; _value=nullptr;}
  PARA_BASE* clone()const override{ untested();return new PARAM_ANY(*this);}
  PARA_BASE* pclone(void*p)const override{return new(p) PARAM_ANY(*this);}
  bool operator==(const PARA_BASE& v)const override {
    auto p = dynamic_cast<PARAM_ANY const*>(&v);
    // return (p && _v == p->_v  &&  _s == p->_s);
    Base* eq = nullptr;
    bool ret = false;
    if(!p || _s != p->_s) { untested();
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
    }else if(v){ untested();
      _value = v->assign(v);
      assert(_value);
      trace3("don't know", _s, v->val_string(), _value->val_string());
    }else{ itested();
      // fall back
      _value = new Float(NOT_INPUT);
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
    }else{
    }
    return _value;
  }
  bool has_good_value()const override { untested();unreachable(); return false;}
  Base const* e_val_(const Base* def, const PARAM_LIST* s, int)const override {
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
void CMD_MODULE_PARAM::parse_def(CS& cmd, PARAM_INSTANCE& par) const
{
   // BUG // need to tokenize right here. strings may contain separators etc.
  Expression e(cmd);
  std::stringstream s;
  e.dump(s);

  trace2("got default expression", s.str(), cmd.tail());
  par = s.str();
}
/*--------------------------------------------------------------------------*/
void CMD_MODULE_PARAM::parse(CS& cmd, CARD* Owner, char what) const
{
  int inc_valid = 0;
  CARD_LIST* Scope;
  if(Owner) {
    Scope = Owner->subckt();
  }else{ untested();
    Scope = &CARD_LIST::card_list;
  }
  PARAM_LIST* pl = Scope->params();
  assert(pl);
  PARAM_INSTANCE par;
  if(cmd >> "real"){
    par = PARAMETER<vReal>();
  }else if(cmd >> "integer"){
    par = PARAMETER<vInteger>();
  }else if(cmd >> "string"){
    par = PARAMETER<vString>();
  }else{
    // TODO: realtime | time
    par = PARAM_ANY();
  }
  size_t here = cmd.cursor();
  for (;;) {
    if (!cmd.more()){ untested();
      break;
    }else if(cmd.is_alpha()){
    }else if(cmd.match1('_')){ untested();
    }else if(cmd.match1('\\')){
      // escaped identifier
    }else{
      break;
    }
    Name_String Name;
    cmd >> Name;
    trace1("CMD_PARAM::parse", Name);
    par = "";
    if(cmd.skip1('=')) {
      parse_def(cmd, par);
    }else{
    }
    pl->set(Name, par);
    if(!Owner){
    }else if(what == 'p'){
      Owner->set_param_by_name(Name, "");
    }else{
    }
    trace3("parsed", Name, par.string(), cmd.tail());

    if(cmd >> ';') {
      assert(cmd);
      break;
    }else if(cmd >> ',') { itested();
    }else if(!parse_range(cmd, pl, Name)) {
    }else if(what == 'l') {
      ++inc_valid;
      // increment valid if local??
    }else{
    }

    if(cmd >> ';') {
      break;
    }else if(cmd >> ',') { untested();
    }

    if (cmd.stuck(&here)) { untested();
      break;
    }else{ itested();
    }
  }
  if(!cmd){
    cmd.warn(bDANGER, "syntax error");
  }else{
  }

  {
    PARAM_INSTANCE v = pl->deep_lookup(HOW_VALID);
    if(!v.has_hard_value()){
      // v = PARAMETER<vInteger>();
      std::string vlvl = to_string(inc_valid+1);
      pl->set(HOW_VALID, vlvl);
    }else if(inc_valid){
      std::string vlvl = to_string(inc_valid);
      vlvl = v.string() + "+" + vlvl;
      pl->set(HOW_VALID, vlvl);
    }else{
      pl->set(HOW_VALID, "1");
    }
  }
}
/*--------------------------------------------------------------------------*/
bool CMD_MODULE_PARAM::parse_range(CS& cmd, PARAM_LIST* Scope, std::string Name) const
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
      }else if(cmd.skip1('(')){
	lo = "<";
	range=interval;
      }else if(cmd.peek() == '\''){
	range=set;
      }else{ untested();
      }

      if(range == set){
	Expression L;
	cmd >> L;
	Expression LL(L, Scope); // reduce_copy.
	if(LL.size()>2){
	  incomplete();
	  auto t = LL.begin();
	  ++t;
	  what = "!(1";
	  std::string n = (*t)->name();
	  while (++t != LL.end()){
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
	    // incomplete();
	    lb = to_string(ii->value());
	  }else if(auto ff = dynamic_cast<Float const*>(lb_)){
	    // incomplete();
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
	  }else if(auto ff = dynamic_cast<Float const*>(ub_)){
	    ub = to_string(ff->value());
	  }else{ untested();
	    incomplete();
	  }
	}
	trace2("bounds", lb, ub);

	if(cmd.skip1(']')){
	  uo = "<=";
	}else if(cmd.skip1(')')){
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

    if (cmd.stuck(&here)) {
      trace2("c_param stuck", cmd.tail(), range_expr);
      return false;
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
    }else if(v.has_hard_value()){
      range_expr = v.string() + "*" + range_expr;
      trace2("c_param hv", IS_VALID, range_expr);
      pl->set(IS_VALID, range_expr);
    }else{
      v = PARAMETER<vInteger>();
      trace2("c_param", IS_VALID, range_expr);
      pl->set(IS_VALID, HOW_VALID + "*" + range_expr);
    }
    return range_type.size();
  }
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_VERILOG::parse_paramset_(CS& cmd, BASE_SUBCKT* x)
{
  assert(x);

  trace1("parse_paramset_", cmd.fullstring());
  trace1("parse_paramset_", cmd.tail());

 // move_attributes(tag_t(&cmd), tag_t(x));
  cmd >> "paramset ";
  parse_label(cmd, x);
  parse_type(cmd, x);
  cmd >> ';';

  x->subckt()->set_verilog_math();

  for (;;) {
    size_t here = cmd.cursor();
    if (cmd >> "parameter |localparam ") {
      auto p = module_param.clone();
      p->set_owner(x);
      p->do_it(cmd, x->subckt());
      x->subckt()->push_back(p);
    }else if (cmd >> "//") {
      cmd.reset(here);
      // new__instance(cmd, x, x->subckt()); // BUG
      cmd.getline("verilog-paramset>");
    }else if (!cmd.more()) {
      cmd.getline("verilog-paramset>");
    }else{
      break;
    }
  }

  for (;;) {
    parse_args_paramset(cmd, x);
    size_t here = cmd.cursor();
    if (cmd >> "endparamset ") {
      break;
    }else if (cmd >> "// ") {
      cmd.reset(here);
      // new__instance(cmd, x, x->subckt()); // BUG
      cmd.getline("verilog-paramset>");
    }else if (!cmd.more()) {
      cmd.getline("verilog-paramset>");
    }else{ untested();
      cmd.check(bWARNING, "what's this?");
      break;
    }
  }
  trace2("LANG_VERILOG::parse_paramset_ done", x->long_label(), ((CARD*)x)->dev_type());
  return x;
}
/* "module" <name> "(" <ports> ")" ";"
 *    <declarations>
 *    <netlist>
 * "endmodule"
 */
//BUG// strictly one device per line

BASE_SUBCKT* LANG_VERILOG::parse_module(CS& cmd, BASE_SUBCKT* x)
{
  assert(x);

  // header
  move_attributes(tag_t(&cmd), x->id_tag());
  (cmd >> "module |macromodule ");
  parse_label(cmd, x);
  parse_ports(cmd, x, true/*all new*/);
  cmd >> ';';
  x->subckt()->set_verilog_math();

  bool have_instance = false;

  // body
  for (;;) {

    cmd.get_line("verilog-module>");
    while (!parse_attributes(cmd, tag_t(&cmd)).more()){
      cmd.get_line("verilog-module>");
    }
    if(has_attributes(tag_t(&cmd))){
    }else{
    }

    if (cmd >> "endmodule ") {
      break;
    }else if (!have_instance && (cmd >> "parameter |localparam ")) {
      auto p = module_param.clone();
      p->set_owner(x);
      p->do_it(cmd, x->subckt());
      x->subckt()->push_back(p);
    }else if (cmd >> "//") {
      cmd.reset();
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "ground ") {
      cmd.reset();
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "wire |electrical |inout |input |output ") {
      net_decl.do_it(cmd, x->subckt());
    }else if (cmd >> "module |macromodule ") {
      cmd.reset();
      cmd.check(bWARNING, "nonstandard nesting in " + x->long_label() + ".");
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "paramset ") {
      cmd.reset();
      cmd.check(bWARNING, "nonstandard nesting in " + x->long_label() + ".");
      new__instance(cmd, x, x->subckt());
    }else{
      if(has_attributes(tag_t(&cmd))){
      }else{
      }
      trace1("parse_module: instance", cmd.tail());
      have_instance = true;
      CARD* stub = device_dispatcher.clone(subdevice_stub);
      assert(stub);
      auto new_instance = dynamic_cast<BASE_SUBCKT*>(stub);
      assert(new_instance);
      CARD_LIST* Scope = x->subckt();
      trace3("parse_module instance", cmd.tail(), Scope, Scope->nodes());
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
  if(has_attributes(tag_t(&cmd))){
  }else{
  }
  assert (!(cmd >> "(*"));
  move_attributes(tag_t(&cmd), x->id_tag());
  parse_type(cmd, x);
  parse_args_instance(cmd, x);
  parse_label(cmd, x);
  parse_ports(cmd, x, false/*allow dups*/);
  cmd >> ';';
  cmd.check(bWARNING, "what's this?");
//  x->set_owner(nullptr);
  return x;
}
/*--------------------------------------------------------------------------*/
std::string LANG_VERILOG::find_type_in_string(CS& cmd)
{
  size_t here = cmd.cursor();
  assert (!(cmd >> "(*"));

  std::string type;
  if ((cmd >> "//")) {
    //assert(here == 0);
    type = "dev_comment";
  }else{
    cmd >> type;
  }
  cmd.reset(here); // where the type is.
  trace1("ftis", type);
  return type;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::new_instance_(CS& cmd, BASE_SUBCKT* Owner, CARD_LIST* Scope)
{
  if (cmd.is_end()) {itested();
    // nothing
  }else{
    std::string type = find_type_in_string(cmd);
    const CARD* proto = find_proto(type, Owner);
    if (dynamic_cast<DEV_DOT const*>(proto)) {
    }else if (dynamic_cast<DEV_COMMENT const*>(proto)) {
    }else if(toplevel_stub == ""){ untested();
    }else{
      proto = device_dispatcher[toplevel_stub];
      assert(proto);
    }

    if(proto){
      if (CARD* new_instance = proto->clone_instance()) {
       new_instance->set_owner(Owner);
       CARD* x = parse_item(cmd, new_instance);
       if (x) {
         assert(Scope);
         Scope->push_back(x);
       }else{
       }
      }else{ untested();
       cmd.warn(bDANGER, type + ": incomplete prototype");
      }
    }else{
      cmd.warn(bDANGER, type + ": no match");
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
  cmd.getline("gnucap-verilog>");
  while(!parse_attributes(cmd, tag_t(&cmd)).more()) {
    cmd.getline("gnucap-verilog>");
  }
  new_instance_(cmd, NULL, Scope);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_attributes(OMSTREAM& o, tag_t x) const
{
  assert(x);

  if (has_attributes(x)) {
    std::string s = attributes(x)->string(x);
    if(s.size()) {
      o << "(* " << s << " *) ";
    }else{ untested();
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const MODEL_CARD* x)
{
  print_args_paramset(o, x);
}
/*--------------------------------------------------------------------------*/
template<class T>
void LANG_VERILOG::print_args_paramset(OMSTREAM& o, const T* x)
{
  assert(x);
  if (x->use_obsolete_callback_print()) {untested();
    unreachable();
    // x->print_args_obsolete_callback(o, this);  //BUG//callback//
  }else{
#if 0
    if(0 && x->subckt()){ untested();
      for(auto p : *x->subckt()->params()){ untested();
	std::string const& f = p.first;
	if(!p.second.has_hard_value()){ untested();
	  o << "  parameter " << p.first << ";\n";
	}else if(f.size() < 2 || f[0] != '_' || f[1] != '.'){ untested();
	  o << "  parameter " << p.first << " = " << p.second.string() << ";\n";
	}else{ untested();
	  // hack: hide internal parameter.
	  // move to instance?
	}
      }
    }else{ untested();
    	//  DEV_DOT for now.
    }
#endif


    { // slow? use common->_params..?
      for (int ii = 0; ii < x->param_count(); ++ii) {
	if (x->param_is_printable(ii)) {
	  o << " ." << x->param_name(ii) << '=' << x->param_value(ii) << ";";
//	  o << arg;
	}else{
	}
      }
//      o << "\\\n";
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
    for (int ii = 0; ii < x->param_count(); ++ii) {
      if (x->param_is_printable(ii)) {
	o << sep;
	print_attributes(o, x->param_id_tag(ii));
	std::string pn = x->param_name(ii);
	if(pn==""){
	  o << x->param_value(ii);
	  sep = ", ";
	}else{
	  o << "." << pn << "(" << x->param_value(ii) << ")";
	  sep = ",";
	}
      }else{
      }
    }
  }
  o << ") ";
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_type(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  dump_identifier(o, x->dev_type());
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_label(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  dump_identifier(o, x->short_label());
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_ports_long(OMSTREAM& o, const COMPONENT* x)
{
  // print in long form ...    .name(value)
  assert(x);

  o << " (";
  std::string sep = "";
  for (int ii = 0;  x->port_exists(ii);  ++ii) {
    if(x->node_is_connected(ii)){
      o << sep;
      print_attributes(o, x->port_id_tag(ii));
      if(!x->port_name(ii).size()){
	dump_identifier(o, x->port_value(ii));
	sep = ", ";
      }else{
	o << '.';
	dump_identifier(o, x->port_name(ii));
	o << '(';
	dump_identifier(o, x->port_value(ii));
	o << ')';
	sep = ',';
      }
    }else{
    }
  }
  o << ')';
}
/*--------------------------------------------------------------------------*/
#if 0
void LANG_VERILOG::print_ports_short(OMSTREAM& o, const COMPONENT* x)
{ untested();
  // print in short form ...   value only
  assert(x);

  o << " (";
  std::string sep = "";
  for (int ii = 0;  x->port_exists(ii);  ++ii) { untested();
    print_attributes(o, x->port_id_tag(ii));
    o << sep << x->port_value(ii);
    sep = ",";
  }
  for (int ii = 0;  x->current_port_exists(ii);  ++ii) {untested();
    o << sep << x->current_port_value(ii);
    sep = ",";
  }
  o << ")";
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_items_sckt(OMSTREAM& o, const COMPONENT* x)
{
  assert(dynamic_cast<BASE_SUBCKT const*>(x));
  ++print_nest;
  for (CARD_LIST::const_iterator ci = x->subckt()->begin(); ci != x->subckt()->end(); ++ci) {
//    o << std::string(print_nest*2, ' ');
    print_item(o, *ci);
  }
  --print_nest;
}
/*--------------------------------------------------------------------------*/
class PARAMSET_MODEL : public MODEL_CARD {
  COMPONENT* _p{NULL};
protected:
  explicit PARAMSET_MODEL() : MODEL_CARD(NULL) {untested();}
public:
  explicit PARAMSET_MODEL(COMPONENT* c) : MODEL_CARD(c) {
    _p = c;
    assert(c);
    // set_label("paramset");
    set_label(c->short_label());
  }

  PARAMSET_MODEL* clone()const override {
    assert(component_proto());
    CARD* c = component_proto()->clone();
    auto* p = prechecked_cast<COMPONENT*>(c);
    assert(p);

    return new PARAMSET_MODEL(p);
  }
  CARD* clone_instance()const override {
    return component_proto()->clone_instance();
  }

  int param_count()const override {
    assert(component_proto());
    return component_proto()->param_count();
  }
  std::string param_name(int i)const override {
    return component_proto()->param_name(i);
  }
  std::string param_name(int i, int j)const override { untested();
    return component_proto()->param_name(i, j);
  }
  void precalc_first()override {
    return _p->precalc_first();
  }
private:
  std::string dev_type()const override { return component_proto()->dev_type(); }
};
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_paramset(OMSTREAM& o, const MODEL_CARD* x)
{
  if(auto m = dynamic_cast<MODEL_SUBCKT const*>(x)){
    auto s = prechecked_cast<BASE_SUBCKT const*>(m->component_proto());
    assert(s);
    print_module(o, s);
  }else{
    print_paramset_(o, x);
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_paramset_(OMSTREAM& o, const MODEL_CARD* x)
{
  if(dynamic_cast<PARAMSET_MODEL const*>(x)) { // } ->short_label() == "paramset") { untested();
    COMPONENT const* bs = prechecked_cast<COMPONENT const*>(x->component_proto());
    print_attributes(o, x->id_tag());
    o << "paramset " << bs->short_label() << ' ' << x->dev_type() << ";\n";
    print_items_sckt(o, bs);
    print_args_paramset(o, bs);
    o << "\nendparamset\n\n";
  }else{
    // spice fallback
    _mode = mPARAMSET;
    o << "paramset " << x->short_label() << ' ' << x->dev_type() << ";\n";
    print_args(o, x);
    o << "\n"
      "endparamset\n\n";
    _mode = mDEFAULT;
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_module(OMSTREAM& o, const BASE_SUBCKT* x)
{
  if(((CARD const*)x)->dev_type()!=""){ untested();
    unreachable();
    // tmp hack. module type is the label, so dev_type is blank.
   // return print_paramset(o, x);
  }else{
  }
  assert(x);
  assert(x->subckt());

  print_attributes(o, x->id_tag());
  o << "module " << x->short_label();
  print_ports_long(o, x);
  o << ";\n";
//  auto p = x->subckt()->params();
//  for(auto i: *p){ untested();
//    o << "  parameter " << i.first << " = " << i.second << ";\n";
//  }
  print_items_sckt(o, x);
  o << std::string(print_nest*2, ' ');
  o << "endmodule // " << x->short_label() << "\n\n";
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_instance(OMSTREAM& o, const COMPONENT* x)
{
  print_attributes(o, x->id_tag());
  if(x->is_device()){
    print_type(o, x);
    print_args(o, x);
    print_label(o, x);
    print_ports_long(o, x);
    o << ";\n";
  }else{ untested();
    incomplete();
   // _mode = mPARAMSET;
   // print_paramset(o, x);
   // _mode = mDEFAULT;
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_comment(OMSTREAM& o, const DEV_COMMENT* x)
{
  assert(x);
  if ((x->comment().compare(0, 2, "//")) != 0) {itested();
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
class CMD_PARAMSET : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope) override {
    if(Scope == &CARD_LIST::card_list){
    }else{
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

    if(!proto){
      // 6.4 Paramsets: second identifier will be [..] module".
      //                "will", maybe at a later stage?
      cmd.warn(bPICKY, here, "unknown proto. will retry later.");
    }else{
    }

    paramset = device_dispatcher.clone("paramset");
    paramset->set_owner(nullptr); // m?
    auto dev = prechecked_cast<BASE_SUBCKT*>(paramset);
    lang_verilog.move_attributes(tag_t(&cmd), dev->id_tag());
    assert(dev);
    cmd.reset(here);
    lang_verilog.parse_paramset_(cmd, dev);
    auto m = new PARAMSET_MODEL(dev);
    lang_verilog.move_attributes(dev->id_tag(), m->id_tag());
    m->set_owner(nullptr);
    Scope->push_back(m);

  }
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "paramset", &p1);
/*--------------------------------------------------------------------------*/
class CMD_MODULE : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    class MODEL_SUBCKT_ : public MODEL_SUBCKT {
    public:
      explicit MODEL_SUBCKT_(COMPONENT* c) : MODEL_SUBCKT(c) {}
      void precalc_first()override {
	if(_proto){
	  return _proto->precalc_first();
	}else{ untested();
	}
      }
    };
    BASE_SUBCKT* new_module = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("module"));
    assert(new_module);
    // assert(!new_module->owner());
    new_module->set_owner(nullptr);
    assert(new_module->subckt());
    assert(new_module->subckt()->is_empty());
    try {
      lang_verilog.parse_module(cmd, new_module);
      auto p = new MODEL_SUBCKT_(new_module);
      p->set_owner(owner());
      p->set_label(new_module->short_label());
      Scope->push_back(p);
    }catch(Exception const& e) {
      cmd.warn(bDANGER, e.message());
      for (;;) {
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
