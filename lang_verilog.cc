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
  MODEL_CARD*	parse_paramset(CS&, MODEL_CARD*)override;
  BASE_SUBCKT*  parse_module(CS&, BASE_SUBCKT*)override;
  COMPONENT*	parse_instance(CS&, COMPONENT*)override;
  std::string	find_type_in_string(CS&)override;
private: // local
  void skip_attributes(CS& cmd);
  std::string  parse_attributes(CS& cmd);
  void store_attributes(std::string attrib_string, tag_t x);
  void parse_attributes(CS& cmd, tag_t x);
//  void parse_type(CS& cmd, CARD* x);
//  void parse_args_paramset(CS& cmd, MODEL_CARD* x);
//  void parse_args_instance(CS& cmd, CARD* x); 
//  void parse_label(CS& cmd, CARD* x);
  void parse_ports(CS& cmd, COMPONENT* x, bool all_new);

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
{ untested();
  while (cmd >> "(*") { untested();
    cmd.skipto1('*') && (cmd >> "*)");
  }
}
/*--------------------------------------------------------------------------*/
std::string LANG_VERILOG::parse_attributes(CS& cmd)
{ untested();
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
{ untested();
  assert(x);
  if(attrib_string!=""){ untested();
    set_attributes(x).add_to(attrib_string, x);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_attributes(CS& cmd, tag_t x)
{ untested();
  assert(x);
  store_attributes(parse_attributes(cmd), x);
}
/*--------------------------------------------------------------------------*/
static void parse_type(CS& cmd, CARD* x)
{
  assert(x);
  std::string new_type;
  cmd >> new_type;
  x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
static void parse_args_paramset(CS& cmd, MODEL_CARD* x)
{ untested();
  assert(x);

  while (cmd >> '.') { untested();
    size_t here = cmd.cursor();
    std::string name, value;
    try{ untested();
      cmd >> name >> '=' >> value >> ';';
      x->set_param_by_name(name, value);
    }catch (Exception_No_Match&) { untested();
      cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + name + " ignored");
    }
  }
}
/*--------------------------------------------------------------------------*/
static void parse_args_instance(CS& cmd, CARD* x)
{
  assert(x);

  if (cmd >> "#(") {
    if (cmd.match1('.')) {
      // by name
      while (cmd >> '.') {
	size_t here = cmd.cursor();
	std::string name  = cmd.ctos("(", "", "");
	std::string value = cmd.ctos(",)", "(", ")");
	cmd >> ',';
	try{
	  x->set_param_by_name(name, value);
	}catch (Exception_No_Match&) {
	  cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + name + " ignored");
	}
      }
    }else{ untested();
      // by order
      int index = 1;
      while (cmd.is_alnum() || cmd.match1("+-.")) { untested();
	size_t here = cmd.cursor();
	try{ untested();
	  std::string value = cmd.ctos(",)", "", "");
	  x->set_param_by_index(x->param_count() - index++, value, 0/*offset*/);
	}catch (Exception_Too_Many& e) {untested();
	  cmd.warn(bDANGER, here, e.message());
	}
      }
    }
    cmd >> ')';
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
  }else if(name[0] == '\\') {
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

  if(cmd.is_digit()) {
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
	}else if (id[i] == '$') {
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
{ untested();
  assert(x);

  if (cmd >> '(') { untested();
    std::string attribs = parse_attributes(cmd);
    size_t here = cmd.cursor();
    
    if (cmd.match1('.')) { untested();
      // by name
      while (cmd >> '.') { untested();
	std::string Name = get_identifier(cmd, "(");
	int paren = cmd.skip1b('(');
	std::string value = get_identifier(cmd, ")");
	if (!paren){ untested();
	  //?
	}else if( cmd.skip1b(')')) { untested();
	}else{untested();
	  cmd.warn(bDANGER, here, x->long_label() + ": need ')'");
	}
	cmd >> ',';
	try{ untested();
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
      for (int Index = 0;  Index < x->min_nodes();  ++Index) { untested();
	//BUG// This may be a bad idea.
	//BUG// It's definitely wrong with all_new.
	//BUG// What should we do with unconnected ports?
	if (!(x->node_is_connected(Index))) {untested();
	  cmd.warn(bDANGER, x->port_name(Index) + ": port unconnected, grounding");
	  x->set_port_to_ground(Index);
	}else{ untested();
	}
      }
    }else{ untested();
      // by order
      int Index;
      for (Index = 0;  cmd.is_alnum() || cmd.peek() == '\\';  ++Index) { untested();
	try{ untested();
	  std::string value = get_identifier(cmd, ",)");
	  cmd >> ',';
	  x->set_port_by_index(Index, value);
	  store_attributes(attribs,  x->port_id_tag(Index));
	  if (all_new) { untested();
	    if (x->node_is_grounded(Index)) { untested();
	      cmd.warn(bDANGER, here, "node 0 not allowed here");
	      --Index;
	    }else if (x->subckt() && x->subckt()->nodes()->how_many() != Index+1) { untested();
	      cmd.warn(bDANGER, here, "duplicate port name, skipping");
	      --Index;
	    }else{ untested();
	    }
	  }else{ untested();
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
      }else{ untested();
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

  // if cmd is a `preprocessor directive, then x->owner will be useful...

  cmd.reset();
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
}
/*--------------------------------------------------------------------------*/
/* "paramset" <my_name> <base_name> ";"
 *    <paramset_item_declaration>*
 *    <paramset_statement>*
 *  "endparamset"
 */
//BUG// no paramset_item_declaration, falls back to spice mode

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
/*--------------------------------------------------------------------------*/
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
  cmd.reset();
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
      cmd.reset();
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "//") {
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
  size_t here = cmd.cursor();
  std::string type;
  if ((cmd >> "//")) {
    assert(here == 0);
    type = "dev_comment";
  }else{
    cmd >> type;
  }
  cmd.reset(here);
  trace2("LANG_VERILOG::find_type_in_string", cmd.fullstring(), type);
  return type;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
  cmd.get_line("gnucap-verilog>");
  new__instance(cmd, NULL, Scope);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const MODEL_CARD* x)
{
  assert(x);
  if (x->use_obsolete_callback_print()) {
    x->print_args_obsolete_callback(o, this);  //BUG//callback//
  }else{
    for (int ii = x->param_count() - 1;  ii >= 0;  --ii) {
      if (x->param_is_printable(ii)) {
	std::string arg = " ." + x->param_name(ii) + "=" + x->param_value(ii) + ";";
	o << arg;
      }else{ untested();
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << " #(";
  if (x->use_obsolete_callback_print()) { untested();
    arg_count = 0;
    x->print_args_obsolete_callback(o, this);  //BUG//callback//
    arg_count = INACTIVE;
  }else{
    std::string sep = ".";
    for (int ii = x->param_count() - 1;  ii >= 0;  --ii) {
      if (x->param_is_printable(ii)) {
	o << sep << x->param_name(ii) << "(" << x->param_value(ii) << ")";
	sep = ",.";
      }else{
      }
    }
  }
  o << ") ";
}
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
  for (int ii = 0;  x->current_port_exists(ii);  ++ii) {untested();
    o << sep << x->current_port_name(ii) << '(' << x->current_port_value(ii) << ')';
    sep = ",.";
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
  for (int ii = 0;  x->current_port_exists(ii);  ++ii) {untested();
    o << sep << x->current_port_value(ii);
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

  o << "module " <<  x->short_label();
  print_ports_short(o, x);
  o << ";\n";
  auto p=x->subckt()->params();
  for(auto i: *p){
    o << "parameter " << i.first << "={" << i.second << "};\n";
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
{untested();
  assert(x);
  o << x->s() << '\n';
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CMD_PARAMSET : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)override { untested();
    // already got "paramset"
    std::string my_name, base_name;
    cmd >> my_name;
    size_t here = cmd.cursor();
    cmd >> base_name;

    //const MODEL_CARD* p = model_dispatcher[base_name];
    const CARD* p = lang_verilog.find_proto(base_name, NULL);
    if (p) { untested();
      MODEL_CARD* new_card = dynamic_cast<MODEL_CARD*>(p->clone());
      if (new_card) { untested();
	assert(!new_card->owner());
	lang_verilog.parse_paramset(cmd, new_card);
	Scope->push_back(new_card);
      }else{untested();
	cmd.warn(bDANGER, here, "paramset: base has incorrect type");
      }
    }else{untested();
      cmd.warn(bDANGER, here, "paramset: no match");
    }
  }
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "paramset", &p1);
/*--------------------------------------------------------------------------*/
class CMD_MODULE : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    BASE_SUBCKT* new_module = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("module"));
    assert(new_module);
    assert(!new_module->owner());
    assert(new_module->subckt());
    assert(new_module->subckt()->is_empty());
    assert(!new_module->is_device());
    try {
      lang_verilog.parse_module(cmd, new_module);
      Scope->push_back(new_module);
    }catch(Exception const& e) {
      cmd.warn(bDANGER, e.message());
      for (;;) {
	cmd.get_line("verilog-module>");

	if (cmd >> "endmodule ") {
	  break;
	}else{
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
