/*                            -*- C++ -*-
 * Copyright (C) 2013-15
 * Author: Stefan Uhl, Felix Salfelder
 *
 * This file is part of "gnucap-bm"
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
 * multivariate polynomial
 */
/*--------------------------------------------------------------------------*/
#ifndef MVPOLY_HH__
#define MVPOLY_HH__
#include <math.h>
#include <assert.h>
#include <vector>
#include <iostream>
#include "io_trace.h"
using std::vector;
using std::pair;
using std::cerr;
using std::cout;
/*--------------------------------------------------------------------------*/
inline unsigned row(unsigned dim, unsigned pos)
{
  unsigned p = 1;
  for (unsigned i=1; i<=dim; ++i) {
    p *= i;
    pos *= i;
  }
  unsigned i = 0;
  while (1) {
    if(p > pos) return i;
    i += 1;
    p *= i+dim;
    // pos *= i; // faster but wrong for (2,78) upwards
    p /= i;
  }
}
/*--------------------------------------------------------------------------*/
inline unsigned total_degree( unsigned dim, size_t s )
{
  if(!s){
    return 0;
  }else{
    return row(dim, unsigned(s-1));
  }
}
/*--------------------------------------------------------------------------*/
inline unsigned s_num(unsigned n, unsigned dim)
{
  unsigned a = 1;
  unsigned b = 1;
  for(unsigned i=0; i<dim; i++){
    a *= n + i;
    b *= i + 1;
  }
  return a / b;
}
/*--------------------------------------------------------------------------*/
class MV_POLY_BASE{
    static vector<vector<unsigned> > _spn_list;
  protected:
    unsigned spn(unsigned n, unsigned dim) const
    {  // simplicial polytopic number P_d(n)
      while( _spn_list.size() <= dim ){
	vector<unsigned> a;
	a.push_back(0);
	_spn_list.push_back(a);
      }
      if(dim){
	if (_spn_list[dim].size() <= n) {
	  assert(n);
	  _spn_list[dim].push_back(spn(n-1, dim) + spn(n, dim-1));
	}
	assert(_spn_list[dim][n] == s_num(n, dim));
	return _spn_list[dim][n];
      }
      return 1;
    }
};
/*------------------------------------------------------------------------------*/
template<class T>
class MV_POLY : MV_POLY_BASE {
  private:
    typedef struct pd_{
      union{
	pd_* _next;
	double _val;
      };
#ifdef DEBUG_POLY
      bool _p;
      pd_& set_ptr(pd_*p){_p=1; _next=p;return *this;}
      pd_& set_val(const double x){_p=0; _val=x;return *this;}
      pd_(double x) : _val(x), _p(0){}
      pd_(): _val(0), _p(0) {}
#else
      pd_& set_ptr(pd_*p){_next=p;return *this;}
      pd_& set_val(const double x){_val=x;return *this;}
      pd_(double x) : _val(x){}
      pd_(): _val(0){}
#endif
    } pd;
  private:
    pd* _coeffs;
    unsigned _dim;
    unsigned _size; // unneccessary?
    unsigned _datasize; // just for debugging
  public: // cons
    template<class S>
    MV_POLY(S poly, unsigned dim=1) :
      _dim(dim), _size(unsigned(poly.size()))
    {
      if (!_dim){ untested();
	if(poly.size()){
	  _coeffs = new pd(poly[0]);
	}else{
	  _coeffs = new pd(0);
	}
	return;
      }

      unsigned td = total_degree( _dim, poly.size());

      _datasize=0;
      for(unsigned a=0; a<=_dim; ++a){
	_datasize += spn(td+1, a);
      }
      _datasize += 1*_dim+1;

      assert(_datasize == spn(td+2,_dim) + 1+1*_dim);
      _coeffs = new pd[_datasize];

      pd*here = _coeffs;

      for (unsigned dim=0; dim<_dim; ++dim) {
	here->set_ptr( here + spn(td+1, dim ) + 1 );
	here = here->_next;
      }

      pd* seekend = convert_poly(_coeffs+2, poly); // , _dim - 1);
      _coeffs[1].set_ptr( seekend );
    }
//    MV_POLY(unsigned s) : _coeffs( new T[s] ){
//      if(!s)_coeffs=0;
//    }
  public:
    unsigned degree() const {return row(_dim, _size-1); }
    unsigned dim()const{return _dim;}
    void set_dim(unsigned x){ _dim = x; }
    unsigned size()const{return _size;}
    void set_size(unsigned x){ _size = x; }
    T eval(double* x)const;
    T eval(const double* x)const;
    MV_POLY& eval_tail(T x, MV_POLY<T>* deriv=0){
      assert(0);
      return(*this);
    }
    template<class X>
    MV_POLY<T>& operator=(const vector<X>& c){
      _size = (unsigned)c.size();
      for(unsigned i=0; i<_size; i++){
	_coeffs[i] = c[i];
      }
      return *this;
    }
//    MV_POLY<T>& operator=(const T& c){
//      // hmm check real size?
//      _dim = 0;
//      _size = 1;
//      assert(_coeffs);
//      _coeffs[0] = c;
//      return *this;
//    }
//    T value()const{
//      assert(!_dim);
//      return _coeffs[0]._val;
//    }
    template<class S>
#ifdef DEBUG_POLY
    void print(S& s){
      s << "poly in " << dim() << " total degree " << degree() << " size " << size() << " ds " << _datasize << "\n (";
      for(unsigned i=0; i< _datasize; i++){
	pd t = _coeffs[i];
	if(t._p)
	  if(t._next)
	    s << " ->" << ( (intptr_t)(t._next) - (intptr_t)(_coeffs) ) / sizeof(pd);
	  else
	    s << " X";
	else
	  s << " " << t._val;
      }
      s << " )\n";
    }
#else
    void print(S& s, pd* here=0){
      if(!here) here=_coeffs;
      s << " ->" << ( here - _coeffs) << "\n";
      s << " )\n";
    }
#endif
  private:
    T _eval(const pd*, unsigned d, double* x)const;
    T _eval(const pd*, unsigned d, const double* x)const;

