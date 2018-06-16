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
#include <e_elemnt.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#ifdef DO_TRACE
#ifdef HAVE_IO_MISC_H
# include <io_misc.h>
#endif
#endif
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
/*--------------------------------------------------------------------------*/
namespace {
using std::string;
/*--------------------------------------------------------------------------*/
/* wrap spice ELT+BM into plain COMPONENT
 * for some reason, this does not work with upstream gnucap
 * ... need to figure out something.
 */
class DEV_BM_WRAP : public COMPONENT { //
	public:
		explicit DEV_BM_WRAP(string e, string b, string n) :
			COMPONENT(), _elt(e), _bm(b), _cname(n){}
		string value_name() const { return "dummy"; }
		string port_name(uint_t) const { return "dummy"; }
		bool print_type_in_spice() const{ untested(); return false;}

		CARD* clone()const
		{
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
			trace2("setting dev type", d->dev_type(), cname());
//			d->set_dev_type(cname());
			if(d->dev_type() == cname()){
			}else{
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
	if (!what) return 0;
	unsigned n = 0;
	while(what[n]){
		++n;
	}
	return n;
}
/*--------------------------------------------------------------------------*/
#ifndef HAVE_ELEMENT_PARAM // upstream gnucap
static void stuff_param_into_element(COMPONENT* c, string P, string V)
{
	ELEMENT* e = prechecked_cast<ELEMENT*>(c);
	assert(e);
	assert(e->has_common());
	COMMON_COMPONENT* m = e->mutable_common()->clone();
	if(e->value_name() == P){ // && bm_value?
		std::string args("= " + V);
		CS cmd(CS::_STRING, args);
		m->parse_params_obsolete_callback(cmd);
	}else{
		trace2("setting params to bm", P, V);
		m->set_param_by_name(P,V);
	}
	e->attach_common(m);
}
#endif
/*--------------------------------------------------------------------------*/
/* wrap COMPONENT proto into SCKT,
 * translate port/parameter names, wrap probes.
 */
class DEV_SCKT_WRAP : public BASE_SUBCKT{
	private:
		const COMPONENT* _wrapproto;
		const char** _port_name;
		const char** _param_name;
		const char** _assignments;
		const unsigned _port_number;
		const unsigned _param_number;
		const unsigned _assign_number; // number of assignments
		const unsigned _nocache_number; // don't cache that many params
		const string _type;
		COMPONENT* _c1;
		node_t _nodes[2]; // more?
		PARAMETER<double>* _param;
		PARAM_LIST _param_cache; // dictionary with params required for
		                         // assignments
		DEV_SCKT_WRAP(const DEV_SCKT_WRAP& p) : BASE_SUBCKT(p),
		_wrapproto(p._wrapproto),
		_port_name(p._port_name),
		_param_name(p._param_name),
		_assignments(p._assignments),
		_port_number(p._port_number),
		_param_number(p._param_number),
		_assign_number(p._assign_number),
		_nocache_number(p._nocache_number),
		_type(p._type),
		_c1(NULL) {
			_n = _nodes;
			for (uint_t ii = 0; ii < max_nodes() + int_nodes(); ++ii) {
				_n[ii] = p._n[ii];
			}
			_param = new PARAMETER<double>[_param_number];
			for (unsigned ii=0; ii < _param_number; ++ii) {
				_param[ii] = p._param[ii];
			}
		}
	public:
		DEV_SCKT_WRAP(COMPONENT* d,
		              const char* pn[],
		              const char* par[],
		              string type,
		              const char* assign[]=NULL,
		              unsigned parcache=-1) : BASE_SUBCKT(),
		_wrapproto(d),
		_port_name(pn),
		_param_name(par),
		_assignments(assign),
		_port_number(count(pn)),
		_param_number(count(par)),
		_assign_number(count(assign)),
		_nocache_number(parcache),
		_type(type),
		_c1(NULL) {
			trace3("wrapper", type, _param_number, _assign_number);
			_n = _nodes;
			assert(_port_number==2); // for now.
			_param = new PARAMETER<double>[_param_number];
		}
		virtual CARD*	 clone()const{return new DEV_SCKT_WRAP(*this);}
		uint_t max_nodes()const {return _port_number;}
		uint_t min_nodes()const {return _port_number;}
		uint_t tail_size()const {return 0;}
		uint_t int_nodes()const {return 0;}
		string value_name()const {return "#";}
		string dev_type()const {return _type;} // printed name
		string modelname()const {return "Vdcmodel";}
		int param_count()const {return (_param_number + BASE_SUBCKT::param_count());}
		bool print_type_in_spice()const {return false;}
		void set_param_by_name(string Name, string Value)
		{
			trace2("DEV_SCKT_WRAP::set_param_by_name", Name, Value);
			for(unsigned i=0; i<_param_number; ++i){
				if (Umatch (Name, _param_name[i])) {
					trace2("DEV_SCKT_WRAP::set_param_by_name", i, Value);
					_param[i] = Value;
					if(i>=_nocache_number){
						CS cs(CS::_STRING, Name+"={"+Value+"}");
						trace2("caching param", Name, Value);
						_param_cache.parse(cs);
					}else{
					}
					return;
				}
			}
			COMPONENT::set_param_by_name(Name,Value);
		}
		std::string param_name(int i, int j)const{ untested();
			if(param_count() - 1 - i >= int(_param_number)){ untested();
				return "";
			}else if(j){ untested();
				return "";
			}else{ untested();
			 return _param_name[param_count() - 1 -i];
			}
		}
		std::string param_name(int i)const{ itested();
			assert(param_count() - 1 - i < int(_param_number));
			return _param_name[param_count() - 1 -i];
		}
		bool param_is_printable(int i)const{ itested();
			trace3("is printable", i, BASE_SUBCKT::param_count(), param_count());
			unsigned j = param_count() - 1 - i;
			return j < _param_number;
		}
		std::string param_value(int i)const{ itested();
			assert(param_count() - 1 - i < int(_param_number));
			return _param[param_count() - 1 - i].string();
		}
		double tr_probe_num(const string& s)const{
			assert(_c1);
			return _c1->tr_probe_num(s);
		}
		std::string port_name(uint_t x)const{
			assert (x<2);
			return _port_name[x];
		}
		void expand(){
			if (!subckt()) {
				assert(scope());
				if(_assignments){
					new_subckt();
					subckt()->params()->set_try_again(&_param_cache);
					_param_cache.set_try_again(scope()->params());
				}else{
#if 0 // uf hack
					new_subckt(scope()->params());
#else

					new_subckt();
					PARAM_LIST* dummy = new PARAM_LIST;
					subckt()->attach_params(dummy, scope());
					delete dummy;
					subckt()->params()->set_try_again(scope()->params());
#endif
				}
				assert(subckt()->params());
			}else{
			}
			if (_sim->is_first_expand()) {
				precalc_first();
				precalc_last();
				if (!_c1) {
					_c1 = dynamic_cast<COMPONENT*>(_wrapproto->clone_instance());
					// _c1->set_dev_type(_type);
					assert(_c1);
					_c1->set_owner(this);
					subckt()->push_front(_c1);
				}else{untested();
				}
				{
					node_t nodes[] = { _n[0], _n[1] }; // more?
					_c1->set_parameters("dev", this, _c1->mutable_common(),
							0/*value*/, 0, NULL, 2, nodes);
					for(unsigned i=0; _param_name[i] && // clumsy?
							            _param_name[i+_param_number+1]; ++i){ itested();
						trace3("params", i, _param_number, _param_name[i+_param_number+1]);
						assert(i<_param_number);
						if(_param[i].has_hard_value()){
							trace3("forwarding params", _param_name[i],
									_param_name[i+_param_number+1], _param[i].string());
#ifdef HAVE_ELEMENT_PARAM // uf
							_c1->set_param_by_name(string(_param_name[i+_param_number+1]), _param[i].string());
#else // upstream gnucap
							stuff_param_into_element(_c1, string(_param_name[i+_param_number+1]),
									                   _param[i].string());
#endif
							trace1("done param", _param_name[i]);
						}else{
						}
					}
					for(unsigned i=0; i<_assign_number; ++i){
						trace3("assign params", i, _assignments[i],_assignments[i+_assign_number+1]);
						_c1->set_param_by_name(_assignments[i],
						                       _assignments[i+_assign_number+1]);
					}
					trace0("done");
				}
			}else{untested();
			}
			assert(!is_constant());
			assert(subckt());
//			subckt()->set_slave();
//			trace3("expanding sckt", subckt()->size(), hp(this), long_label());
			subckt()->expand();
		}
		void precalc_last()
		{
			COMPONENT::precalc_last();
			assert(subckt());
#ifdef HAVE_IO_MISC_H
			trace1("wrap:expand", *(subckt()->params()));
#endif
			subckt()->precalc_last();
		}
};

/*--------------------------------------------------------------------------*/
const char* pn[3] = {"p","n", NULL};
/*--------------------------------------------------------------------------*/
// dispatch(vsource, qucs_trivial_U, Vdc)
const char* Vdc_param[5] = {"U",  "Temp", NULL,
                            "dc", "temp"};
DEV_BM_WRAP qucs_Vdc("vsource", "value", "uneeded");
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
// Vpulse:V3 _net2 _net3 U1="0 V" U2="1 V" T1="0" T2="1 ms" Tr="1 ns" Tf="1 ns"
// Vpulse:V2 _net2 gnd U1="0 V" U2="1 V" T1="5m" T2="10m" Tr="1 ns" Tf="1 ns"
// incomplete.
const char* Vpulse_param[] =
{  "U1", "U2", "T2",   "TL",   "Tr",   "Tf",   "T1", NULL,
   "iv", "pv", "width","tlow", "rise", "fall", "delay" };
const char* Vpulse_assign[] =
{  "width",     NULL,
   "{T2-T1}"};
DEV_BM_WRAP qucs_Vpulse("vsource", "pulse", "uneeded");
DEV_SCKT_WRAP m3(&qucs_Vpulse, pn, Vpulse_param, "Vpulse", Vpulse_assign, 1);
DISPATCHER<CARD>::INSTALL d3(&device_dispatcher, "Vpulse", &m3);
/*--------------------------------------------------------------------------*/
// Vrect:V2 _net1 gnd U="1 V" TH="1 ms" TL="1 ms" Tr="1 ns" Tf="1 ns" Td="0 ns"
const char* Vrect_param[] =
{  "U",  "Td",    "Tr",   "Tf",   "TH", "TL", NULL,
   "pv", "delay", "rise", "fall", NULL, NULL   };
const char* Vrect_assign[] =
{  "period",        "width",      "iv", NULL,
   "{TH+TL+Tr+Tf}", "{TH+Tf+Tr}", "0"};
DEV_BM_WRAP qucs_Vrect("vsource", "pulse", "uneeded");
DEV_SCKT_WRAP m3b(&qucs_Vpulse, pn, Vrect_param, "Vrect", Vrect_assign, 2);
DISPATCHER<CARD>::INSTALL d3b(&device_dispatcher, "Vrect", &m3b);
/*--------------------------------------------------------------------------*/
// dispatch(resistor, bm_value, R)
const char* Vres_param[] = {"R", "Tc1", "Tc2", "Temp", "Tnom", "ic", NULL,
                            "r", "tc1", "tc2", "temp", "tnom", "ic"};
DEV_BM_WRAP qucs_Res("resistor", "value", "uneeded");
DEV_SCKT_WRAP m4(&qucs_Res, pn, Vres_param, "R");
DISPATCHER<CARD>::INSTALL d4(&device_dispatcher, "R", &m4);
/*--------------------------------------------------------------------------*/
const char* Vcap_param[] = {"C", "V", NULL,
                            "c", "ic"};
DEV_BM_WRAP qucs_Cap("capacitor", "value", "uneeded");
DEV_SCKT_WRAP m5(&qucs_Cap, pn, Vcap_param, "C");
DISPATCHER<CARD>::INSTALL d5(&device_dispatcher, "C", &m5);
/*--------------------------------------------------------------------------*/
}
