/* Copyright (C) 2015 Felix Salfelder
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
 * qucs commands to gnucap
 * this is essentially the gnucap driver.
 * remember: qucs simulator drivers under consruction. see qucs/
 */
#include "globals.h"
#include "u_lang.h"
#include "c_comand.h"
#include "l_qucs.h"
#include "u_prblst.h"
#include "u_sim_data.h"
#include "e_cardlist.h"

using std::string;
using std::stringstream;
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
enum typeT{
	tLin = 0,
	tLog = 1
};
/*--------------------------------------------------------------------------*/
class AC_WRAP : public CMD {
public:
	AC_WRAP(): CMD() {}
public:
	typedef struct{
		double _start;
		double _stop;
		std::string _args;
	} data_t;
private:
	double _start;
	double _stop;
	unsigned _points;
	method_t _integration;
	int _order;
	double _initialstep;
	double _dtmin;
	unsigned _itl;
	double _reltol;
	double _abstol;
	double _vntol;

	void options(CS&);
	void do_it(CS&cmd, CARD_LIST* cl) {
		assert(cl);
		options(cmd);
		data_t t;
		t._start = _start;
		t._stop = _stop;

		if(_points<2){ untested();
			incomplete();
		}else if (_type == tLin) {
			double range = _stop - _start;
			double step = range / (_points-1);
			incomplete();
			t._args = "step " + to_string(step) + " ";
		}else if (_type == tLog) {
			double range = _stop / _start;
			double step = exp ( log(range)  / (_points-1));

			// log b = log( stop / start ) / points
			t._args = "* " + to_string(step) + " ";
		}else{ untested();
			incomplete();
		}

		_stash[short_label()] = t;
	}
public: // GO
	static std::map<string, data_t> _stash;
private:
	typeT _type;
} pac;
std::map<string, AC_WRAP::data_t> AC_WRAP::_stash;
DISPATCHER<CMD>::INSTALL ddc(&command_dispatcher, "AC", &pac);
/*--------------------------------------------------------------------------*/
void AC_WRAP::options(CS& cmd)
{
	_start = 0;
	_stop = 0;
	_points = 0;
	double _whatever; // incomplete
	size_t here = cmd.cursor();
	do{
		trace1("options", cmd.tail());
		ONE_OF
			|| QucsGet(cmd, "Start", 	   &_start)
			|| QucsGet(cmd, "Stop",		   &_stop)
			|| QucsGet(cmd, "Points",	   &_points)
			|| (cmd.umatch( "Type") &&
					(ONE_OF
					 || QucsSet(cmd, "lin",   &_type, tLin)
					 || QucsSet(cmd, "log",   &_type, tLog)
					 || cmd.warn(bWARNING, "need lin, log... incomplete")
					)
				)
			;
	}while (cmd.more() && !cmd.stuck(&here));
	cmd.check(bWARNING, "what's this (incomplete)?");
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DC_WRAP : public CMD {
public:
	DC_WRAP(): CMD() {}
public:
	typedef struct{
		double _start;
		double _stop;
	} data_t;
private:
	double _start;
	double _stop;
	unsigned _points;
	method_t _integration;
	int _order;
	double _initialstep;
	double _dtmin;
	unsigned _itl;
	double _reltol;
	double _abstol;
	double _vntol;

//	void options(CS&);
	void do_it(CS&cmd, CARD_LIST* cl) {
		assert(cl);
//		options(cmd);
		data_t t;
		t._start = _start;
		t._stop = _stop;
		_stash[short_label()] = t;
	}
public: // GO
	static std::map<string, data_t> _stash;
} pdc;
std::map<string, DC_WRAP::data_t> DC_WRAP::_stash;
DISPATCHER<CMD>::INSTALL dac(&command_dispatcher, "DC", &pdc);
/*--------------------------------------------------------------------------*/
class SP_WRAP : public CMD {
public:
	SP_WRAP(): CMD() {}
public:
	typedef struct{
		double _start;
		double _stop;
		std::string _args;
	} data_t;
private:
	double _start;
	double _stop;
	unsigned _points;
	method_t _integration;
	int _order;
	double _initialstep;
	double _dtmin;
	unsigned _itl;
	double _reltol;
	double _abstol;
	double _vntol;

	void options(CS&);
	void do_it(CS&cmd, CARD_LIST* cl) {
		assert(cl);
		options(cmd);
		data_t t;
		t._start = _start;
		t._stop = _stop;

		double range = _stop - _start;
		if (_type == tLin) {
			assert(_points>1);
			double step = range / (_points-1);
			incomplete();
			t._args = "step " + to_string(step) + " ";
		}else if (_type == tLog) { untested();
			double step = log(range / (_points-1));
			t._args = "* " + to_string(step) + " ";
		}else{ untested();
			incomplete();
		}

		_stash[short_label()] = t;
	}
public: // GO
	static std::map<string, data_t> _stash;
private:
	typeT _type;
} psp;
std::map<string, SP_WRAP::data_t> SP_WRAP::_stash;
DISPATCHER<CMD>::INSTALL dsp(&command_dispatcher, "SP", &psp);
/*--------------------------------------------------------------------------*/
void SP_WRAP::options(CS& cmd)
{
	_order = -1;
	_points = 0;
	_dtmin = 0.;
	_itl = 0;
	_reltol = -1.;
	_abstol = -1.;
	_vntol = -1.;
	size_t here = cmd.cursor();

	// .SP:SP1 Type="lin" Start="1" Stop="2" Points="3" Noise="no" NoiseIP="1" NoiseOP="2" saveCVs="no" saveAll="no"
	do{
		trace1("options", cmd.tail());
		ONE_OF
			|| QucsGet(cmd, "Start", 	   &_start)
			|| QucsGet(cmd, "Stop",		   &_stop)
			|| QucsGet(cmd, "Points",	   &_points)
			|| (cmd.umatch( "Type") &&
					(ONE_OF
					 || QucsSet(cmd, "lin",   &_type, tLin)
					 || QucsSet(cmd, "log",   &_type, tLog)
					 || cmd.warn(bWARNING, "need lin, log... incomplete")
					)
				)
			;
	}while (cmd.more() && !cmd.stuck(&here));
	cmd.check(bWARNING, "what's this (incomplete)?");
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class TRAN_WRAP : public CMD {
public:
	TRAN_WRAP(): CMD() {}
public:
	typedef struct{
		double _start;
		double _stop;
	} tran_t;
	static std::map<string, tran_t> _stash;
private:
	double _start;
	double _stop;
	unsigned _points;
	method_t _integration;
	int _order;
	double _initialstep;
	double _dtmin;
	unsigned _itl;
	double _reltol;
	double _abstol;
	double _vntol;

	void options(CS&);
	void do_it(CS&cmd, CARD_LIST* cl){
		trace1("tran_wrap", cmd.fullstring());
		assert(cl);
		options(cmd);
		tran_t t;
		t._start = _start;
		t._stop = _stop;
		TRAN_WRAP::_stash[short_label()] = t;
	}

} p8;
std::map<string, TRAN_WRAP::tran_t> TRAN_WRAP::_stash;
/*--------------------------------------------------------------------------*/
void TRAN_WRAP::options(CS& cmd)
{
	_order = -1;
	_points = 0;
	_dtmin = 0.;
	_itl = 0;
	_reltol = -1.;
	_abstol = -1.;
	_vntol = -1.;
	double _whatever; // incomplete
	string type; // incomplete();
	size_t here = cmd.cursor();
	do{
		trace1("tran_wrap options", cmd.tail());
		ONE_OF
			|| QucsGet(cmd, "Start", 	   &_start)
			|| QucsGet(cmd, "Stop",		   &_stop)
			|| QucsGet(cmd, "Points",	   &_points)
			|| QucsGet(cmd, "Type",	   &type)
			|| (cmd.umatch( "IntegrationMethod") &&
					(ONE_OF
					 || QucsSet(cmd, "Trapezoidal",   &_integration, meTRAP)
					 || QucsSet(cmd, "Euler",         &_integration, meEULER)
					 || cmd.warn(bWARNING, "need Trapezoidal, Euler ... incomplete")
					)
				)
			|| QucsGet(cmd, "Order",      &_order)
			|| QucsGet(cmd, "InitialStep",&_initialstep)
			|| QucsGet(cmd, "MinStep",    &_dtmin)
			|| QucsGet(cmd, "MaxIter",    &_itl)
			|| QucsGet(cmd, "reltol",     &_reltol)
			|| QucsGet(cmd, "abstol",     &_abstol)
			|| QucsGet(cmd, "vntol",      &_vntol)
			|| QucsGet(cmd, "Temp",       &_whatever)
			|| QucsGet(cmd, "LTEreltol",  &_whatever)
			|| QucsGet(cmd, "LTEabstol",  &_whatever)
			|| QucsGet(cmd, "LTEfactor",  &_whatever)
			|| QucsGet(cmd, "Solver",     &_whatever)
			|| QucsGet(cmd, "relaxTSR",   &_whatever)
			|| QucsGet(cmd, "initialDC",  &_whatever)
			|| QucsGet(cmd, "MaxStep",    &_whatever)
			;
	}while (cmd.more() && !cmd.stuck(&here));
	cmd.check(bWARNING, "what's this (incomplete)?");
} // TRAN_WRAP::options
/*--------------------------------------------------------------------------*/
DISPATCHER<CMD>::INSTALL d8(&command_dispatcher, "TR", &p8);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// go. run commands that are scattered
// BUG // that's what 'end' is supposed to do.
	class GO : public CMD {
		void do_it(CS&cmd, CARD_LIST*cl) {
			trace2("GO", _sim->is_first_expand(), _sim);
			assert(cl);
			cmd >> _outfile;
			// std::string tail=cmd.tail();
			CMD::command("print tran +v(nodes)", cl);
			CMD::command("print op v(nodes)", cl);
			CMD::command("print ac vr(nodes) vi(nodes)", cl);
			CMD* c = NULL;
			CMD* s = NULL;
			CMD* o = NULL;
			CMD* a = NULL;
			try {
				c = command_dispatcher["transient"];
				s = command_dispatcher["sp"];
				o = command_dispatcher["op"];
				a = command_dispatcher["ac"];
			}catch(Exception const&){ untested();
				error(bDANGER, "some commands are missing, load plugin?\n");
				exit(1);
			}

			if(!c){ untested();
				error(bDANGER, "transient command missing, load plugin?\n");
				exit(1);
			}else{
			}

			trace2("GO2", _sim->is_first_expand(), _sim);

			// what happens if there are multiple trans?
			for(auto const&i : TRAN_WRAP::_stash){
				stringstream x;
				auto j = i.second;
				x << j._start << " " << j._stop << " " << j._stop
				  << " trace=a basic > " << _outfile << ".tr";
				trace1("tran_wrap", x.str());
				CS wcmd(CS::_STRING, x.str());
				c->do_it(wcmd, cl);
			}
			if(TRAN_WRAP::_stash.empty()){ untested();
				CS wcmd(CS::_STRING, " >/dev/null");
				o->do_it(wcmd, cl);
			}else{
			}
			for(auto const&i : DC_WRAP::_stash){ untested();
				incomplete();
				(void) i;
				stringstream x;
				x << " trace=n basic > " << _outfile << ".dc";
				CS wcmd(CS::_STRING, x.str());
				o->do_it(wcmd, cl);
			}
			for(auto const&i : AC_WRAP::_stash){ untested();
				stringstream x;
				auto j = i.second;
				x << j._start << " " << j._stop << " ";
				x << j._args;
					
				x << " basic > " << _outfile << ".ac";
				CS wcmd(CS::_STRING, x.str());
				trace1("run ac", wcmd.fullstring());
				a->do_it(wcmd, cl);
			}
			for(auto&i : SP_WRAP::_stash){ untested();
				stringstream x;
				auto j = i.second;
				x << "port * " << j._start << " " << j._stop << " " <<  j._args
				  << " > " << _outfile << ".sp";
				trace1("SP_WRAP running", x.str());
				CS wcmd(CS::_STRING, x.str());
				s->do_it(wcmd, cl);
			}

			cmd.reset();
#if 0 // probably not a good idea.
			if(cmd >> "quit"){ itested();
				quit(cl);
			}else{untested();
			}
#endif
		}

		// quit is invoked from main()
		// exit is an alias, we overload quit and call exit.
		void quit(CARD_LIST* cl)
		{ untested();
			trace0("exiting...");
			CMD* c = NULL;
			try { untested();
				c = command_dispatcher["exit"];
			}catch(Exception const&){ incomplete();
			}
			CS wcmd(CS::_STRING, "");
			c->do_it(wcmd, cl);
		}
	private:
		std::string _outfile;
	}go;
DISPATCHER<CMD>::INSTALL d9(&command_dispatcher, "go", &go);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
