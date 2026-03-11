/* Copyright (C) 2021 Felix Salfelder
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
 * sweep command and objects
 */
#include "u_parameter.h"
#include "e_card.h"
#include "c_comand.h"
#include "globals.h"
#include "l_qucs.h" // QucsGet
#include "s__out.cc"
#include "s__init.cc"

namespace{
/*--------------------------------------------------------------------------*/
enum typeT{
	tLin = 0,
	tLog = 1
};
/*--------------------------------------------------------------------------*/
class SW_WRAP : public CMD {
	double _start; // PARAMETER?
	double _stop;
	unsigned _points = 1;
	std::string _sim;
	std::string _param;
	typeT _type;
public:
	SW_WRAP(): CMD() {}
private:
	SW_WRAP(SW_WRAP const&x): CMD(x), _start(x._start),
		_stop(x._stop), _points(x._points), _sim(x._sim),
		_param(x._param), _type(x._type)	{untested();}
	CARD* clone()const override {untested(); return new SW_WRAP(*this);}
	std::string value_name()const override { untested();unreachable(); return "";}
private:

	void options(CS& cmd){
		size_t here = cmd.cursor();
		do{
			ONE_OF
			  || QucsGet(cmd, "Sim",     &_sim)
			  || QucsGet(cmd, "Start",   &_start)
			  || QucsGet(cmd, "Stop",    &_stop)
			  || QucsGet(cmd, "Param",   &_param)
			  || QucsGet(cmd, "Points",  &_points)
			  || (cmd.umatch( "Type") &&
			       (ONE_OF
			        || QucsSet(cmd, "lin",   &_type, tLin)
			        || QucsSet(cmd, "log",   &_type, tLog)
			        || cmd.warn(bWARNING, "need lin, log... incomplete")
			       )
			     )
			  ;
		}while (cmd.more() && !cmd.stuck(&here));

		assert(_points);
	}

	void do_it(CS& cmd, CARD_LIST* Scope)override {
		assert(Scope);
		trace1("SWEEP::do_it", cmd.fullstring());
		if(cmd >> "go"){
			error(bTRACE, "sweep " + cmd.fullstring() + "\n");
			assert(_sim.size());
			auto it = Scope->find_(_sim);
			if(it==Scope->end()){ untested();
				throw Exception_CS("unknown sim " + _sim, cmd);
			}else if(auto c=dynamic_cast<CMD*>(*it)){
				cmd.reset();
				std::stringstream x;
				x << cmd.fullstring() << " " << _param
				                      << " " << _start
				                      << " " << _stop
				                      << " " << (_stop-_start) / (_points - 1);
				CS xx(CS::_STRING, x.str());
				c->do_it(xx, Scope);
			}else{ untested();
				unreachable();
				assert(0);
			}
		}else{
			options(cmd);
			trace4("sweep", _sim, _param, _start, _stop);

			PARAM_LIST* pl = Scope->params();
			assert(pl);
//			pl->set(_param, _start);

#if 0
			pl->set("__omit_"+_sim, "");
			auto omit = pl->deep_lookup("__omit_"+_sim);
			assert(!omit.has_hard_value());
#endif
			pl->set("__omit_"+_sim, "1");

			auto cl = clone();
			trace2("SW", short_label(), _sim);
			assert(short_label().size());
			assert(cl->short_label() == short_label());
			Scope->push_back(cl);
		}
	}
} psw;
DISPATCHER<CMD>::INSTALL dsw(&command_dispatcher, "SW", (CMD*)&psw);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
