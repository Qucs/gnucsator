/*                               -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2015-18 Felix Salfelder
 * Author: Albert Davis <aldavis@gnu.org>
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
/*--------------------------------------------------------------------------*/
#include <globals.h>
#include <e_compon.h>
/*--------------------------------------------------------------------------*/
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_G_POLY_K : public COMPONENT {
public:
	std::string value_name() const { return "dummy"; }
	std::string port_name(uint_t) const { return "dummy"; }

	CARD* clone()const
	{
		const CARD* c = device_dispatcher["cpoly_g"];
		assert(c);
		CARD* c2 = c->clone();
		COMPONENT* d = prechecked_cast<COMPONENT*>(c2);
		assert(d);
		const COMMON_COMPONENT* b = bm_dispatcher["poly_k"];
		assert(b);
		COMMON_COMPONENT* bc = b->clone();
		d->attach_common(bc);
		// d->set_dev_type("g_poly_k");
		// assert(d->dev_type() == "g_poly_k");
		return d;
	}
private:
	bool print_type_in_spice()const{untested(); return false; }
}p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "g_poly_k|G_poly", &p1);
/*--------------------------------------------------------------------------*/
}
