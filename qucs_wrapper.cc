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
 *
 * qucs component wrapper for gnucap
 * compile with -DQUCS_DEVICE=<name>
 */

#ifndef QUCS_DEVICE
# warning no QUCS_DEVICE specified, defaulting to opamp
# define QUCS_DEVICE opamp
#endif

#include <globals.h>
#include <e_storag.h>

// BUG, qucs headers lack typedefs
#include <qucs_typedefs.h>
// BUG, qucs circuit.h uses object, but does not include header
#include <object.h>

// http://okmij.org/ftp/cpp-digest/computable-include.txt
// looks ugly, works perfectly
#define COMPPATH qucs-core/components
#define _XqYq(x,y) x ## y
#define _XY(x,y) _XqYq(x,y)
#define QUOTE(x) #x
#define MAKESTR(x) QUOTE(x)
#define MAKEINC(x) COMPPATH/x.h
#define QDINC MAKEINC(QUCS_DEVICE)


#include <qucs-core/complex.h>
#include <qucs-core/circuit.h>
#include MAKESTR(QDINC)

#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

using std::string;

namespace{
class DEV_QUCS : public STORAGE {
	private:
	  static int _count;
	public:
		CARD*	clone()const;
		DEV_QUCS(): STORAGE(){ untested();
			_comp = new QUCS_DEVICE();
		}
		void tr_iwant_matrix(){ untested(); incomplete(); }
		void ac_iwant_matrix(){ untested(); incomplete(); }
		std::string value_name()const { untested();return "nothing";}
	public: // ports
		std::string port_name(uint_t)const { untested();incomplete(); return "";}
		void set_port_by_name(std::string& name, std::string& value);
		void set_port_by_index(uint_t index, std::string& value);
		bool port_exists(uint_t i)const { untested();return i < net_nodes();}
		const std::string port_value(uint_t i)const;
		void set_port_to_ground(uint_t index);

		std::string current_port_name(uint_t)const { untested();return "";}
		const std::string current_port_value(uint_t)const;
		void set_current_port_by_index(uint_t, const std::string&) { untested();unreachable();}
		bool current_port_exists(uint_t i)const { untested();return i < num_current_ports();}

		uint_t max_nodes()const { untested();unreachable(); return 0;}
		uint_t min_nodes()const { untested();unreachable(); return 0;}
		uint_t num_current_ports()const { untested();return 0;}
		uint_t tail_size()const { untested();return 0;}

		uint_t net_nodes()const {return 0;} //override
		uint_t ext_nodes()const { untested();return max_nodes();}
		uint_t int_nodes()const { untested();return 0;}
		uint_t matrix_nodes()const { untested();return 0;}

		bool has_inode()const { untested();return false;}
		bool has_iv_probe()const { untested();return false;}
		bool is_source()const { untested();return false;}
		bool f_is_value()const { untested();return false;}

		bool node_is_grounded(uint_t)const { untested();incomplete(); return false;}
		bool node_is_connected(uint_t)const { untested();incomplete(); return false;}
		//--------------------------------------------------------------------
	public:
		double tr_review_trunc_error(const FPOLY1* q);
		double tr_review_check_and_convert(double timestep);

		double tr_involts()const { untested();unreachable(); return 0;}
		//double  tr_input()const { untested();return tr_involts();}
		double tr_involts_limited()const { untested();unreachable(); return 0.;}
		double tr_input_limited()const { untested();return tr_involts_limited();}
		//double  tr_amps()const;
		COMPLEX ac_involts()const { untested();unreachable(); return COMPLEX();}
		//COMPLEX ac_amps()const;

		// int order()const return _trsteporder;}
		// double error_factor()const { untested();return OPT::trstepcoef[_trsteporder];}
	private:
		qucs::circuit* _comp;
}p0;
/*--------------------------------------------------------------------------*/
static DISPATCHER<CARD>::INSTALL
  d0(&device_dispatcher, QUOTE(QUCS_DEVICE), &p0);
/*--------------------------------------------------------------------------*/
CARD*	DEV_QUCS::clone()const
{ untested();
	return new DEV_QUCS(*this);
}
/*--------------------------------------------------------------------------*/
const string DEV_QUCS::current_port_value(uint_t)const
{ incomplete();
	return "";
}
/*--------------------------------------------------------------------------*/
void DEV_QUCS::set_port_by_name(std::string& /*name*/, std::string& /*value*/)
{ untested();
	incomplete();
}
/*--------------------------------------------------------------------------*/
void DEV_QUCS::set_port_by_index(uint_t /*index*/, std::string& /*value*/)
{ untested();
	incomplete();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
