/*                            -*- C++ -*-
 * Copyright (C) 2020 Felix Salfelder
 * Author: Felix Salfelder <felix@salfelder.org>
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
 * Gnucap driver for Qucs.
 */

// Kludge
//  - use Qucsator models
//  - stick to "dat" for now.
// TODO
//  - model selection
//  - run in separate thread
//  - construct proper data

// Gnucap
#include <md.h>
#include <e_base.h>
#include <e_cardlist.h>
#include <e_subckt.h>
#include <io_error.h>
#include "../env.h"
#include <globals.h>
#include <u_lang.h>
#include <c_comand.h>
#include <u_prblst.h>

// Qucs
#undef AP_H
#define Parameter PARAMETER
#include <command.h>
#include <simulator.h>
#include <qucs_globals.h>
#include <settings.h>
#include <qio.h>
#include <sckt_base.h>
#include <element_list.h>
#include <symbol.h>
#undef CMD
#undef OPT
#undef DISPATCHER
#undef PARA_BASE
#undef CS
#undef DEV_DOT

// #include <boost/thread/thread.hpp> later.

namespace{
/* -------------------------------------------------------------------------------- */
using qucs::Simulator;
using qucs::SimCtrl;
using qucs::Symbol;
using qucs::SubcktBase;
using qucs::ElementList;
using qucs::TaskElement;
/* -------------------------------------------------------------------------------- */
// omit this hack.
class CMD_HIDE : public CMD {
public:
  void do_it(CS&, CARD_LIST*) {}
} p0;
DISPATCHER<CMD>::INSTALL dh(&command_dispatcher, "hidemodule", &p0);
/* -------------------------------------------------------------------------------- */
static void command(std::string const& cmd, BASE_SUBCKT* scope)
{
	assert(scope);
	assert(scope->subckt());
	CS cs(CS::_STRING, cmd);
	OPT::language->new__instance(cs, scope, scope->subckt());
}
/* -------------------------------------------------------------------------------- */
static void read_startup_files(char *const* argv, CARD_LIST* scope)
{
  {
    // TODO: look in $HOME/.gnucap/config:/etc/gnucap/config
    trace2("read_startup_files", SYSTEMSTARTFILE, SYSTEMSTARTPATH);
    std::string name = findfile(SYSTEMSTARTFILE, SYSTEMSTARTPATH, R_OK);
    if (name != "") {
      trace2("", name, scope);
      CMD::command("include " + name, &CARD_LIST::card_list);
      trace2("done", name, &CARD_LIST::card_list);
    }else{ untested();
      trace1("not found", name);
      CMD::command(std::string("load " DEFAULT_PLUGINS), &CARD_LIST::card_list);
    }
  }
  {
    // TODO: also scan parent directories
    std::string name = findfile(USERSTARTFILE, USERSTARTPATH, R_OK);
    if (name != "") {untested();
      CMD::command("include " + name, scope);
    }else{
    }
  }
  //CMD::command("clear", &CARD_LIST::card_list);
  if (!OPT::language) { untested();
    OPT::language = language_dispatcher[DEFAULT_LANGUAGE];

    for(DISPATCHER<LANGUAGE>::const_iterator i=language_dispatcher.begin();
        !OPT::language && i!=language_dispatcher.end(); ++i) {untested();
      OPT::language = prechecked_cast<LANGUAGE*>(i->second);
    }
  }else{
    // already have a language specified in a startup file
  }
  if (OPT::language) {
    OPT::case_insensitive = OPT::language->case_insensitive();
    OPT::units            = OPT::language->units();
  }else{ untested();
    OPT::case_insensitive = false;
    OPT::units            = uSI;
  }
}
/* -------------------------------------------------------------------------------- */
char const* argv[] = {"gnucsator"};
class Gnucap : public Simulator {
private:
	Gnucap(Gnucap const& s);
public:
	explicit Gnucap();
	~Gnucap() {
		delete _data;
		_data = nullptr;
	}

private: // Element
  Simulator* clone() const override{ return new Gnucap(*this); }

private: // Simulator
  virtual void init() override{ incomplete();}
  void run(istream_t& cs, SimCtrl* ctx) override {incomplete();}
  void join() override {incomplete();}
  void do_it(istream_t& cs, ElementList const* ctx) override;
  std::string errorString() const override {incomplete(); return "..";}
  void kill() override {incomplete();}

private: // internal
	void load_circuit(ElementList const* ctx, BASE_SUBCKT* model);
	void load_symbol(Symbol const* i, BASE_SUBCKT* model);
	void load_task(TaskElement const* i, BASE_SUBCKT* model);
	void copy_param_values(Symbol const* sym, COMPONENT* model);

private:
//   boost::thread _t; later.
  BASE_SUBCKT* _main;
  SIM_DATA _sim_data; // for now.
  PROBE_LISTS _probe_lists; // for now

