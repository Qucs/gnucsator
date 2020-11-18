/* Copyright (C) 2012 Fabian Vallon, Felix Salfelder
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

#include <u_status.h>
#include <c_comand.h>
#include <d_dot.h>
#include <d_coment.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#include <globals.h>

// header hack
// #include "d_logic.h"

#include "e_model.h"
#include "bm.h"
#include "u_lang.h"
#include "l_qucs.h"

#include <algorithm>
#include <cctype>
#include <string>

#ifndef GCUF_CONST
# define GCUF_CONST
#endif

#define QUCS_ANTI_COMMENT "#>"

using std::stringstream;
using std::string;

/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static /*const*/  std::string ground_name="0";
/*--------------------------------------------------------------------------*/
struct subckt_alias{
	subckt_alias(){
		CARD* x=device_dispatcher["subckt"];
		assert(x);
		_d=new DISPATCHER<CARD>::INSTALL(&device_dispatcher, "Sub", x);
	}
	~subckt_alias(){
		delete _d;
	}
	DISPATCHER<CARD>::INSTALL* _d;
}a;
/*--------------------------------------------------------------------------*/
class LANG_QUCSATOR : public LANGUAGE {
	public:
		enum EOB {NO_EXIT_ON_BLANK, EXIT_ON_BLANK};

	public: // override virtual, used by callback
		std::string arg_front()const {return " ";}
		std::string arg_mid()const {return "=";}
		std::string arg_back()const {return "";}

	public: // override virtual, called by commands
		DEV_COMMENT*	parse_comment(CS&, DEV_COMMENT*);
		DEV_DOT*	parse_command(CS&, DEV_DOT*);
		MODEL_CARD*	parse_paramset(CS&, MODEL_CARD*);
		BASE_SUBCKT* parse_module(CS&, BASE_SUBCKT*);
		COMPONENT*	parse_instance(CS&, COMPONENT*);
		std::string	find_type_in_string(CS&) GCUF_CONST;
	public: // "local?", called by own commands
		void parse_module_body(CS&, BASE_SUBCKT*, CARD_LIST*, const std::string&,
				EOB, const std::string&);
	private: // local
		void parse_type(CS&, CARD*);
		void parse_args(CS&, CARD*);
		void parse_label(CS&, CARD*);
		void parse_ports(CS&, COMPONENT*, int minnodes, int start, int num_nodes, bool all_new);
	private: // compatibility hacks
		void parse_element_using_obsolete_callback(CS&, COMPONENT*);
		void parse_logic_using_obsolete_callback(CS&, COMPONENT*);
		void cmdproc(CS&, CARD_LIST*);
	private: // override virtual, called by print_item
		void print_paramset(OMSTREAM&, const MODEL_CARD*);
		void print_module(OMSTREAM&, const BASE_SUBCKT*);
		void print_instance(OMSTREAM&, const COMPONENT*);
		void print_comment(OMSTREAM&, const DEV_COMMENT*);
		void print_command(OMSTREAM&, const DEV_DOT*);
	private: // local
		void print_args(OMSTREAM&, const MODEL_CARD*);
		void print_type(OMSTREAM&, const COMPONENT*);
		void print_args(OMSTREAM&, const COMPONENT*);
		void print_label(OMSTREAM&, const COMPONENT*);
		void print_ports(OMSTREAM&, const COMPONENT*);
};

/*--------------------------------------------------------------------------*/

// idea: different qucs language versions?
// (hopefully not!)
class LANG_QUCS : public LANG_QUCSATOR {
	public:
		std::string name()const {return "qucs";}
		bool case_insensitive()const {return false;}
		UNITS units()const {return uSI;}
		void parse_top_item(CS&, CARD_LIST*);
} lang_qucs;
DISPATCHER<LANGUAGE>::INSTALL
ds(&language_dispatcher, lang_qucs.name(), &lang_qucs);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class CMD_SUBCKT : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)
  {
    BASE_SUBCKT* new_module = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("subckt"));
    assert(new_module);
    assert(!new_module->owner());
    assert(new_module->subckt());
    assert(new_module->subckt()->is_empty());
    assert(!new_module->is_device());
    lang_qucs.parse_module(cmd, new_module);
    Scope->push_back(new_module);
  }
} p2;
DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "Def", &p2);
/*--------------------------------------------------------------------------*/
DEV_COMMENT p0;
DISPATCHER<CARD>::INSTALL
d0(&device_dispatcher, ";|*|'|\"|dev_comment", &p0);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void skip_pre_stuff(CS& cmd)
{
	cmd.skipbl();
	while (cmd.umatch(CKT_PROMPT)) {untested();
		/* skip any number of copies of the prompt */
	}
	cmd.umatch(QUCS_ANTI_COMMENT); /* skip mark so spice ignores but gnucap reads */
}
/*--------------------------------------------------------------------------*/
/* count_ports: figure out how many ports
 * returns the number of ports
 * side effect:  "CS" is advanced to past the ports, ready for what follows
 */
