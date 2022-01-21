/*                            -*- C++ -*-
 * Copyright (C) 2021 Felix Salfelder
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
 * root subckt for use in gnucap-qucs
 */
#include <e_subckt.h>
#include <globals.h>
#include "e_paramlist.h"

/* -------------------------------------------------------------------------------- */
namespace{
/* -------------------------------------------------------------------------------- */
class ROOT : public BASE_SUBCKT{
public:
	explicit ROOT() : BASE_SUBCKT(){ untested();
		new_subckt();
	}
	ROOT(ROOT const& p) : BASE_SUBCKT(p){ untested();
		new_subckt();
	}
	~ROOT(){ untested();
	}

private:
	void precalc_first() override { untested();
		BASE_SUBCKT::precalc_first();
		untested();
		assert (subckt());
		subckt()->precalc_first();
	}
	void precalc_last() override { untested();
		BASE_SUBCKT::precalc_last();
		untested();
		assert (subckt());
		subckt()->precalc_last();
	}

private:
  CARD* clone()const override{return new ROOT(*this);}
  std::string value_name()const{incomplete(); unreachable(); return ""; }
  std::string port_name(int)const {unreachable(); return "";}
  bool print_type_in_spice()const {return false;}
}p2;
DISPATCHER<CMD>::INSTALL d2(&device_dispatcher, "root", &p2);
/* -------------------------------------------------------------------------------- */
class MAIN : public BASE_SUBCKT{
public:
	explicit MAIN() : BASE_SUBCKT(){ untested();
		new_subckt();
	}
	~MAIN(){}

private:
	explicit MAIN(MAIN const& p) : BASE_SUBCKT(p){ untested();
		new_subckt();
	}
	CARD* clone()const override{return new MAIN(*this);}

private:
	void precalc_first() override { untested();

		BASE_SUBCKT::precalc_first();
		untested();
		assert (subckt());
		subckt()->precalc_first();
	}
	void precalc_last() override { untested();
		BASE_SUBCKT::precalc_last();
		subckt()->precalc_last();
		assert(!is_constant()); /* because I have more work to do */
	}
	void expand() override { untested();
		assert(false); // not reached
		COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
		assert(c);
		assert(CARD_LIST::card_list.params());
		PARAM_LIST* pl = const_cast<PARAM_LIST*>(CARD_LIST::card_list.params());
		assert(pl);
		c->_params.set_try_again(pl); // needed?

		for(auto i : *CARD_LIST::card_list.params()){ untested();
			trace2("root PARAM", i.first, i.second);
		}

		subckt()->expand();
	}

private:
  std::string value_name()const{incomplete(); unreachable(); return ""; }
  std::string port_name(int)const {unreachable(); return "";}
  bool print_type_in_spice()const {return false;}
}p1;
DISPATCHER<CMD>::INSTALL d1(&device_dispatcher, "main", &p1);
/* -------------------------------------------------------------------------------- */
}
/* -------------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------------- */
