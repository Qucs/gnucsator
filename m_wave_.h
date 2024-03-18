/*$Id: m_wave.h 2014/11/23$ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
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
 *------------------------------------------------------------------
 * "wave" class, for transmission lines and delays
 */
#ifndef M_WAVE_H
#define M_WAVE_H
#include "l_denoise.h"
#include "m_interp.h"
/*--------------------------------------------------------------------------*/
template<class T>
class WAVE_ {
private:
  typedef std::pair<double, T> PAIR;
  std::deque<PAIR> _w;
  double _delay;
public:
  typedef typename std::deque<PAIR>::iterator iterator;
  typedef typename std::deque<PAIR>::const_iterator const_iterator;

  explicit WAVE_(double d=0);
  explicit WAVE_(const WAVE_&);
	  ~WAVE_() {}
  WAVE_&	   set_delay(double d);
  WAVE_&	   initialize();
  WAVE_&	   push(double t, T v);
  FPOLY1   v_out(double t)const;
  double   v_reflect(double t, double v_total)const;
  WAVE_&	   operator+=(const WAVE_& x);
  WAVE_&	   operator+=(double x);
  WAVE_&	   operator*=(const WAVE_& x);
  WAVE_&	   operator*=(double x);
  const_iterator begin()const {return _w.begin();}
  const_iterator end()const {return _w.end();}
  size_t size()const{return _w.size();}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// push: insert a signal on the "input" end.
// args: t = the time now
//       v = the value to push
//
template<class T>
WAVE_<T>& WAVE_<T>::push(double t, T v)
{
  _w.push_back(std::make_pair(t+_delay, v));
  return *this;
}
/*--------------------------------------------------------------------------*/
// initialize: remove all info, fill it with all 0.
//
template<class T>
WAVE_<T>& WAVE_<T>::initialize()
{
  _w.clear();
  return *this;
}
/*--------------------------------------------------------------------------*/
template<class T>
WAVE_<T>::WAVE_(const WAVE_<T>& w)
  :_w(w._w),
   _delay(w._delay)
{ untested();
}
/*--------------------------------------------------------------------------*/
// constructor -- argument is the delay
template<class T>
WAVE_<T>::WAVE_(double d)
  :_w(),
   _delay(d)
{
  initialize();
}
/*--------------------------------------------------------------------------*/
template<class T>
WAVE_<T>& WAVE_<T>::set_delay(double d) 
{
  _delay = d; 
  return *this;
}
/*--------------------------------------------------------------------------*/
// v_out: return the value at the "output" end
// args: t = the time now
//
template<class T>
inline FPOLY1 WAVE_<T>::v_out(double t)const
{
  return interpolate(_w.begin(), _w.end(), t, 0., 0.);
}
/*--------------------------------------------------------------------------*/
// reflect: calculate a reflection
// args: t = the time now
//       v_total = actual voltage across the termination
// returns: the value (voltage) to send back as the reflection
//
template<class T>
inline double WAVE_<T>::v_reflect(double t, double v_total)const
{
  // return (v_total*2 - v_out(t)); // de-noised
  return dn_diff(v_total*2, v_out(t).f0);
}
/*--------------------------------------------------------------------------*/
template<class T>
inline WAVE_<T>& WAVE_<T>::operator+=(const WAVE_& x)
{ untested();
  for (typename std::deque<std::pair<double,T>>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    (*i).second += x.v_out((*i).first).f0;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
template<class T>
inline WAVE_<T>& WAVE_<T>::operator+=(double x)
{ untested();
  for (typename std::deque<std::pair<double,T>>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    (*i).second += x;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
template<class T>
inline WAVE_<T>& WAVE_<T>::operator*=(const WAVE_<T>& x)
{ untested();
  for (typename std::deque<std::pair<double,T>>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    (*i).second *= x.v_out((*i).first).f0;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
template<class T>
inline WAVE_<T>& WAVE_<T>::operator*=(double x)
{ untested();
  for (typename std::deque<std::pair<double,T>>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    (*i).second *= x;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
// typedef WAVE_<double> WAVE;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif

// vim:ts=8:sw=2:noet:
