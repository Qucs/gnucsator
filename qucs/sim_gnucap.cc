/*                            -*- C++ -*-
 * Copyright (C) 2020, 2021 Felix Salfelder
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
//  - fallback to Qucsator models
//  - only one device per verilog file (inherited from Qucs, fix later)
//  - stick to "dat" for now.
// TODO
//  - model selection, Sub
//  - run in separate thread
//  - construct proper data
//  - error handling

// Gnucap
#include <md.h>
#include <e_base.h>
#include <e_cardlist.h>
#include <e_subckt.h>
#include <e_model.h>
#include <io_error.h>
#include "../env.h"
#include <globals.h>
#include <u_lang.h>
#include <c_comand.h>
#include <u_prblst.h>
#include <e_paramlist.h>

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
#include <place.h>
#include <conductor.h>
#include <model.h>
#include <factory.h>
#undef CMD
#undef OPT
#undef DISPATCHER
#undef PARA_BASE
#undef CS
#undef DEV_DOT

// #include <boost/thread/thread.hpp> later.
//
#include "../l_qucs.h" // QucsGuessParam

namespace{
/* -------------------------------------------------------------------------------- */
using qucs::Simulator;
using qucs::SimCtrl;
using qucs::Symbol;
using qucs::SubcktBase;
using qucs::ElementList;
using qucs::TaskElement;
using qucs::Place;
using qucs::Conductor;
using qucs::Component;
using qucs::Model;
using qucs::SymbolFactory;
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
char const* argv[] = {"gnucsator", nullptr};
class Gnucap : public Simulator {
private:
	Gnucap(Gnucap const& s);
public:
	explicit Gnucap();
	~Gnucap() {
		if(_root){
			CMD::command("clear", _root->subckt());
			delete _root;
			_root = nullptr;
		}else{
		}

		if(_data){
			delete _data;
		  _data = nullptr;
		}else{
		}
	}

private: // Element
  Simulator* clone() const override{ return new Gnucap(*this); }

private: // Simulator
  virtual void init() override{ incomplete();}
  void run(istream_t&, SimCtrl*) override {incomplete();}
  void join() override {incomplete();}
  void do_it(istream_t& cs, ElementList const* ctx) override;
  std::string errorString() const override {incomplete(); return "..";}
  void kill() override {incomplete();}

private: // internal
	void new_root();
	CARD_LIST* root_scope(){ return _root->subckt(); }

	void load_main(ElementList const* ctx);
	void load_symbol(Symbol const* i, BASE_SUBCKT* model);
	void load_task(TaskElement const* i, BASE_SUBCKT* model);
	void copy_param_values(Symbol const* sym, COMPONENT* model);

