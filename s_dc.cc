/*$Id: s_dc.cc 2016/09/22 al $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
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
 *------------------------------------------------------------------
 * dc analysis top
 */
#include "globals.h"
#include "u_status.h"
#include "u_prblst.h"
#include "u_cardst.h"
#include "e_elemnt.h"
#include "e_subckt.h"
#include "s__.h"
#include "s_dc_out.cc"
#include "s__init.cc"
#include "s__solve.cc"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DCOP : public SIM {
public:
  void	finish();
protected:
  void	fix_args(int);
  void	options(CS&, int);
private:
  void	sweep();
  void	dc_eval();
  void	sweep_recursive(int);
  void	first(int);
  bool	next(int);
  explicit DCOP(const DCOP&): SIM() {unreachable(); incomplete();}
protected:
  void set_sweepval(int i, double d){
    assert(_sweepval[i]);
    std::string n = _param_name[i];
    if(n!=""){
      PARAM_LIST* pl = _scope->params();
      assert(pl);
      CS cmd(CS::_STRING, n + "=" + std::to_string(d));
      trace4("sweep", i, d, n, cmd.fullstring());
      pl->parse(cmd);
    }else{
      // legacy
    }
    *_sweepval[i] = d;
  }
  double get_sweepval(int i) const{
    assert(_sweepval[i]);
    return *_sweepval[i];
  }
protected:
  explicit DCOP();
  ~DCOP() {}
  