static int count_ports(CS& cmd, int maxnodes, int minnodes, int leave_tail, int start)
{
	trace4("count_ports", leave_tail, start, maxnodes, cmd.tail());
	assert(start <= maxnodes);
	assert(minnodes <= maxnodes);

	int num_nodes = 0;
	std::vector<unsigned> spots;
	int paren = cmd.skip1b('(');
	int i = start;
	// loop over the tokens to try to guess where the nodes end
	// and other stuff begins
	spots.push_back(cmd.cursor());
	for (;;) {
		trace1("loop", cmd.tail());

		++i;
		//cmd.skiparg();
		std::string node_name;
		cmd >> node_name;
		spots.push_back(cmd.cursor());

		if (paren && cmd.skip1b(')')) { untested();
			num_nodes = i;
			break;
		}else if (cmd.is_end()) {
			// found the end, no '='
			if (i <= minnodes) {
				num_nodes = i;
			}else if (i <= minnodes + leave_tail) { untested();
				num_nodes = minnodes;
			}else if (i <= maxnodes + leave_tail) {
				num_nodes = i - leave_tail;
			}else{ untested();
				num_nodes = maxnodes;
			}
			break;
		}else if (cmd.skip1b("({})")) { untested();
			// found '(', it's past the end of nodes
			if (i > maxnodes + leave_tail) { untested();
				num_nodes = maxnodes;
			}else{ untested();
				num_nodes = i - leave_tail;
			}
			break;
		}else if (cmd.skip1b('=')) {
			// found '=', it's past the end of nodes
			if (i > maxnodes + leave_tail + 1) { untested();
				num_nodes = maxnodes;
			}else{
				num_nodes = i - leave_tail - 1;
			}
			break;
		}else{
		}
	}
	trace2("nn", num_nodes, start);
	if (num_nodes < start) { untested();
		cmd.reset(spots.back());
		throw Exception("what's this (qucs)?");
	}else{
	}

	cmd.reset(spots[static_cast<unsigned>(num_nodes-start)]);

	//cmd.warn(bDANGER, "past-nodes?");
	//BUG// assert fails on current controlled sources with (node node dev) syntax
	// it's ok with (node node) dev syntax or node node dev syntax
	assert(num_nodes <= maxnodes);
	trace2("count_ports", num_nodes, cmd.tail());
	return num_nodes;
}
/*--------------------------------------------------------------------------*/
/* parse_ports: parse circuit connections from input string
 * fills in the rest of the array with 0
 * returns the number of nodes actually read
 * The purpose of this complexity is to handle the variants of Spice formats,
 * which might have keys between nodes,
 * an unknown number of nodes with unknown number of args and no delimeters.
 * Consider:  X1 a b c d e f g h i j k
 * Clearly (?) a,b,c,d,e are nodes, f is a subckt name, the rest are args.
 * But how do you know?
 *
 * Args:
 * maxnodes: the maximum number of port nodes to parse.
 *	Stop here, even if it seems there should be more.
 *
 * minnodes: the minimum number of port nodes to parse.
 *	It is an error if there are fewer than this.
 *
 * leave_tail: The number of arguments that are not nodes,
 *	and don't have equal signs, following.
 *	It is an aid to distinguishing the nodes from things that follow.
 *	minnodes wins over leave_tail, but leave_tail wins over maxnodes.
 *	The above example would need leave_tail=6 to parse correctly.
 *	This one: X1 a b c d e f g=h i=j k
 *	where a,b,c,d,e are nodes, f is a subckt or model identifier
 *	would parse correctly with leave_tail=1.
 *	Usual values for leave_tail are 0 or 1.
 *
 * start: start at this number, used for continuing after HSPICE kluge.
 *	Other than that, the only useful value for start is 0.
 *
 * all_new: All node names must be new (not already defined) and unique.
 *	This is used for the header line in .subckt, which starts clean.
 *	The main benefit is to reject duplicates.
 */
