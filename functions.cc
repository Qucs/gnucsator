/*                      -*- C++ -*-
 * Copyright (C) 2015 Felix Salfelder
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
 * more functions.
 */

#include <math.h>
#include <globals.h>
#include <u_function.h>
#include <u_parameter.h>

#ifndef HAVE_UINT_T
typedef int uint_t;
#endif

#ifndef _U_FUNC
typedef std::string fun_t;
#define to_fun_t to_string
#endif

#define function_plugin(name, expr) \
namespace { \
class FUNC_##name : public FUNCTION { \
  public: \
    fun_t eval(CS& Cmd, const CARD_LIST* Scope)const \
    { \
      PARAMETER<double> x; \
      Cmd >> x; \
      x.e_val(NOT_INPUT, Scope); \
      return to_fun_t(expr); \
    } \
} p##name; \
DISPATCHER<FUNCTION>::INSTALL d##name(&function_dispatcher, #name, &p##name); \
}

// exponential and logarithmic functions
function_plugin(exp, exp(x))
function_plugin(limexp, exp(x)) // uuh
function_plugin(log10, log10(x))
function_plugin(log2, log2(x))
function_plugin(ln, log(x))

// inverse trigonometric functions
function_plugin(arccos, acos(x))
function_plugin(arccosec, asin(1./x))
function_plugin(arccot, M_PI_2 - atan(x))
function_plugin(arcsec, acos(1./x))
function_plugin(arcsin, asin(x))
// function_plugin(arctan, arctan); two args. FIXME

// hyperbolic functions
function_plugin(cosh, cosh(x)) // (exp(x)+exp(-x))*.5)
function_plugin(cosech, 1./sinh(x)) // 2./(exp(x)-exp(-x))
function_plugin(coth, 1.+2./exp(2*x-1)) // cosh(x)/sinh(x)
function_plugin(sech, 2./(exp(x)+exp(-x))) // 1/cosh(x)
function_plugin(sinh, sinh(x))
function_plugin(tanh, 1.-2./exp(2*x+1)) // sinh(x)/cosh(x)
