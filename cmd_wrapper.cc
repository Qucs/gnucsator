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
 * remember: qucs does not support simulator drivers (yet?!).
 */
#include "globals.h"
#include "u_lang.h"
#include "c_comand.h"
#include "l_qucs.h"
#include "u_prblst.h"
#include "e_cardlist.h"

using std::map;
using std::string;
using std::stringstream;
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class SP_WRAP : public CMD {
public:
	SP_WRAP(): CMD() {untested();}
public:
	typedef struct{
		double _start;
		double _stop;
	} data_t;
private: // types
	enum typeT{
		tLin = 0,
		tLog = 1
	};
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
	void do_it(CS&cmd, CARD_LIST* cl);
public: // GO
	static std::map<string, data_t> _stash;
private:
	typeT _type;
} psp;
map<string, SP_WRAP::data_t> SP_WRAP::_stash;
DISPATCHER<CMD>::INSTALL dsp(&command_dispatcher, "SP", &psp);
/*--------------------------------------------------------------------------*/
void SP_WRAP::do_it(CS&cmd, CARD_LIST* cl)
{
	assert(cl);
	options(cmd);
	data_t t;
	t._start = _start;
	t._stop = _stop;
	_stash[short_label()] = t;
}
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
	double _whatever; // incomplete
	unsigned here = cmd.cursor();

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
			TRAN_WRAP(): CMD() {untested();}
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
			void do_it(CS&cmd, CARD_LIST* cl);

	} p8;
	map<string, TRAN_WRAP::tran_t> TRAN_WRAP::_stash;
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
		unsigned here = cmd.cursor();
		do{
			trace1("options", cmd.tail());
			ONE_OF
				|| QucsGet(cmd, "Start", 	   &_start)
				|| QucsGet(cmd, "Stop",		   &_stop)
				|| QucsGet(cmd, "Points",	   &_points)
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
				|| QucsGet(cmd, "Type",       &_whatever)
				;
		}while (cmd.more() && !cmd.stuck(&here));
		cmd.check(bWARNING, "what's this (incomplete)?");
	} // TRAN_WRAP::options
/*--------------------------------------------------------------------------*/
	void TRAN_WRAP::do_it(CS&cmd, CARD_LIST* cl)
	{
		assert(cl);
		options(cmd);
		tran_t t;
		t._start = _start;
		t._stop = _stop;
		TRAN_WRAP::_stash[short_label()] = t;
	}
DISPATCHER<CMD>::INSTALL d8(&command_dispatcher, "TR", &p8);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// go. run commands that are scattered
	class GO : public CMD {
		void do_it(CS&cmd, CARD_LIST*cl)
		{
			cmd.skip1b("go");
			trace1("go", cmd.tail());
			// std::string tail=cmd.tail();
			CMD::command("print tran +v(nodes)", &CARD_LIST::card_list);
			CMD::command("print tran -v(gnd)", &CARD_LIST::card_list);
			CMD* c = NULL;
			CMD* s = NULL;
			CMD* o = NULL;
			try {
				c = command_dispatcher["transient"];
				s = command_dispatcher["sp"];
				o = command_dispatcher["op"];
			}catch(Exception){ untested();
				error(bDanger, "some commands are missing, load plugin?\n");
				exit(1);
			}
			assert(c);

			// what happens if there are multiple trans?
			for(auto const&i : TRAN_WRAP::_stash){
				stringstream x;
				auto j = i.second;
				x << j._start << " " << j._stop << " " << j._stop << " trace=a basic"; // tail, redirect?
				CS wcmd(CS::_STRING, x.str());
				c->do_it(wcmd, cl);
			}
			if(TRAN_WRAP::_stash.empty()){
				CS wcmd(CS::_STRING, " >/dev/null");
				o->do_it(wcmd, cl);
			}
			for(auto&i : SP_WRAP::_stash){
				stringstream x;
				auto j = i.second;
				x << "port * " << j._start << " " << j._stop;
				trace1("running", x.str());
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
		{
			trace0("exiting...");
			CMD* c = NULL;
			try {
				c = command_dispatcher["exit"];
			}catch(Exception){ incomplete();
			}
			CS wcmd(CS::_STRING, "");
			c->do_it(wcmd, cl);
		}
	}go;
DISPATCHER<CMD>::INSTALL d9(&command_dispatcher, "go", &go);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