void LANG_QUCSATOR::parse_ports(CS& cmd, COMPONENT* x, int minnodes,
		int start, int num_nodes, bool all_new)
{
	assert(x);

	int paren = cmd.skip1b('(');
	int ii = start;
	size_t here1 = cmd.cursor();
	try{
		for (;;) {
			here1 = cmd.cursor();
			if (paren && cmd.skip1b(')')) { untested();
				--paren;
				break; // done.  have closing paren.
			}else if (ii >= num_nodes) {
				break; // done.  have maxnodes.
			}else if (!cmd.more()) {untested();
				break; // done.  premature end of line.
			}else{
				//----------------------
				size_t here = cmd.cursor();
				std::string node_name;
				cmd >> node_name;
				if (cmd.stuck(&here)) {itested();
					// didn't move, probably a terminator.
					throw Exception("bad node name");
				}else if(node_name=="gnd"){
					x->set_port_by_index(ii, ground_name);
				}else{
					// legal node name, store it.
					x->set_port_by_index(ii, node_name);
				}
				//----------------------
				if (!(x->node_is_connected(ii))) {untested();
					break; // illegal node name, might be proper exit.
				}else{
					if (all_new) {
						if (x->node_is_grounded(ii)) { untested();
							cmd.warn(bDANGER, here1, "node 0 not allowed here");
						}else{
						}
					}else{
					}
					++ii;
				}
			}
		}
	}catch (Exception& e) {itested();
		cmd.warn(bDANGER, here1, e.message());
	}
	if (ii < minnodes) { untested();
		cmd.warn(bDANGER, "need " + to_string(minnodes-ii) +" more nodes");
	}else{
	}
	if (paren != 0) {untested();
		cmd.warn(bWARNING, "need )");
	}else{
	}
	//assert(x->_net_nodes == ii);

	// ground unused input nodes
	for (int iii = ii;  iii < minnodes;  ++iii) { untested();
		x->set_port_to_ground(iii);
	}
	//assert(x->_net_nodes >= ii);
}
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::parse_type(CS& cmd, CARD* x)
{
	assert(x);
	std::string new_type;
	new_type = cmd.get_to(":");
	trace3("LANG_QUCSATOR::parse_type", cmd.fullstring(), new_type, x->dev_type());

	// HACK (qucs language misfeature)
	if(new_type=="Sub"){
		// it's not really Sub. let's see.
		int here=cmd.cursor();
		scan_get(cmd, "Type", &new_type);
		cmd.reset(here);
	}
	trace3("LANG_QUCSATOR::parse_type", cmd.fullstring(), new_type, x->dev_type());

	// this has weird side effects for elt+bm
	x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::parse_args(CS& cmd, CARD* x)
{
	trace1("LANG_QUCSATOR::parse_args (card)", cmd);
	assert(x);
	COMPONENT* xx = dynamic_cast<COMPONENT*>(x);
	if (xx) {
		COMMON_COMPONENT* cc = xx->mutable_common();
		size_t here = cmd.cursor();
		for (size_t i=0; ; ++i) {
			if (!cmd.more()) {
				break;
			}else{
				std::string Name  = cmd.ctos("=", "", "");
				cmd >> '=';
				std::string value = cmd.ctos(",=;)", "\"'{[(", "\"'}])");
				if(!value.size()) {
				}else if(value[0]=='['){
					value[0]='{';
					value[value.size()-1]='}';
				}else{
				}
				trace2("LANG_QUCSATOR::parse_args", Name, value);
				size_t there = here;
				if (cmd.stuck(&here)) { untested();
					break;
				}else{
					try{
						if (value == "") {
							cmd.warn(bDANGER, there, x->long_label() + ": " + Name + " has no value?");
						}else if (QucsGuessParam(value)){
							trace1("guessed value", value);
						}else{ untested();
						}

						OPT::case_insensitive = true;
						bool isame = Umatch(Name, xx->value_name()); // HACK, value_name is case sensitive
						                                             // and does not permit alternatives
						                                             // maybe write smarter wrappers instead...
						OPT::case_insensitive = false;

						trace3("LANG_QUCSATOR value_name", xx->value_name(), value, Name);
						if(value==""){
							// bug in qucs?
						}else if (cc && isame) {untested();
							trace1("isame hack", value);
							CS v(CS::_STRING, value);
							cc->parse_numlist(v); // HACK.
							try{ // to be sure, maybe later.
							  	xx->set_param_by_name(Name,value);
							}catch(Exception const&){
							}
						}else if (cc){
							trace2("have common", Name, value);

							try{
								cc->set_param_by_name(Name, value);
							}catch(Exception const&){ untested();
							  	// retry if common did not like it...
								xx->set_param_by_name(Name, value);
							}
						} else {
							trace2("no common", Name, value);
							xx->set_param_by_name(Name,value);
						}

					}catch (Exception_No_Match&) {untested();
						cmd.warn(bDANGER, there, x->long_label() + ": bad parameter " + Name + " ignored");
					}
				}
			}
		}
		xx->attach_common(cc);

	}else if (dynamic_cast<MODEL_CARD*>(x)) { untested();
		unreachable(); // used only for "table"
		int paren = cmd.skip1b('(');
		bool in_error = false;
		for (;;) { untested();
			size_t here = cmd.cursor();
			// pp->parse_params_obsolete_callback(cmd);  //BUG//callback//
			if (!cmd.more()) { untested();
				break;
			}else if (paren && cmd.skip1b(')')) {untested();
				break;
			}else if (cmd.stuck(&here)) {untested();
				if (in_error) {untested();
					// so you don't get two errors on name = value.
					cmd.skiparg();
					in_error = false;
				}else{untested();
					cmd.warn(bDANGER, "bad paramerter -- ignored");
					cmd.skiparg().skip1b("=");
					in_error = true;
				}
			}else{ untested();
				in_error = false;
			}
		}
	}
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::parse_label(CS& cmd, CARD* x)
{
	trace0(("LANG_QUCSATOR::parse_label " + cmd.tail()).c_str() );
	assert(x);

	if (cmd.skip1b(":") != true){ untested();
		cmd.reset(0);
		throw Exception("missing colon. device name not recognized: " + cmd.fullstring());
	}

	std::string my_name;
	cmd >> my_name;
	trace1("LANG_QUCSATOR::parse_label LABELNAME  ",my_name);
	x->set_label(my_name);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_COMMENT* LANG_QUCSATOR::parse_comment(CS& cmd, DEV_COMMENT* x)
{
	assert(x);
	x->set(cmd.fullstring());
	return x;
}
/*--------------------------------------------------------------------------*/
DEV_DOT* LANG_QUCSATOR::parse_command(CS& cmd, DEV_DOT* x)
{
	assert(x);
	x->set(cmd.fullstring());
	CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;

	cmd.reset().umatch(QUCS_ANTI_COMMENT);
	skip_pre_stuff(cmd);
	unsigned here = cmd.cursor();

	std::string id_string;
	cmd.reset(here); // skip dot
	if (cmd.scan(":")){
		cmd.reset(here); // skip dot again
		id_string = cmd.get_to(":");
	} else {
		cmd >> id_string;
	}
	trace1("LANG_QUCSATOR::parse_command", id_string);
	cmd.reset(here);
	if (!command_dispatcher[id_string]) {
		// there's just a DOT in the way. duh
		cmd.skip();
		++here;
	}else{
	}

	// WARNING: not to be confused with CMD::cmdproc
	cmdproc(cmd, scope);

	delete x;
	return NULL;
}
/*--------------------------------------------------------------------------*/
MODEL_CARD* LANG_QUCSATOR::parse_paramset(CS& cmd, MODEL_CARD* x)
{ untested();
	assert(x);
	cmd.reset();
	cmd >> ".model ";
	parse_label(cmd, x);
	parse_type(cmd, x);
	parse_args(cmd, x);
	cmd.check(bWARNING, "parse:paramset: what's this?");
	return x;
}
/*--------------------------------------------------------------------------*/
BASE_SUBCKT* LANG_QUCSATOR::parse_module(CS& cmd, BASE_SUBCKT* x)
{
	trace0(("LANG_QUCSATOR::parse_module " + cmd.tail()) );
	assert(x);

	// header
	cmd.reset();
	(cmd >> ".Def");
	parse_label(cmd, x);
	{
		unsigned here = cmd.cursor();
		trace2("LANG_QUCSATOR::parse_module ", x->min_nodes(), x->max_nodes() );
		int num_nodes = count_ports(cmd, x->max_nodes(), x->min_nodes(),
				0/*no unnamed par*/, 0/*start*/);

		trace1("LANG_QUCSATOR::parse_module ", num_nodes );
		cmd.reset(here);
		parse_ports(cmd, x, x->min_nodes(), 0/*start*/, num_nodes, true/*all new*/);
	}
	x->subckt()->params()->parse(cmd);

	// body
	parse_module_body(cmd, x, x->subckt(), name() + "-subckt>", NO_EXIT_ON_BLANK, ".Def:End ");
	trace0("LANG_QUCSATOR::parse_module done " );
	return x;
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::parse_module_body(CS& cmd, BASE_SUBCKT* x, CARD_LIST* Scope,
		const std::string& prompt, EOB exit_on_blank, const std::string& exit_key)
{
	try {
		for (;;) {
			cmd.get_line(prompt);

			if ((exit_on_blank==EXIT_ON_BLANK && cmd.is_end()) 
					|| cmd.umatch(exit_key)) {
				break;
			}else{
				trace2("LANG_QUCSATOR::parse_module_body ", cmd.fullstring(), OPT::language);
				skip_pre_stuff(cmd);
				new__instance(cmd, x, Scope);
			}
		}
	}catch (Exception_End_Of_Input& e) { untested();
	}
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_QUCSATOR::parse_instance(CS& cmd, COMPONENT* x)
{
	assert(x);
	cmd.reset().umatch(QUCS_ANTI_COMMENT);

	// cmd.skip1b(":");
	cmd.reset();
	parse_type(cmd, x);
	string id_string = cmd.get_to(":");

	COMMON_COMPONENT * c = NULL;

	parse_label(cmd, x);

	{
		unsigned here = cmd.cursor();
		int num_nodes = count_ports(cmd, x->max_nodes(), x->min_nodes(), 0, 0);
		//int num_nodes = count_ports(cmd, x->max_nodes(), x->min_nodes(), x->tail_size(), 0);
		cmd.reset(here);
		parse_ports(cmd, x, x->min_nodes(), 0/*start*/, num_nodes, false);
		trace0(("LANG_QUCSATOR::parse_instance parsed ports " + (std::string) cmd.tail()).c_str() );

	}

	try {

		if (x->print_type_in_spice()) {
			trace0(("LANG_QUCSATOR::parse_instance ptis " + (std::string) cmd.tail()).c_str() );
			// parse_type(cmd, x);
		}else{
		}

		if (c) { untested();
			trace2("LANG_QUCSATOR::parse_instance", c, x->long_label());
			x->attach_common(c);    
			parse_args(cmd, x);

			// At this point, there is ALWAYS a common "c", which may have more
			// commons attached to it.  Try to reduce its complexity.
			// "c->deflate()" may return "c" or some simplification of "c".
			assert(x->mutable_common());
			COMMON_COMPONENT* dc = x->mutable_common()->deflate();

			// dc == deflated_common
			// It might be just "c".
			// It might be something else that is simpler but equivalent.
			x->attach_common(dc);

		}else{
			parse_args(cmd, x);
		}


	}catch (Exception& e) { untested();
		cmd.warn(bDANGER, e.message());
	}
	return x;
}


/*--------------------------------------------------------------------------*/
std::string LANG_QUCSATOR::find_type_in_string(CS& cmd) GCUF_CONST
{
	cmd.umatch(QUCS_ANTI_COMMENT);

	unsigned here = cmd.cursor();
	std::string id_string;

	char first_letter = cmd.peek();
	trace2("LANG_QUCSATOR::find_type_in_string", first_letter, cmd.tail());

	assert(!OPT::case_insensitive);

	if (first_letter == '.') {
		cmd.reset(here+1); // cut the dot
		if(cmd.scan(":")){
			cmd.reset(here+1); // cut the dot again
			id_string = cmd.get_to(":");
			trace1("LANG_QUCSATOR::parse_instance colon", id_string);
		}else {
			cmd >> id_string;
		}
		trace1("LANG_QUCSATOR::find_type_in_string found leading dot. assuming command",id_string);
		if (!command_dispatcher[id_string]) { untested();
			cmd.skip();
			++here;
			id_string = id_string.substr(1);
		}else{
		}
		trace1("LANG_QUCSATOR::find_type_in_string found command", id_string);
		return id_string;
	}else if (!cmd.scan(":")){
		cmd >> id_string;
	}else{
		cmd.reset();
		id_string = cmd.get_to(":");
		cmd.reset(here);
		if(id_string=="Sub"){
			trace1("get_to", cmd.tail());
			scan_get(cmd, "Type", &id_string);
			trace1("got_to", cmd.tail());
		}else{
		}
	}
	trace2("LANG_QUCSATOR::find_type_in_string", cmd.fullstring(), id_string);
	return id_string;
}
/*--------------------------------------------------------------------------*/
// cloning from c__cmd.cc for now
void LANG_QUCSATOR::cmdproc(CS& cmd, CARD_LIST* scope)
{
	trace1("LANG_QUCSATOR::cmdproc", cmd.fullstring());
	bool get_timer_was_running = ::status.get.is_running();
	::status.get.stop();

	static TIMER timecheck;
	bool didsomething = true;

	error(bTRACE, ">>>>>" + cmd.fullstring() + "\n");

	timecheck.stop().reset().start();

	if(cmd.umatch(QUCS_ANTI_COMMENT)){ unreachable();
	}

	while (cmd.umatch(I_PROMPT)) {itested();
		/* skip any number of these */
	}

	unsigned here = cmd.cursor();
	std::string id_string;
	std::string cmdname;
	trace1("LANG_QUCSATOR::cmdproc", cmd.tail());
	if(cmd.scan(":")){
		cmd.reset(here);
		id_string = cmd.get_to(":");
		cmd.skip1b(":");
		cmd >> cmdname;
	}else{
		cmd >> id_string;
	}

	trace2("LANG_QUCSATOR::cmdproc", id_string, cmdname);
	if (cmd.umatch("'|*|#|//|\"")) { untested();
		unreachable();
	}else if (id_string != "") {
		CMD* c = command_dispatcher[id_string];
		if (c) {
			c->set_label(cmdname);

			c->do_it(cmd, scope);
			trace1("LANG_QUCSATOR::cmdproc",c->short_label());
			didsomething = true;
		}else{itested();
			cmd.warn(bWARNING, here, "cmd: what's this?");
		}
	}else if (!didsomething) {itested();
		cmd.check(bWARNING, "bad command");
		didsomething = false;
	}else{itested();
	}

	if (OPT::acct  &&  didsomething) { untested();
		IO::mstdout.form("time=%8.2f\n", timecheck.check().elapsed());
	}else{
	}
	outreset();

	if (get_timer_was_running) { untested();
		::status.get.start();
	}else{
	}
}
/*--------------------------------------------------------------------------*/
void LANG_QUCS::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
	if (0 && cmd.is_file()
			&& cmd.is_first_read()
			&& (Scope == &CARD_LIST::card_list)
			&& (Scope->is_empty())
			&& (head == "'")) {untested();	//BUG// ugly hack
		cmd.get_line("gnucap-qucs-title>");
		head = cmd.fullstring();
		IO::mstdout << head << '\n';
	}else{itested();
		cmd.get_line("gnucap-qucs>");
		new__instance(cmd, NULL, Scope);
	}
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_paramset(OMSTREAM& o, const MODEL_CARD* x)
{
	assert(x);
	o << ".model " << x->short_label() << ' ' << x->dev_type() << " (";
	print_args(o, x);
	o << ")\n";
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_module(OMSTREAM& o, const BASE_SUBCKT* x)
{
	assert(x);
	assert(x->subckt());

	o << ".Def:" <<  x->short_label();
	print_ports(o, x);
	o << '\n';

	for (CARD_LIST::const_iterator 
			ci = x->subckt()->begin(); ci != x->subckt()->end(); ++ci) {
		print_item(o, *ci);
	}

	o << ".Def:End\n";
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_instance(OMSTREAM& o, const COMPONENT* x)
{
	print_type(o, x);
	o << ":";
	print_label(o, x);
	print_ports(o, x);
	print_args(o, x);
	o << '\n';
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_comment(OMSTREAM& o, const DEV_COMMENT* x)
{
	assert(x);
	if (x->comment()[1] != '+') {
		o << x->comment() << '\n';
	}else{ untested();
	}
	// Suppress printing of comment lines starting with "*+".
	// These are generated as a way to display calculated values.
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_command(OMSTREAM& o, const DEV_DOT* x)
{ untested();
	assert(x);
	o << x->s() << '\n';
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_args(OMSTREAM& o, const MODEL_CARD* x)
{
	assert(x);
	for (int ii = x->param_count() - 1;  ii >= x->param_count_dont_print();  --ii) {
		if (x->param_is_printable(ii)) {
			std::string arg = " " + x->param_name(ii) + "=" + x->param_value(ii);
			o << arg;
		}else{ untested();
		}
	}
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_type(OMSTREAM& o, const COMPONENT* x)
{
	assert(x);
	o << "  " << x->dev_type();
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_args(OMSTREAM& o, const COMPONENT* x)
{
	assert(x);
	o << ' ';
		for (int ii = x->param_count() - 1;  ii >= x->param_count_dont_print();  --ii) {
			if (x->param_is_printable(ii)) {
				if ((ii != x->param_count() - 1) || (x->param_name(ii) != x->value_name())) {
					// skip name if plain value
					o << " " << x->param_name(ii) << "=";
				}else{
				}
				o << x->param_value(ii);
			}else{
			}
		}
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_label(OMSTREAM& o, const COMPONENT* x)
{
	assert(x);
	std::string label = x->short_label();
	o << label;
}
/*--------------------------------------------------------------------------*/
void LANG_QUCSATOR::print_ports(OMSTREAM& o, const COMPONENT* x)
{
	assert(x);

	o <<  " ";
	std::string sep = "";
	for (unsigned ii = 0;  x->port_exists(ii);  ++ii) {
		o << sep;
		if(x->port_value(ii) == "0"){
			o << "gnd";
		}else{
		   o << x->port_value(ii);
		}
		sep = " ";
	}
	for (unsigned ii = 0;  x->current_port_exists(ii);  ++ii) { untested();
		o << sep << x->current_port_value(ii);
		sep = " ";
	}
	o << " ";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
enum Skip_Header {NO_HEADER, SKIP_HEADER};
/*--------------------------------------------------------------------------*/
/* getmerge: actually do the work for "get", "merge", etc.
*/
# if 0 // what is this good for?
static void getmerge(CS& cmd, Skip_Header skip_header, CARD_LIST* Scope)
{ unreachable();
	::status.get.reset().start();
	assert(Scope);

	std::string file_name, section_name;
	cmd >> file_name;

	bool  echoon = false;		/* echo on/off flag (echo as read from file) */
	bool  liston = false;		/* list on/off flag (list actual values) */
	bool  quiet = false;		/* don't echo title */
	unsigned here = cmd.cursor();
	do{ untested();
		ONE_OF
			|| Get(cmd, "echo",  &echoon)
			|| Get(cmd, "list",  &liston)
			|| Get(cmd, "quiet", &quiet)
			|| Get(cmd, "section", &section_name)
			;
	}while (cmd.more() && !cmd.stuck(&here));
	if (cmd.more()) { untested();
		cmd >> section_name;
	}else{ untested();
	}
	cmd.check(bWARNING, "need section, echo, list, or quiet");

	trace0(("getmerge opening " + file_name).c_str());
	CS file(CS::_INC_FILE, file_name);

	if (skip_header) { // get and store the header line
		file.get_line(">>>>");
		head = file.fullstring();

		if (!quiet) { untested();
			IO::mstdout << head << '\n';
		}else{untested();
		}
	}else{ untested();
	}
	if (section_name == "") { untested();
		trace0("spice?");
		lang_qucs.parse_module_body(file, NULL, Scope, ">>>>", lang_qucs.NO_EXIT_ON_BLANK, ".end ");
		trace0("done spice?");
	}else{ untested();
		try { untested();
			for (;;) { untested();
				file.get_line("lib " + section_name + '>');
				if (file.umatch(".lib " + section_name + ' ')) { untested();
					lang_qucs.parse_module_body(file, NULL, Scope, section_name,
							lang_qucs.NO_EXIT_ON_BLANK, ".endl {" + section_name + "}");
				}else{ untested();
					// skip it
				}
			}
		}catch (Exception_End_Of_Input& e) { untested();
		}
	}
	::status.get.stop();
}
#endif
/*--------------------------------------------------------------------------*/
	class CMD_QUCS : public CMD {
		public:
			void do_it(CS&, CARD_LIST* Scope)
			{
				command("options lang=qucs", Scope);
			}
	} p9;
	DISPATCHER<CMD>::INSTALL d9(&command_dispatcher, "qucs|#qucs", &p9);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:noet
