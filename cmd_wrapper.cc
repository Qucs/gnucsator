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

namespace{
	class TRAN_WRAP : public CMD {
		public:
			TRAN_WRAP(): CMD() {untested();}
		private:
			double _start;
			double _stop;
			unsigned _points;
			method_t _integration;
			double _initialstep;

			void options(CS&);
			void do_it(CS&cmd, CARD_LIST* cl);

	} p8;

	void TRAN_WRAP::options(CS& cmd)
	{
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
				|| QucsGet(cmd, "InitialStep",&_initialstep)
				;
		}while (cmd.more() && !cmd.stuck(&here));
		cmd.check(bWARNING, "what's this (incomplete)?");
	}
	void TRAN_WRAP::do_it(CS&cmd, CARD_LIST* cl)
	{ untested();
		CMD* c = NULL;
		try { untested();
			c = command_dispatcher["transient"];
		}catch(Exception){ incomplete();
		}
		assert(c);
		options(cmd);
		stringstream x;
		x << _start << " " << _stop << " " << _stop << " trace=a";
		trace1("assembled arglist", x.str());
		CS wcmd(CS::_STRING, x.str());
		c->do_it(wcmd, cl);
	}
DISPATCHER<CMD>::INSTALL d8(&command_dispatcher, "TR", &p8);
}
