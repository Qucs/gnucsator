/*
 * Copyright (C) 2017 Felix Salfelder
 * Author: Felix Salfelder <felix@salfelder.org>
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
 */
#include "u_lang.h"
#include "c_comand.h"
#include "e_cardlist.h"
#include "e_model.h"
#include "e_subckt.h"
#include "globals.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
// this is necessary, as subcircuit models are not stored in COMMON yet.
class HACK_CARDLIST{
  void* _parent;
  void* _nm;
  void* _params;
  std::list<CARD*> _cl;
public:
  std::list<CARD*>& cl(){ return _cl; }
};
/*--------------------------------------------------------------------------*/
class CMD_HIDE : public CMD {
public:
  typedef DISPATCHER<CARD>::INSTALL installer;
public:
  ~CMD_HIDE(){
    for(auto i : _hidden){
      trace1("delete", i.second->short_label());
      delete i.first;
      delete i.second;
    }
  }
public:
  void do_it(CS& cmd, CARD_LIST* Scope)
  {

    // only operate on top level, for now.
    if(Scope!=&CARD_LIST::card_list){ untested();
      incomplete();
      return;
    }
    unsigned here = cmd.cursor();
    try {
      std::string sckt_name;
      cmd >> sckt_name;

      CARD_LIST::iterator i = Scope->begin();
      for(;i != Scope->end(); ++i) {
	if (dynamic_cast<BASE_SUBCKT const*>(*i)
	 || dynamic_cast<MODEL_CARD const*>(*i)) {
	  if ((*i)->short_label()==sckt_name) {
	    trace2("hide found", sckt_name, (*i)->short_label());
	    auto sl=(*i)->short_label();

	    // take it out of the cardlist.
///	    ((HACK_CARDLIST*)(Scope))->cl().erase(i);
	    reinterpret_cast<HACK_CARDLIST*>(Scope)->cl().erase(i);
	    auto I=new installer(&device_dispatcher, sl, *i);

	    assert(device_dispatcher[sl]);
	    auto P=std::make_pair(I, *i);
	    _hidden.push_back(P);

	    break;
	  }else{
	  }
	}else{
	}
      }

    }catch (Exception_File_Open& e) { untested();
      cmd.warn(bDANGER, here, e.message() + '\n');
    }catch (Exception_End_Of_Input& e) { untested();
      // done
    }
  }

private:
  std::list<std::pair<installer*, CARD*> > _hidden;
} p0;

DISPATCHER<CMD>::INSTALL d0(&command_dispatcher, "hidemodule", &p0);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