  private:
    template<class S>
    pd* convert_poly(pd* seek, const S poly, unsigned d=1, unsigned j=0, unsigned k=0)
    {
      assert(seek);
      unsigned h = _dim - d;
      pd* start = seek;
      pd* seekend = start;
      if (h) {
	while ( j < poly.size()) {
	  assert(seek->_next);
	  seekend = convert_poly(seek->_next, poly, d+1, j, k);
	  seek[1].set_ptr(seekend);
	  seek++;
	  k++;
	  j += spn(k + 1, d+h-1);
	  if (d > 1) j -= spn(k+1, d-2);
	}
	for(; seek>start && ( seek[0]._next == seek[1]._next ); --seek);
      }else{
	while(j < poly.size()){
	  seek->set_val( T(poly[j]) );
	  seek++;
	  k++;
	  j += spn( k, d-1);
	}
        while( seek>start && (!(seek-1)->_val)) seek--;
      }
      return seek;
    }
};
/*------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------*/
template<class T, class S>
T& operator<<(T& o, MV_POLY<S> p)
{ untested();
  p.print(o);
  return o;
}
/*--------------------------------------------------------------------------*/
// coeff order: - is forward.
#define CORDER -
template<class T>
T MV_POLY<T>::eval(const double* x)const
{ itested();
  return _eval(_coeffs, _dim, x+_dim-1);
}
/*--------------------------------------------------------------------------*/
template<class T>
T MV_POLY<T>::eval(double* x)const
{
  trace2("eval", _dim, x[0]);
  trace2("eval", _dim, x[1]);
  return _eval(_coeffs, _dim, x+_dim-1);
}
/*--------------------------------------------------------------------------*/
template<class T>
T MV_POLY<T>::_eval(const pd* p, unsigned d, const double* x)const
{ untested();
  if(!d) return p[0]._val;

  double a = 0;
  intptr_t size = p[1]._next - p[0]._next;
  if(size)
  for( pd* i = p[1]._next; ; ){
    --i;
    a += _eval(i, d-1, x CORDER 1);
    if(i==p[0]._next)break;
    a *= *x;
  }
  return a;
}
/*--------------------------------------------------------------------------*/
// evaluate and write derivative into input vector
template<class T>
T MV_POLY<T>::_eval(const pd* p, unsigned d, double* x)const
{
  if(!d) return p[0]._val;

  assert(d>0);
  double _xsave[d-1];
  double* xsave = _xsave-1;
  for(unsigned l=1; l<d; l++){
    xsave[l] = *(x CORDER l);
  }

  double ret = 0;
  double df[d];
  for(unsigned l=0; l<d; l++){
    df[l] = 0;
  }
  intptr_t size = p[1]._next - p[0]._next;
  if(size){
    for(intptr_t i=size-1; i>0; --i) {
      double s = _eval(p[0]._next+i, d-1, x CORDER 1);

      for(unsigned l=1; l<d; l++){
	df[l] += *(x CORDER l);
	df[l] *= x[0];
	*(x CORDER l) = xsave[l];
      }
      ret += s;
      ret *= x[0];
      df[0] *= x[0];
      df[0] += s * double(i);
    }
    ret += _eval(p[0]._next, d-1, x CORDER 1);
    for(unsigned l=1; l<d; l++){
      df[l] += *(x CORDER l);
      *(x CORDER l) = xsave[l];
    }
  }

  for(unsigned l=0; l<d; l++){
    *(x CORDER l) = df[l];
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
//template<class T, class S>
//MV_POLY<T>::MV_POLY(S poly, unsigned n_vars) :
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2
