/*                -*- C++ -*-
 * Copyright (C) 2018 Felix Salfelder
 * Author: Felix Salfelder <felix@salfelder.org>
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
 * non-sophisticated verilog preprocessor stub
 * `define, `ifdef, `ifndef, constants only. enough for include guards.
 */
#include <c_comand.h>
#include <ap.h>
#include <globals.h>
#include <u_lang.h>
#include <stack>
#include <e_subckt.h>
#include <d_coment.h>

extern std::stack<CARD*> owner_hack;
namespace {
/*--------------------------------------------------------------------------*/
class CMD_VPP_DEFINE : public CMD {
public:
	CMD_VPP_DEFINE() {
		_defs["GNUCAP"];
	}
public:
	void do_it(CS& cmd, CARD_LIST*)override { untested();
		std::string what;
		cmd >> what;
		trace2("def", cmd.fullstring(), what);
		if(defined(what)){ untested();
			error(bPICKY, "already defined: %s\n", what.c_str());
		}else{ untested();
			_defs[what];
		}
	}
	bool defined(std::string const& what) const{
		bool v=_defs.find(what)!=_defs.end();
		trace2("defined", what, v);
		return v;
	}
	void undef(std::string const& what){ untested();
		auto v=_defs.find(what);
		if(v!=_defs.end()){ untested();
			_defs.erase(v);
		}else{ untested();
			error(bPICKY, "not defined: %s\n", what.c_str());
		}
	}
private:
	std::map<std::string, std::string> _defs;
} p_define;
DISPATCHER<CMD>::INSTALL d_define(&command_dispatcher, "`define", &p_define);
/*--------------------------------------------------------------------------*/
class CMD_VPP_IF : public CMD {
public:
	enum status_t {
		c_disabled=0,
		c_active=1,
		c_past_else=2,
		c_done=4
	};
public:
	CMD_VPP_IF() : CMD(){
	}
	~CMD_VPP_IF() {
		assert(!_nest.size());
	}
public:
	void do_it(CS& cmd, CARD_LIST* Scope)override {
		trace1("if", Scope);
		if(cmd.umatch("0 ")){
			if0(cmd, Scope);
		}else if(cmd.umatch("1 ")){
			if1(cmd, Scope);
		}else{ untested();
			incomplete();
		}
	}
public:
	void if0(CS& cmd, CARD_LIST* Scope){
		status_t t = c_disabled;
		if(!_nest.size()){
		}else if(c_active & _nest.top()){ untested();
		}else{ untested();
			t = status_t(t | c_done);
		}
		_nest.push(t);
		cond_block(cmd, Scope, true);
	}
	void if1(CS& cmd, CARD_LIST* Scope){
		status_t t = c_active;
		if(_nest.size()){
			t = status_t(c_active & _nest.top());
		}else{
		}
		_nest.push(t);
		set(c_done);
		cond_block(cmd, Scope, true);
	}
private:
	// allow_else: we haven't seen an "else" yet.
	void cond_block(CS& cmd, CARD_LIST* scope, bool allow_else){

		BASE_SUBCKT* o=NULL;
		if(scope!=&CARD_LIST::card_list){ untested();
			assert(!owner_hack.empty());
			trace1("not toplevel, need owner", scope->parent());
			o = prechecked_cast<BASE_SUBCKT*>(owner_hack.top());
			assert(o);
//			o->scope()->params()->set_try_again(scope->params());
		}else{
			trace0("no owner");
		}

		assert(_nest.size());
		size_t depth = _nest.size()-1;
		trace3("cond push ", cmd.fullstring(), allow_else, _nest.size());

		for (;;) {
			assert(OPT::language);
			cmd.get_line("gnucap->");
			trace3("got...", cmd.fullstring(), _nest.size(), depth);

			if(_nest.top() & c_active){
				OPT::language->new__instance(cmd, o, scope);
			}else if(cmd.umatch("`elsif ")){ untested();
				cmd.reset();
				OPT::language->new__instance(cmd, o, scope);
			}else if(cmd.umatch("`ifdef|`ifndef ")){ untested();
				cmd.reset();
				OPT::language->new__instance(cmd, o, scope);
			}else if(cmd.umatch("`if ")){
				cmd.reset();
				OPT::language->new__instance(cmd, o, scope);
			}else if(cmd.umatch("`else ")){ untested();
				cmd.reset();
				OPT::language->new__instance(cmd, o, scope);
			}else if(cmd.umatch("`endif ")){
				cmd.reset();
				OPT::language->new__instance(cmd, o, scope);
			}else{
				trace3("ignoring nested...", cmd.fullstring(), o, cmd.peek());
				CARD* x=device_dispatcher.clone("dev_comment");
				DEV_COMMENT* c=prechecked_cast<DEV_COMMENT*>(x);
				assert(c);
				OPT::language->parse_comment(cmd, c);
				scope->push_back(x);
			}

			trace3("parsed...", cmd.fullstring(), _nest.size(), depth);

			if(_nest.size()==depth){
				// past endif
				break;
			}else{
			}
		}
	}
public:
	bool is(status_t x){
		assert(_nest.size());
		return _nest.top() & x;
	}
	void set(status_t x){
		assert(_nest.size());
		_nest.top() = status_t( _nest.top() | x );
	}
	void unset(status_t x){
		assert(_nest.size());
		_nest.top() = status_t( _nest.top() & ~x );
	}
private:
	std::map<std::string, std::string> _defs;
public:
	std::stack<status_t> _nest;
} p_if;
DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "`if", &p_if);
/*--------------------------------------------------------------------------*/
class CMD_VPP_ELSE : public CMD {
public:
	CMD_VPP_ELSE() : CMD(){
	}
public:
	void do_it(CS& cmd, CARD_LIST*)override {
		trace1("`else", p_if._nest.size());
		if(!p_if._nest.size()){ untested();
			cmd.warn(bDANGER, 0, "misplaced else");
		}else if(p_if.is(CMD_VPP_IF::c_past_else)){ untested();
			cmd.warn(bDANGER, 0, "double else");
		}else if(p_if.is(CMD_VPP_IF::c_done)){
			p_if.set( CMD_VPP_IF::c_past_else );
			p_if.unset( CMD_VPP_IF::c_active );
		}else if(p_if.is(CMD_VPP_IF::c_active)){ untested();
			unreachable(); // implies "done".
		}else{ untested();
			trace1("top is else branch", p_if._nest.size());
			// no further else please.
			// p_if._nest.pop();
			p_if.set(CMD_VPP_IF::c_past_else );
			p_if.set(CMD_VPP_IF::c_done );
			p_if.set(CMD_VPP_IF::c_active);
		}
	}
} p_else;
DISPATCHER<CMD>::INSTALL d_else(&command_dispatcher, "`else", &p_else);
/*--------------------------------------------------------------------------*/
class CMD_VPP_ENDIF : public CMD {
public:
	void do_it(CS& cmd, CARD_LIST*)override {
		trace2("endif", cmd.fullstring(), p_if._nest.size());
		if(!p_if._nest.size()){ untested();
			cmd.warn(bDANGER, 0, "unmatched endif");
		}else if(p_if._nest.top() & p_if.c_active){ untested();
			p_if._nest.pop();
		}else{
			p_if._nest.pop();
		}
	}
} p_endif;
DISPATCHER<CMD>::INSTALL d_endif(&command_dispatcher, "`endif", &p_endif);
/*--------------------------------------------------------------------------*/
class CMD_VPP_UNDEF : public CMD {
public:
	void do_it(CS& cmd, CARD_LIST*)override { untested();
		std::string what;
		cmd >> what;
		p_define.undef(what);
	}
} p_undef;
DISPATCHER<CMD>::INSTALL d_undef(&command_dispatcher, "`undef", &p_undef);
/*--------------------------------------------------------------------------*/
class CMD_VPP_IFDEF : public CMD {
public:
	CMD_VPP_IFDEF() : CMD(){
	}
public:
	void do_it(CS& cmd, CARD_LIST* Scope)override {
		std::string what;
		cmd >> what;
		if (p_define.defined(what)){ untested();
			p_if.if1(cmd, Scope);
		}else{
			p_if.if0(cmd, Scope);
		}
	}
} p_ifdef;
DISPATCHER<CMD>::INSTALL d_ifdef(&command_dispatcher, "`ifdef", &p_ifdef);
/*--------------------------------------------------------------------------*/
class CMD_VPP_ELSIF : public CMD {
public:
	CMD_VPP_ELSIF() : CMD(){
	}
public:
	void do_it(CS& cmd, CARD_LIST*)override { untested();
		if (!p_if._nest.size()){ untested();
			cmd.warn(bDANGER, 0, "misplaced `elsif");
		}else if( p_if.is(CMD_VPP_IF::c_past_else ) ){ untested();
			cmd.warn(bDANGER, 0, "`elsif after else?");
			p_if.unset(CMD_VPP_IF::c_active);
		}else if(p_if.is(CMD_VPP_IF::c_done)){ untested();
			// no need to evaluate, TODO: parse
			p_if.unset(CMD_VPP_IF::c_active);
		}else if(!(p_if.is(CMD_VPP_IF::c_active))){ untested();

			std::string w;
			cmd >> w;
			if(p_define.defined(w)){ untested();
				p_if.set(CMD_VPP_IF::c_active);
				p_if.set(CMD_VPP_IF::c_done);
			}else{ untested();
			}

		}else{ untested();
			unreachable();
			// active implies done
		}
	}
} p_elif;
DISPATCHER<CMD>::INSTALL d3(&command_dispatcher, "`elsif", &p_elif);
/*--------------------------------------------------------------------------*/
class CMD_VPP_IFNDEF : public CMD {
public:
	void do_it(CS& cmd, CARD_LIST* Scope)override { untested();
		std::string what;
		cmd >> what;
		if (p_define.defined(what)){ untested();
			p_if.if0(cmd, Scope);
		}else{ untested();
			p_if.if1(cmd, Scope);
		}
	}
} p_ifndef;
DISPATCHER<CMD>::INSTALL d_ifndef(&command_dispatcher, "`ifndef", &p_ifndef);
/*--------------------------------------------------------------------------*/
}

// vim:noet