protected:
  enum {DCNEST = 4};
  int _n_sweeps;
  PARAMETER<double> _start[DCNEST];
  PARAMETER<double> _stop[DCNEST];
  PARAMETER<double> _step_in[DCNEST];
  double _step[DCNEST];
  bool _linswp[DCNEST];
  double* _sweepval[DCNEST];	/* pointer to thing to sweep, dc command */
  ELEMENT* _zap[DCNEST];	/* to branch to zap, for re-expand */
  std::string _param_name[DCNEST];
  PARAMETER<double> _param[DCNEST]; // sweep this value: TODO: use double.
  PARAMETER<double> _param_zap[DCNEST]; // keep a backup
  CARDSTASH _stash[DCNEST];	/* store std values of elements being swept */
  bool _loop[DCNEST];		/* flag: do it again backwards */
  bool _reverse_in[DCNEST];	/* flag: sweep backwards, input */
  bool _reverse[DCNEST];	/* flag: sweep backwards, working */
  bool _cont;			/* flag: continue from previous run */
  TRACE _trace;			/* enum: show extended diagnostics */
  enum {ONE_PT, LIN_STEP, LIN_PTS, TIMES, OCTAVE, DECADE} _stepmode[DCNEST];
};
/*--------------------------------------------------------------------------*/
class DC : public DCOP {
public:
  explicit DC(): DCOP() {}
  ~DC() {}
  void	do_it(CS&, CARD_LIST*);
private:
  void	setup(CS&);
  explicit DC(const DC&): DCOP() {unreachable(); incomplete();}
};
/*--------------------------------------------------------------------------*/
class OP : public DCOP {
public:
  explicit OP(): DCOP() {}
  ~OP() {}
  void	do_it(CS&, CARD_LIST*);
private:
  void	setup(CS&);
  explicit OP(const OP&): DCOP() {unreachable(); incomplete();}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DC::do_it(CS& Cmd, CARD_LIST* Scope)
{
  assert(Scope);
  if (Scope == &CARD_LIST::card_list) {
  }else{untested();
  }
  _scope = Scope;
  _sim->_time0 = 0.;
  trace0("set_cmd_dc");
  _sim->set_command_dc();
  _sim->_phase = p_INIT_DC;
  ::status.dc.reset().start();
  command_base(Cmd);
  finish_hack(this);
  _sim->_has_op = s_DC;
  _scope = NULL;
  ::status.dc.stop();
}
/*--------------------------------------------------------------------------*/
void OP::do_it(CS& Cmd, CARD_LIST* Scope)
{
  assert(Scope);
  if (Scope == &CARD_LIST::card_list) {
  }else{untested();
  }
  _scope = Scope;
  _sim->_time0 = 0.;
  _sim->set_command_op();
  _sim->_phase = p_INIT_DC;
  ::status.op.reset().start();
  command_base(Cmd);
  _sim->_has_op = s_OP;
  _scope = NULL;
  ::status.op.stop();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DCOP::DCOP()
  :SIM(),
   _n_sweeps(1),
   _cont(false),
   _trace(tNONE)
{
  for (int ii = 0; ii < DCNEST; ++ii) {
    _loop[ii] = false;
    _reverse_in[ii] = false;
    _reverse[ii] = false;
    _step[ii]=0.;
    _linswp[ii]=true;
    _sweepval[ii]=&_sim->_genout;
    _zap[ii]=NULL; 
    _stepmode[ii] = ONE_PT;
    _param[ii] = "";
  }
  
  //BUG// in SIM.  should be initialized there.
  //_sim->_genout=0.;
  _out=IO::mstdout;
  //_sim->_uic=false;
}
/*--------------------------------------------------------------------------*/
static void set_probes_recursive(CARD_LIST* s, int c)
{
  assert(s);
  for (CARD_LIST::iterator ci=s->begin(); ci!=s->end(); ++ci) {
    BASE_SUBCKT* b = dynamic_cast<BASE_SUBCKT*>(*ci);

    if(!b){
    }else if(CARD_LIST* ss = (*ci)->subckt()){
      set_probes_recursive(ss, c);
    }else{ untested();
    }
    if(c==1){
      (*ci)->inc_probes();
    }else if(c==-1){
      (*ci)->dec_probes();
    }else{
      unreachable();
    }
  }
}
/*--------------------------------------------------------------------------*/
void DCOP::finish(void)
{
  bool have_params = false;
  for (int ii = 0;  ii < _n_sweeps;  ++ii) {
    std::string n = _param_name[ii];
    if (_zap[ii]) { // component
      _stash[ii].restore();
      _zap[ii]->dec_probes();
      _zap[ii]->precalc_first();
      _zap[ii]->precalc_last();
      _zap[ii] = NULL;
    }else if (n != "") {
      PARAM_LIST* pl = _scope->params();
      assert(pl);
      CS cmd(CS::_STRING, n + "=" + _param_zap[ii].string());
      trace3("reset", ii, n, cmd.fullstring());
      pl->parse(cmd);
      have_params = true;
    }
  }
  if(have_params){
    set_probes_recursive(_scope, -1);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void OP::setup(CS& Cmd)
{
  _sim->_temp_c = OPT::temp_c;
  _cont = false;
  _trace = tNONE;
  _out = IO::mstdout;
  _out.reset(); //BUG// don't know why this is needed */
  bool ploton = IO::plotset  &&  plotlist().size() > 0;

  _zap[0] = NULL;
  _sweepval[0] = &(_sim->_temp_c);

  if (Cmd.match1("'\"({") || Cmd.is_float()) { untested();
    Cmd >> _start[0];
    if (Cmd.match1("'\"({") || Cmd.is_float()) { untested();
      Cmd >> _stop[0];
    }else{ untested();
      _stop[0] = _start[0];
    }
  }else{
  }
  
  _step[0] = 0.;
  _sim->_genout = 0.;

  options(Cmd,0);

  _n_sweeps = 1;
  Cmd.check(bWARNING, "what's this?");
  _sim->_freq = 0;

  IO::plotout = (ploton) ? IO::mstdout : OMSTREAM();
  initio(_out);

  _start[0].e_val(OPT::temp_c, _scope);
  fix_args(0);
}
/*--------------------------------------------------------------------------*/
static ELEMENT* find_sweep(CARD* c)
{
  assert(c);
  if (ELEMENT* e = dynamic_cast<ELEMENT*>(c)) { untested();
    return e;
  }else if(c->subckt()){
    auto d = c->subckt()->find_("dev");
    if (d==c->subckt()->end()) { untested();
    }else if(ELEMENT* e = dynamic_cast<ELEMENT*>(*d)) {
      return e;
    }else{
    }
  }else{
  }
  throw Exception("dc/op: can't sweep " + c->long_label() + '\n');
}
/*--------------------------------------------------------------------------*/
void DC::setup(CS& Cmd)
{
  _sim->_temp_c = OPT::temp_c;
  _cont = false;
  _trace = tNONE;
  _out = IO::mstdout;
  _out.reset(); //BUG// don't know why this is needed */
  bool ploton = IO::plotset  &&  plotlist().size() > 0;

  if (Cmd.more()) {
    for (_n_sweeps = 0; Cmd.more() && _n_sweeps < DCNEST; ++_n_sweeps) {
      CARD_LIST::fat_iterator ci = findbranch(Cmd, _scope);
      if (!ci.is_end()) {			// sweep a component
	_zap[_n_sweeps] = find_sweep(*ci);
      }else if (Cmd.is_float()) {		// sweep the generator
	_zap[_n_sweeps] = NULL;
      }else if (Cmd.is_alpha()) {
	std::string pname;

	size_t here = Cmd.cursor();
        Cmd >> pname;
	PARAMETER<double> zap = _scope->params()->deep_lookup(pname);
	if(zap.has_hard_value()){
	  trace2("backup", pname, zap);
	  _param_zap[_n_sweeps] = zap;
	  _param_name[_n_sweeps] = pname;
	}else{
	  // throw Exception("dc/op: can't sweep " + pname + '\n');
	  Cmd.reset(here);
	  // Cmd.check(bWARNING, "what's this?");
	}

      }else{
	// leave as it was .. repeat Cmd with no args
      }
      
      if (Cmd.match1("'\"({") || Cmd.is_float()) {	// set up parameters
	_start[_n_sweeps] = "NA";
	_stop[_n_sweeps] = "NA";
	Cmd >> _start[_n_sweeps] >> _stop[_n_sweeps];
	_step[_n_sweeps] = 0.;
      }else{
	// leave it as it was .. repeat Cmd with no args
      }
      
      _sim->_genout = 0.;
      options(Cmd,_n_sweeps);
    }
  }else{
  }
  Cmd.check(bWARNING, "what's this?");

  IO::plotout = (ploton) ? IO::mstdout : OMSTREAM();
  initio(_out);

  bool have_params = false;

  assert(_n_sweeps > 0);
  for (int ii = 0;  ii < _n_sweeps;  ++ii) {
    _start[ii].e_val(0., _scope);
    fix_args(ii);

    if (_zap[ii]) { // component
      _stash[ii] = _zap[ii];			// stash the std value
      _zap[ii]->inc_probes();			// we need to keep track of it
      _zap[ii]->set_value(_zap[ii]->value(),0);	// zap out extensions
      _zap[ii]->set_constant(false);		// so it will be updated
      _sweepval[ii] = _zap[ii]->set__value();	// point to value to patch
    }else if (_param_name[ii] != "") {
      _sweepval[ii] = _param[ii].pointer_hack();
      have_params = true;
    }else{ // generator
      _sweepval[ii] = &_sim->_genout;			// point to value to patch
    }
  }

  if(have_params){
    set_probes_recursive(_scope, 1);
  }else{
  }

  _sim->_freq = 0;
}
/*--------------------------------------------------------------------------*/
void DCOP::fix_args(int Nest)
{
  _stop[Nest].e_val(_start[Nest], _scope);
  _step_in[Nest].e_val(0., _scope);
  _step[Nest] = _step_in[Nest];
  
  switch (_stepmode[Nest]) {
  case ONE_PT:
  case LIN_STEP:
    _linswp[Nest] = true;
    break;
  case LIN_PTS:untested();
    if (_step[Nest] <= 2.) {untested();
      _step[Nest] = 2.;
    }else{untested();
    }
    _linswp[Nest] = true;
    break;
  case TIMES:untested();
    if (_step[Nest] == 0.  &&  _start[Nest] != 0.) {untested();
      _step[Nest] = _stop[Nest] / _start[Nest];
    }else{untested();
    }
    _linswp[Nest] = false;
    break;
  case OCTAVE:untested();
    if (_step[Nest] == 0.) {untested();
      _step[Nest] = 1.;
    }else{untested();
    }
    _step[Nest] = pow(2.00000001, 1./_step[Nest]);
    _linswp[Nest] = false;
    break;
  case DECADE:
    if (_step[Nest] == 0.) {untested();
      _step[Nest] = 1.;
    }else{ untested();
    }
    _step[Nest] = pow(10., 1./_step[Nest]);
    _linswp[Nest] = false;
    break;
  };
  
  if (_step[Nest] == 0.) {	// prohibit log sweep from 0
    _step[Nest] = _stop[Nest] - _start[Nest];
    _linswp[Nest] = true;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DCOP::options(CS& Cmd, int Nest)
{
  _sim->_uic = _loop[Nest] = _reverse_in[Nest] = false;
  size_t here = Cmd.cursor();
  do{
    ONE_OF
      || (Cmd.match1("'\"({")	&& ((Cmd >> _step_in[Nest]), (_stepmode[Nest] = LIN_STEP)))
      || (Cmd.is_float()	&& ((Cmd >> _step_in[Nest]), (_stepmode[Nest] = LIN_STEP)))
      || (Get(Cmd, "*",		  &_step_in[Nest]) && (_stepmode[Nest] = TIMES))
      || (Get(Cmd, "+",		  &_step_in[Nest]) && (_stepmode[Nest] = LIN_STEP))
      || (Get(Cmd, "by",	  &_step_in[Nest]) && (_stepmode[Nest] = LIN_STEP))
      || (Get(Cmd, "step",	  &_step_in[Nest]) && (_stepmode[Nest] = LIN_STEP))
      || (Get(Cmd, "d{ecade}",	  &_step_in[Nest]) && (_stepmode[Nest] = DECADE))
      || (Get(Cmd, "ti{mes}",	  &_step_in[Nest]) && (_stepmode[Nest] = TIMES))
      || (Get(Cmd, "lin",	  &_step_in[Nest]) && (_stepmode[Nest] = LIN_PTS))
      || (Get(Cmd, "o{ctave}",	  &_step_in[Nest]) && (_stepmode[Nest] = OCTAVE))
      || Get(Cmd, "c{ontinue}",   &_cont)
      || Get(Cmd, "dt{emp}",	  &(_sim->_temp_c),   mOFFSET, OPT::temp_c)
      || Get(Cmd, "lo{op}", 	  &_loop[Nest])
      || Get(Cmd, "re{verse}",	  &_reverse_in[Nest])
      || Get(Cmd, "te{mperature}",&(_sim->_temp_c))
      || (Cmd.umatch("tr{ace} {=}") &&
	  (ONE_OF
	   || Set(Cmd, "n{one}",      &_trace, tNONE)
	   || Set(Cmd, "o{ff}",       &_trace, tNONE)
	   || Set(Cmd, "w{arnings}",  &_trace, tUNDER)
	   || Set(Cmd, "i{terations}",&_trace, tITERATION)
	   || Set(Cmd, "v{erbose}",   &_trace, tVERBOSE)
	   || Cmd.warn(bWARNING, 
		       "need none, off, warnings, iterations, verbose")
	   )
	  )
      || outset(Cmd,&_out)
      ;
  }while (Cmd.more() && !Cmd.stuck(&here));
}
/*--------------------------------------------------------------------------*/
void DCOP::sweep()
{
  init_hack(this);
//  _out << "===DC===;\n";
  for(int i=0; i<_n_sweeps; ++i){
    std::string label="swp"+std::to_string(i);
    if (!_zap[i]) {
    }else if (_zap[i]->short_label()=="dev") {
      label = _zap[i]->owner()->long_label();
    }else{
      label = _zap[i]->long_label();
    }
    head(_start[i], _stop[i], label);
  }
  _sim->_bypass_ok = false;
  _sim->set_inc_mode_bad();
  if (_cont) {untested();
    _sim->restore_voltages();
    _scope->tr_restore();
  }else{
    _sim->clear_limit();
    _scope->tr_begin();
  }
  sweep_recursive(_n_sweeps);
}
/*--------------------------------------------------------------------------*/
static void set_mutable_recursive(CARD_LIST* s)
{
  assert(s);
  for (CARD_LIST::iterator ci=s->begin(); ci!=s->end(); ++ci) {
    BASE_SUBCKT* b = dynamic_cast<BASE_SUBCKT*>(*ci);

    if(!b){
    }else if(CARD_LIST* ss = (*ci)->subckt()){
      set_mutable_recursive(ss);
    }else{ untested();
    }
    (*ci)->set_constant(false);

  }
}
/*--------------------------------------------------------------------------*/
void DCOP::dc_eval()
{
  CARD_LIST* s = _scope;
  s->precalc_last();

  bool have_params = false;

  for (int ii = 0;  ii < _n_sweeps;  ++ii) {
    if (_zap[ii]) { // component
      _zap[ii]->set_constant(false);
    }else if(_param_name[ii]!=""){
      have_params = true;
    }
  }

  // any of them might use sweep parameters
  if(have_params){
    set_mutable_recursive(s);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DCOP::sweep_recursive(int Nest)
{
  --Nest;
  assert(Nest >= 0);
  assert(Nest < DCNEST);

  OPT::ITL itl = OPT::DCBIAS;
  
  first(Nest);
  do {
    if (Nest == 0) {
      dc_eval();
      int converged = solve_with_homotopy(itl,_trace);
      if (!converged) {untested();
	error(bWARNING, "did not converge\n");
      }else{
      }
      ::status.accept.start();
      _sim->set_limit();
      _scope->tr_accept();
      ::status.accept.stop();
      _sim->_has_op = _sim->_mode;
      // outdata(*_sweepval[Nest], ofPRINT | ofSTORE | ofKEEP);
      {
	outdata(*_sweepval[Nest], ofPRINT);
      }
      if(0)
      { untested();
	trace1("qucs output", _sim->_mode);
//	qucsator style output hack
	for(auto i: printlist()){
	  _out << "<indep " << i.object()->short_label() <<
	        "." << char(toupper(i.label().c_str()[0])) << " 1>\n";
	  _out << i.value() << "\n"
	    << "</indep>\n";
	}
      }

      itl = OPT::DCXFER;
    }else{
      outdata(*_sweepval[Nest], ofKEY);
      sweep_recursive(Nest);
    }
  } while (next(Nest));
}
/*--------------------------------------------------------------------------*/
void DCOP::first(int Nest)
{
  assert(Nest >= 0);
  assert(Nest < DCNEST);
  assert(_start);
  assert(_sweepval);
  assert(_sweepval[Nest]);

  if (ELEMENT* c = dynamic_cast<ELEMENT*>(_zap[Nest])) {
    c->set_constant(false); 
    // because of extra precalc_last could set constant to true
    // will be obsolete, once pointer hack is fixed
  }else{
    // not needed if not sweeping an element
  }

  set_sweepval(Nest, _start[Nest]);
  _reverse[Nest] = false;
  if (_reverse_in[Nest]) {untested();
    while (next(Nest)) {untested();
      /* nothing */;
    }
    _reverse[Nest] = true;
    next(Nest);
  }else{
  }
  _sim->_phase = p_INIT_DC;
}
/*--------------------------------------------------------------------------*/
bool DCOP::next(int Nest)
{
  trace3("DCOP::next", _start[Nest], _stop[Nest], _step[Nest]);
  double sweepval = NOT_VALID;
  bool ok = false;

  if (_linswp[Nest]) {
    double fudge = _step[Nest] / 10.;
    if (_step[Nest] == 0.) {
      // not stepping
      assert(!ok);
      assert(sweepval == NOT_VALID);
    }else{
      // stepping
      if (!_reverse[Nest]) {
	sweepval = *(_sweepval[Nest]) + _step[Nest];
	fixzero(&sweepval, _step[Nest]);
	ok = in_order(_start[Nest]-fudge, sweepval, _stop[Nest]+fudge);
	if (!ok  &&  _loop[Nest]) { untested();
	  // turn around
	  _reverse[Nest] = true;
	}else{
	  // forward
	}
      }else{ untested();
	assert(_reverse[Nest]);
	assert(!ok);
	assert(sweepval == NOT_VALID);
      }
      if (_reverse[Nest]) { untested();
	assert(!ok);
	//assert(sweepval == NOT_VALID);
	sweepval = *(_sweepval[Nest]) - _step[Nest];
	fixzero(&sweepval, _step[Nest]);
	ok = in_order(_start[Nest]-fudge, sweepval, _stop[Nest]+fudge);
      }else{
	// not sure of status
      }
    }
  }else{
    // not linswp
    double fudge = pow(_step[Nest], .1);
    if (_step[Nest] == 1.) {untested();
      // not stepping
      assert(!ok);
      assert(sweepval == NOT_VALID);
    }else{
      if (!_reverse[Nest]) {
	sweepval = get_sweepval(Nest) * _step[Nest];
	ok = in_order(_start[Nest]/fudge, sweepval, _stop[Nest]*fudge);
	if (!ok  &&  _loop[Nest]) {untested();
	  // turn around
	  _reverse[Nest] = true;
	}else{
	  // forward
	}
      }else{untested();
	assert(_reverse[Nest]);
	assert(!ok);
	assert(sweepval == NOT_VALID);
      }
      if (_reverse[Nest]) {untested();
	assert(!ok);
	assert(sweepval == NOT_VALID);
	sweepval = get_sweepval(Nest) / _step[Nest];
	ok = in_order(_start[Nest]/fudge, sweepval, _stop[Nest]*fudge);
      }else{
	// not sure of status
      }
    }
  }
  _sim->_phase = p_DC_SWEEP;
  if (ok) {
    assert(sweepval != NOT_VALID);
    set_sweepval(Nest, sweepval);
    return true;
  }else{
    //assert(sweepval == NOT_VALID);
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static DC p2;
static OP p4;
static DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "dc", &p2);
static DISPATCHER<CMD>::INSTALL d4(&command_dispatcher, "op", &p4);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
