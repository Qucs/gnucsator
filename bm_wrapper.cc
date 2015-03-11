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
 *           the others in bm.
 * put this into a sckt shell and dispatch
 */
#include <globals.h>
#include <e_compon.h>
#include <d_subckt.h>
/*--------------------------------------------------------------------------*/
namespace {
using std::string;
/*--------------------------------------------------------------------------*/
/* wrap spice ELT+BM into plain COMPONENT
 */
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
				throw Exception("cannot clone from wrapper, missing model " + _bm);
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
/*--------------------------------------------------------------------------*/
template<class T>
static unsigned count(const T* what){
	unsigned n = 0;
	while(what[n]){ untested();
		++n;
	}
	return n;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* wrap COMPONENT proto into SCKT,
 * translate port/parameter names, wrap probes.
 */
class DEV_SCKT_WRAP : public BASE_SUBCKT{
	private:
		const COMPONENT* _wrapdev;
		const char** _port_name;
		const char** _param_name;
		const unsigned _port_number;
		const unsigned _param_number;
		const string _type;
		COMPONENT* _c1;
		node_t _nodes[2]; // more?
		PARAMETER<double> _param[3]; // more?
		DEV_SCKT_WRAP(const DEV_SCKT_WRAP& p) : BASE_SUBCKT(p),
		_wrapdev(p._wrapdev),
		_port_name(p._port_name),
		_param_name(p._param_name),
		_port_number(p._port_number),
		_param_number(p._param_number),
	  	_type(p._type) {
			_n = _nodes;
			for (uint_t ii = 0; ii < max_nodes() + int_nodes(); ++ii) {
				_n[ii] = p._n[ii];
			}
		}
	public:
		DEV_SCKT_WRAP(COMPONENT* d,
		              const char* pn[],
		              const char* par[],
		              string type) : BASE_SUBCKT(),
		_wrapdev(d),
		_port_name(pn),
		_param_name(par),
		_port_number(count(pn)),
		_param_number(count(par)),
		_type(type)	{ untested();
			_n = _nodes;
			assert(_port_number==2);
		}
		virtual CARD*	 clone()const{return new DEV_SCKT_WRAP(*this);}
		uint_t max_nodes()const {return _port_number;}
		uint_t int_nodes()const {return 0;}
		string value_name()const {return "#";}
		string dev_type()const {return _type;} // printed name
		string modelname()const {return "Vdcmodel";}
		int param_count()const {return (_param_number + BASE_SUBCKT::param_count());}
		void set_param_by_name(string Name, string Value)
		{ untested();
			trace2("DEV_SCKT_WRAP::set_param_by_name", Name, Value);
			for(unsigned i=0; i<_param_number; ++i){
				if (Umatch (Name, _param_name[i])) { untested();
					_param[i] = Value;
					return;
				}
			}
			COMPONENT::set_param_by_name(Name,Value);
		}
		std::string param_name(int, int)const{incomplete();
			return "";
		}
		std::string param_name(int i)const{ itested();
			return _param_name[BASE_SUBCKT::param_count() - 1 -i];
		}
		bool param_is_printable(int i)const{ itested();
			unsigned j = BASE_SUBCKT::param_count() - 1 - i;
			return j < _param_number;
		}
		std::string param_value(int i)const{ itested();
			return _param[BASE_SUBCKT::param_count() - 1 - i];
		}
		double tr_probe_num(const string& s)const{ untested();
			assert(_c1);
			return _c1->tr_probe_num(s);
		}
		void precalc_last()
		{untested();
			COMPONENT::precalc_last();
			assert(subckt());
			subckt()->precalc_last();
		}
		std::string port_name(uint_t x)const{ untested();
			assert (x<2);
			return _port_name[x];
		}
		void expand(){ untested();
			if (!subckt()) { untested();
				assert(scope());
				new_subckt(scope()->params());
				assert(subckt()->params());
			}else{
			}
			if (_sim->is_first_expand()) { untested();
				precalc_first();
				precalc_last();
				if (!_c1) {
					const CARD* p = _wrapdev;
					_c1 = dynamic_cast<COMPONENT*>(p->clone_instance());
					_c1->set_dev_type("Vdc");
					assert(_c1);
					subckt()->push_front(_c1);
				}else{untested();
				}
				{
					node_t nodes[] = { _n[0], _n[1] }; // more?
					_c1->set_parameters("anonymous", this, _c1->mutable_common(), 0/*value*/, 0, NULL, 2, nodes);
					for(unsigned i=0; i<_param_number; ++i){ untested();
						trace1("forwarding params", _param_name[i]);
						_c1->set_param_by_name(_param_name[i+_param_number+1], _param[i]);
					}
				}
			}else{untested();
			}
			assert(!is_constant());
			subckt()->set_slave();
			subckt()->expand();
		}
};

/*--------------------------------------------------------------------------*/
const char* pn[3] = {"p","n", NULL};
/*--------------------------------------------------------------------------*/
// dispatch(vsource, qucs_trivial_U, Vdc)
const char* Vdc_param[3] = {"U", NULL,
                            "dc"};
DEV_BM_WRAP qucs_Vdc("vsource", "qucs_trivial_U", "uneeded");
DEV_SCKT_WRAP m1(&qucs_Vdc, pn, Vdc_param, "Vdc");
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "Vdc", &m1);
/*--------------------------------------------------------------------------*/
// dispatch(vsource, sin, Vsin)
const char* Vsin_param[] = {"U", NULL,
                            "dc"};
DEV_BM_WRAP qucs_Vsin("vsource", "sin", "uneeded");
DEV_SCKT_WRAP m2(&qucs_Vsin, pn, Vsin_param, "Vsin");
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "Vsin", &m2);
/*--------------------------------------------------------------------------*/
// dispatch(vsource, pulse, Vpulse)
const char* Vpulse_param[] =
{  "U", "TH",   "TL",   "Tr",   "Tf",   "Td", NULL,
   "pv","width","tlow", "rise", "fall", "delay" };
DEV_BM_WRAP qucs_Vpulse("vsource", "pulse", "uneeded");
DEV_SCKT_WRAP m3(&qucs_Vpulse, pn, Vpulse_param, "Vpulse");
DISPATCHER<CARD>::INSTALL d3(&device_dispatcher, "Vpulse|Vrectangle", &m3);
/*--------------------------------------------------------------------------*/
// dispatch(resistor, bm_value, R)
const char* Vres_param[] = {"R", "Tc1", "Tc2", "Temp", "ic", NULL,
                            "r", "tc1", "tc2", "temp", "ic"};
DEV_BM_WRAP qucs_Res("vsource", "pulse", "uneeded");
DEV_SCKT_WRAP m4(&qucs_Res, pn, Vres_param, "R");
DISPATCHER<CARD>::INSTALL d4(&device_dispatcher, "R", &m4);
/*--------------------------------------------------------------------------*/
}