  Data* _data{nullptr};
} g;
static Dispatcher<Data>::INSTALL d0(&qucs::data_dispatcher, "gnucap", &g);
/* -------------------------------------------------------------------------------- */
Gnucap::Gnucap() : Simulator(), _main(nullptr)
{
	set_label("gnucap");
	trace1("Gnucap::Gnucap", &_sim_data);
	CKT_BASE::_sim = &_sim_data;
	CKT_BASE::_probe_lists = &_probe_lists;
}
/* -------------------------------------------------------------------------------- */
Gnucap::Gnucap(Gnucap const& s) : Simulator(s), _main(nullptr)
{
	trace1("Gnucap::Gnucap copy", &_sim_data);
	CARD* c = device_dispatcher.clone("subckt");
	_main = prechecked_cast<BASE_SUBCKT*>(c);
	_main->set_label("main");
	_main->COMPONENT::precalc_first(); // set mfactor (bug?)
	assert(_main->mfactor() == 1.);
	assert(_main);
	assert(_main->subckt());
	assert(c);
	prepare_env();
	read_startup_files((char * const*) argv, &CARD_LIST::card_list);

	{ // yikes.
		CKT_BASE::_sim = &_sim_data;
		CKT_BASE::_probe_lists = &_probe_lists;
		//CARD_LIST::card_list.push_back(_main);
	}
}
/* -------------------------------------------------------------------------------- */
struct default_sim{
	default_sim(){
		QucsSettings.setSimulator(&g);
	}
}ds;
/* -------------------------------------------------------------------------------- */
void Gnucap::copy_param_values(Symbol const* sym, COMPONENT* model)
{
	for(index_t i=0; i<sym->param_count(); ++i){
		std::string name = sym->param_name(i);
		std::string value = sym->param_value(i);

		if(name[0]=='$'){
		}else{
			try{
				model->set_param_by_name(name, value);
			}catch(Exception_No_Match const&){
				message(qucs::MsgWarning, "cannot set " + name + " in " + model->short_label());
			}
		}
	}
}
/* -------------------------------------------------------------------------------- */
static void copy_port_values(Symbol const* sym, COMPONENT* model)
{
	for(index_t i=0; i<sym->numPorts(); ++i){
		std::string name = sym->portName(i);
		std::string value = sym->port_value(i);
		auto owner=prechecked_cast<SubcktBase const*>(sym->owner());
		assert(owner);
		std::string net = net_name(owner, value);
		trace3("copy_port_values", owner->label(), value, net);
		trace3("port", sym->label(), name, net);
		if(net=="gnd"){
			net = "0";
		}else{
		}
		model->set_port_by_index(i, net);
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::load_task(TaskElement const* tsk, BASE_SUBCKT* scope)
{
	if(scope == _main){
		std::string cmd = tsk->dev_type();
		for(index_t i=0; i<tsk->param_count(); ++i){
			cmd += " " + tsk->param_name(i) + "=\"" + tsk->param_value(i) + "\"";
		}
		trace2("TASK", tsk->label(), cmd);

		CS cs(CS::_STRING, cmd);
		OPT::language->new__instance(cs, scope, scope->subckt());
	}else{
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::load_symbol(Symbol const* i, BASE_SUBCKT* model)
{
	assert(model->scope());
	assert(OPT::language);
	CARD const* proto = OPT::language->find_proto(i->dev_type(), model);
	if(proto){
		trace3("load", i->dev_type(), i->label(), i->has_common());
		CARD* c = proto->clone_instance();
		assert(c);

		// TODO: this does not work for nontrivial models
		if(auto d=dynamic_cast<COMPONENT*>(c)){
			d->set_label(i->label());
			d->set_dev_type(i->dev_type());
			d->set_owner(model);
			copy_param_values(i, d);
			model->scope()->push_back(d);
			copy_port_values(i, d);
		}else{ untested();
		}
	}else{
		trace3("unknown", i->dev_type(), i->label(), i->has_common());
		incomplete();
		return;
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::load_circuit(ElementList const* ctx, BASE_SUBCKT* model)
{
	assert(model->scope());
	for(auto i : *ctx){
		if(auto sym = dynamic_cast<Symbol const*>(i)){
			load_symbol(sym, model);
		}else if(auto task = dynamic_cast<TaskElement const*>(i)){
			load_task(task, model);
		}else{
		}
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::do_it(istream_t& cs, ElementList const* ctx)
{
	trace1("Gnucap::do_it", cs.fullstring());
	assert(_main->scope());
	_main->scope()->erase_all();

	auto i = ctx->find_("main");

	if (i==ctx->end()){ untested();
		message(qucs::MsgWarning, "cannot find main");
		return;
	}else if((*i)->scope()){
		load_circuit((*i)->scope(), _main);
	}else{ untested();
		incomplete();
		unreachable();
	}
	SET_RUN_MODE xx(rBATCH);

	trace0("================= list ============");
	command("list", _main);
	trace1("===================================", _sim_data.is_first_expand());
	command("go FILE", _main);
	trace0("============== DONE ===============");
	command("status notime", _main);

	_sim_data.uninit();

	delete _data;
	_data = nullptr;

	_data = qucs::data_dispatcher.clone("datfile");
	auto lang = qucs::language_dispatcher["qucsator"];
	assert(lang);

	istream_t stream(istream_t::_WHOLE_FILE, "FILE.tr");
	lang->parseItem(stream, _data);

	attach(_data->common());
} // Gnucap::do_it
/* -------------------------------------------------------------------------------- */
}
/* -------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------- */
