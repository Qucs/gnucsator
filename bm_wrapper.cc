/* Copyright (C) 2012-2015 Felix Salfelder
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
 * wrapping spice bms to other languages. here: qucs
 * elt+bm -> component that doesnt look spicey
 *           has parameters, one ("value") stored in elt,
 *           the others in bm
 */
#include <globals.h>
#include <e_compon.h>

namespace {
using std::string;

class DEV_BM_WRAP : public COMPONENT { //
	public:
		explicit DEV_BM_WRAP(string e, string b, string n) :
			COMPONENT(), _elt(e), _bm(b), _cname(n){}
		string value_name() const { return "dummy"; }
		string port_name(uint_t) const { return "dummy"; }
		bool print_type_in_spice() const{ untested(); return false;}

		CARD* clone()const
		{ untested();
			const CARD* c = device_dispatcher[_elt];
			assert(c);
			CARD* c2 = c->clone();
			COMPONENT* d = prechecked_cast<COMPONENT*>(c2);
			assert(d);
			const COMMON_COMPONENT* b = bm_dispatcher[_bm];
			if(!b){ untested();
				error(bDANGER, "cannot clone from wrapper, missing model " + _bm);
			}
			assert(b);
			COMMON_COMPONENT* bc = b->clone();
			d->attach_common(bc);

// #ifndef NDEBUG // this is just a sanity check
// 			d->set_dev_type("TEST");
// 			trace2("dev_type test", d->dev_type(), cname());
// 			assert(d->dev_type() == "TEST");
// #endif
			d->set_dev_type(cname());
			if(d->dev_type() == cname()){
			}else{ untested();
				trace2("dev_type problem", d->dev_type(), cname());
				//assert(d->dev_type() == cname());
			}

			return d;
		}
		string cname() const { if(_cname!="") return _cname; else return _elt+"_"+_bm; }
	private:
		const string _elt;
		const string _bm;
		const string _cname;
};

#define dispatch(elt,bm,name) \
namespace elt ## bm ## name { \
	DEV_BM_WRAP dev(#elt, #bm, #name); \
	DISPATCHER<CARD>::INSTALL d(&device_dispatcher, dev.cname(), &dev); \
}

dispatch(vsource, qucs_trivial_U, Vdc)
dispatch(vsource, sin, Vsin)
dispatch(vsource, pulse, Vpulse)
dispatch(vsource, pulse, Vrectangle)

dispatch(resistor, qucs_value, R)

}