	void load_models(ElementList const* ctx, CARD_LIST* models);
	CARD* new_model_choose(qucs::Element const* m, ElementList const* scope);
	CARD* new_model_subckt(qucs::SubcktBase const*);
	CARD* new_model(qucs::Element const* m);

private:
//   boost::thread _t; later.
  BASE_SUBCKT* _main;
  CARD* _root{nullptr};
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
static COMMON_PARAMLIST Default_SUBCKT(CC_STATIC);
/* -------------------------------------------------------------------------------- */
Gnucap::Gnucap(Gnucap const& s) : Simulator(s), _main(nullptr)
{
	prepare_env();
	try{
		read_startup_files((char * const*) argv, &CARD_LIST::card_list);
	}catch(Exception_CS const& e){ itested();
		trace1("error reading startup files", e.message());
		message(qucs::MsgFatal, e.message());
		return;
	}

	try{ // HACK
		// just try and load those, in case they are available
      CMD::command(std::string("load custom/c_make_attach.so"), &CARD_LIST::card_list);
		CMD::command(std::string("load lang_adms.so"), &CARD_LIST::card_list);
		CMD::command(std::string("load d_adms_i.so"), &CARD_LIST::card_list);
		CMD::command(std::string("load d_adms_vbr.so"), &CARD_LIST::card_list);
	}catch(Exception_CS const& e){ itested();
		message(qucs::MsgFatal, "failed to load gnucap-adms modules: " + e.message());
	}

	new_root();
	untested();

	_root->precalc_first(); // set mfactor (bug?)
	_root->precalc_last(); // set mfactor (bug?)
	untested();

	_main->COMPONENT::precalc_first(); // set mfactor (bug?)
	untested();

	CARD_LIST::card_list.precalc_first(); // here?
	CARD_LIST::card_list.precalc_last(); // here?
	untested();

	assert(_main->mfactor() == 1.);
	assert(_main);
	assert(_main->subckt());

	// CS off(CS::_STRING, "off=0");
	// _root->subckt()->params()->parse(off);


	// TODO: set from qucs.
	// CMD::command(std::string("options trace"), &CARD_LIST::card_list);

	{ // yikes.
		CKT_BASE::_sim = &_sim_data;
		CKT_BASE::_probe_lists = &_probe_lists;
		//CARD_LIST::card_list.push_back(_main);
	}

	_main->attach_common(&Default_SUBCKT);
}
/* -------------------------------------------------------------------------------- */
struct default_sim{
	default_sim(){
		QucsSettings.setSimulator(&g);
	}
}ds;
/* -------------------------------------------------------------------------------- */
void Gnucap::new_root()
{
	_root = device_dispatcher.clone("root");
	assert(_root);
	_root->set_label("root");

	// move to ::ROOT
	{
	trace1("Gnucap::Gnucap copy", &_sim_data);
	CARD* c = device_dispatcher.clone("main");
	assert(c);
	_main = prechecked_cast<BASE_SUBCKT*>(c);
	assert(_main);
	_main->set_label("main");
	_main->set_owner(_root);
	}

	_root->subckt()->push_back(_main);
	assert(_main->scope() == _root->subckt());

	assert(_root->subckt());
	assert(_root->subckt()->params());
	assert(_main->subckt());
	assert(_main->subckt()->params());

	// need _root params?
	_main->subckt()->params()->set_try_again(CARD_LIST::card_list.params());
}
/* -------------------------------------------------------------------------------- */
void Gnucap::copy_param_values(Symbol const* sym, COMPONENT* x)
{
	for(index_t i=0; i<sym->param_count(); ++i){
		std::string name = sym->param_name(i);
		std::string value = sym->param_value(i);


		if(name[0]=='$'){
//			message(qucs::MsgTrace, "ignore " + name + " in " + x->short_label());
		}else if(value.substr(0,3) == "NA("){
			// BUG: printable?
			message(qucs::MsgWarning, "ignore NA: " + name + " in " + x->short_label());
		}else{
			QucsGuessParam(value); // strip of markup, essentially
			message(qucs::MsgTrace, "set " + name + " in " + x->short_label());
			try{
				x->set_param_by_name(name, value);
			}catch(Exception_No_Match const&){
				message(qucs::MsgWarning, "cannot set " + name + " in " + x->short_label());
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
	}else{ untested();
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::load_symbol(Symbol const* i, BASE_SUBCKT* model)
{
	assert(model->scope());
	assert(i);
	assert(OPT::language);

	std::string modelname;
	if(i->common()){
		modelname = i->common()->modelname();
	}else{
  		modelname = i->dev_type();
	}

	CARD const* proto = OPT::language->find_proto(modelname, model);
	if(!proto){
		assert(root_scope());
		auto i = root_scope()->find_(modelname);
		if(i!=root_scope()->end()){ untested();
			proto = *i;
		}else{
		}
	}else{
	}
	if(proto){
		message(qucs::MsgTrace, "load " + modelname);
		trace3("load", i->dev_type(), i->label(), i->has_common());
		CARD* c = proto->clone_instance();
		assert(c);

		std::string dev_type = i->dev_type();
		if(i->has_common()){
			dev_type = i->common()->modelname();
		}else{
		}

		if(auto d=dynamic_cast<COMPONENT*>(c)){
			trace2("load component", dev_type, i->label());
			d->set_label(i->label());
			d->set_dev_type(dev_type);
			d->set_owner(model);
			copy_param_values(i, d);
			model->subckt()->push_back(d);
			copy_port_values(i, d);
		}else{ untested();
		}
	}else if(i->subckt()){ untested();
		message(qucs::MsgWarning, "type with sckt: " + modelname + " " + i->label());
	}else if(modelname == "GND"){
		incomplete(); // ignore, for now.
	}else if(modelname == "Port"){
		incomplete(); // ignore, for now.
	}else{ untested();
		message(qucs::MsgFatal, "unknown type: " + modelname + " " + i->label());
	}
}
/* -------------------------------------------------------------------------------- */
// there is one verilog module in file, and its name is dev_type.
// load it and create a paramset for it.
static CARD* new_verilog_handle(std::string const& file, std::string const& dev_type)
{
	CMD::command("load_va " + file, &CARD_LIST::card_list);

	MODEL_CARD* paramset = model_dispatcher.clone(dev_type);

	assert(paramset);
	trace1("new_verilog_handle", paramset->dev_type());
	return paramset;
}
/* -------------------------------------------------------------------------------- */
static CARD* new_model_verilogref(qucs::Element const* m)
{
	std::string filename = "";
	std::string dev_type = "";
	try{
		filename = m->param_value_by_name("filename");
		dev_type = m->param_value_by_name("dev_type");
	}catch(qucs::ExceptionCantFind const&){ untested();
	}

	if(filename == ""){ incomplete();
	}else if(dev_type == ""){ incomplete();
	}else{
		auto h = new_verilog_handle(filename, dev_type);
		return h;
	}

	return nullptr;
}
/* -------------------------------------------------------------------------------- */
// BUG: same as load_main except load_tasks but ports.
CARD* Gnucap::new_model_subckt(qucs::SubcktBase const* from)
{
	assert(from);
	assert(from->makes_own_scope());
	auto c = device_dispatcher.clone("subckt");
	auto to = dynamic_cast<BASE_SUBCKT*>(c);
	assert(to);
	auto scope = from->scope();

	{ // tmp hack.
		for(index_t i=0; i<from->numPorts(); ++i){
			auto value = from->port_value(i);
			trace4("set proto port", i, from->label(), to->short_label(), value);
			to->set_port_by_index(i, value);
		}
	}

	for(auto i : *scope){
		if(dynamic_cast<Place const*>(i)){
		}else if(dynamic_cast<Conductor const*>(i)){
		}else if(auto sym = dynamic_cast<Symbol const*>(i)){
			load_symbol(sym, to);
		}
	}
	return to;
}
/* -------------------------------------------------------------------------------- */
CARD* Gnucap::new_model_choose(qucs::Element const* m, ElementList const* scope)
{
	auto it = scope->find_("VerilogRef");
	if(it==scope->end()){
	}else{ untested();
		return new_model_verilogref(*it);
	}

	it = scope->find_("main");
	if(it==scope->end()){
	}else if(auto s = dynamic_cast<SubcktBase const*>(*it)){
		return new_model_subckt(s);
	}else{ untested();
		message(qucs::MsgFatal, "main is not a circuit");
	}

	for(auto i : *scope){
		message(qucs::MsgLog, "found " + i->dev_type() + " " + i->short_label());

		if(i->dev_type()=="VerilogRef"){
			return new_model_verilogref(i);
		}else{
		}
	}
	message(qucs::MsgFatal, "cant find model for " + m->dev_type() + " " + m->label());
	/// throw? message?
	return nullptr;
}
/* -------------------------------------------------------------------------------- */
CARD* Gnucap::new_model(qucs::Element const* m)
{
	CARD* ret=nullptr;
	std::string modelname;
	ElementList const* l = nullptr;
	if(dynamic_cast<Model const*>(m)){ untested();
		incomplete();
//		l = q->subckt();
	}else if(auto q=dynamic_cast<qucs::Component const*>(m)){
		if(q->common()){
			modelname = q->common()->modelname();
		}else{ untested();
		}
		l = q->subckt();
	}else{ untested();
	}

	if(l){
		trace2("model", m->label(), m->dev_type());
		ret = new_model_choose(m, l);
	}else{ untested();
		trace1("empty model", m->label());
	}

	if(ret){
		ret->set_label(modelname);
	}else{ untested();
		trace1("no ret", modelname);
	}

	return ret;
}
/* -------------------------------------------------------------------------------- */
void Gnucap::load_models(ElementList const* ctx, CARD_LIST* models)
{
	assert(models);
	for(auto i : *ctx){
		if(i->label()=="main"){
		}else if(i->label()=="Symbol"){
			incomplete();
		}else if(i->label()=="protos"){
			incomplete();
		}else if(i->label()==":Paintings:"){
			incomplete();
		}else if(i->label().c_str()[0] == '.'){ untested();
			// ignore
		}else if(dynamic_cast<Simulator const*>(i)){
			incomplete();
		}else if(dynamic_cast<SymbolFactory const*>(i)){
			incomplete();
		}else if(dynamic_cast<Component const*>(i)
		      || dynamic_cast<Model const*>(i)){
			// need Model and Component?
			if(auto m = new_model(i)){
				trace2("got model", i->short_label(), m->short_label());
				m->set_owner(_root);
				root_scope()->push_back(m);
			}else{ untested();
				trace1("not a model?", i->short_label());
				message(qucs::MsgFatal, "cant load " + i->dev_type() + " " + i->label());

				incomplete();
			}
		}
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::load_main(ElementList const* ctx)
{
	for(auto i : *ctx){
		if(dynamic_cast<Place const*>(i)){
		}else if(dynamic_cast<Conductor const*>(i)){
		}else if(auto sym = dynamic_cast<Symbol const*>(i)){
			load_symbol(sym, _main);
		}else if(auto task = dynamic_cast<TaskElement const*>(i)){
			load_task(task, _main);
		}else{
		}
	}
}
/* -------------------------------------------------------------------------------- */
void Gnucap::do_it(istream_t& cs, ElementList const* ctx)
{
	trace1("Gnucap::do_it", cs.fullstring());
	assert(_main->scope());
	assert(_main->subckt());
	_main->subckt()->erase_all();

	load_models(ctx, root_scope());
	assert(_main->owner() == _root);

	auto i = ctx->find_("main");

	if (i==ctx->end()){ untested();
		message(qucs::MsgWarning, "cannot find main");
		return;
	}else if((*i)->scope()){
		load_main((*i)->scope());
	}else{ untested();
		incomplete();
		unreachable();
	}
	SET_RUN_MODE xx(rBATCH);

	trace0("================= list ============");
	command("list", _main);
	trace1("===================================", _sim_data.is_first_expand());

	try{ untested();
		command("go FILE", _main);
	}catch(Exception_No_Match const& e){ untested();
		message(qucs::MsgFatal, "Error running simulation: " + e.message());
	}

	trace0("=========stat==========================");
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
	notifyState(Simulator::sst_idle);
} // Gnucap::do_it
/* -------------------------------------------------------------------------------- */
}
/* -------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------- */
