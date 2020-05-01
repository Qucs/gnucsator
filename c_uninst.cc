/*                            -*- C++ -*-
 * Copyright (C) 2018 Felix Salfelder
 * Author: Felix
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
#define NDEBUG
#include <globals.h>
#include <c_comand.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class CMD_UNINST : public CMD {
public:
	~CMD_UNINST(){
		for(auto& d : _l){
			delete d;
			d = NULL;
		}
		_l.resize(0);
	}
public:
	void do_it(CS& cmd, CARD_LIST*) {
		DISPATCHER<CARD>* d=NULL;
		if (cmd.umatch("command")) {
			d = (DISPATCHER<CARD>*) &command_dispatcher;
		}else if(cmd.umatch("status")){
			d = (DISPATCHER<CARD>*) &status_dispatcher;
		}else if(cmd.umatch("lang{uage}")){
			d = (DISPATCHER<CARD>*) &language_dispatcher;
		}else if(cmd.umatch("comp{onent}")){
			d = (DISPATCHER<CARD>*) &device_dispatcher;
		}else{
			incomplete();
			return;
		}
		std::string what;
		while(cmd.more()){
			cmd >> what;
			int prev_picky = OPT::picky;
			OPT::picky = bDANGER;
			DISPATCHER<CARD>::INSTALL* di =
				new DISPATCHER<CARD>::INSTALL(d, what, NULL);
			OPT::picky = prev_picky;
			_l.push_back(di);
		}
	}
private:
	std::vector<DISPATCHER<CARD>::INSTALL*> _l;
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "uninst|uninstall", &p1);
/*--------------------------------------------------------------------------*/
}
