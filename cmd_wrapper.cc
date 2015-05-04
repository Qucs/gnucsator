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
 * wrapping commands to qucs
 */
#include "globals.h"
#include "u_lang.h"
#include "c_comand.h"
#include "l_qucs.h"
#include "u_prblst.h"

using std::map;
using std::string;

namespace{
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
	}
	void TRAN_WRAP::do_it(CS&cmd, CARD_LIST* cl)
	{ untested();
		assert(cl);
		options(cmd);
		tran_t t;
		t._start = _start;
		t._stop = _stop;
		TRAN_WRAP::_stash[short_label()] = t;
	}
DISPATCHER<CMD>::INSTALL d8(&command_dispatcher, "TR", &p8);

// go. run commands that are scattered
	class GO : public CMD {
		void do_it(CS&cmd, CARD_LIST*cl)
		{
			trace0("go");
			CS p(CS::_STRING, "v(nodes)");
			PROBE_LISTS::print[s_TRAN].add_list(p);
			CMD* c = NULL;
			try { untested();
				c = command_dispatcher["transient"];
			}catch(Exception){ incomplete();
			}
			assert(c);

			for(auto&i : TRAN_WRAP::_stash){
				stringstream x;
				auto j = i.second;
				x << j._start << " " << j._stop << " " << j._stop << " trace=a basic";
				CS wcmd(CS::_STRING, x.str());
				c->do_it(wcmd, cl);
			}

			cmd.reset();
			if(cmd >> "quit"){ itested();
				quit(cl);
			}else{untested();
			}
		}

		// quit is invoked from main()
		// exit is an alias, we overload quit and call exit.
		void quit(CARD_LIST* cl)
		{
			trace0("exiting...");
			CMD* c = NULL;
			try { untested();
				c = command_dispatcher["exit"];
			}catch(Exception){ incomplete();
			}
			CS wcmd(CS::_STRING, "");
			c->do_it(wcmd, cl);
		}
	}go;
DISPATCHER<CMD>::INSTALL d9(&command_dispatcher, "quit|go", &go);
}
