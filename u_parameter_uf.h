/*                                -*- C++ -*-
 * Copyright (C) 2010-18 Felix
 * Author: Felix
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
 * gnucap-uf parameter extensions. mostly vector.
 * (works with both upstream and -uf, hence still ugly)
 */
#ifndef PAR_UF_H
/*--------------------------------------------------------------------------*/
#define IString std::string
#include "u_parameter.h"
/*--------------------------------------------------------------------------*/
template <class T>
class PARAMETER<std::vector<PARAMETER<T> > > : public PARA_BASE{
  private:
    mutable std::vector<PARAMETER<T> > _v;
    std::vector<PARAMETER<T> > _NOT_INPUT() const;
  public:
    operator std::vector<PARAMETER<T> >()const { return _v;}
    explicit PARAMETER(T v) : PARA_BASE("#"), _v(v) {}
    PARAMETER() : PARA_BASE(), _v(std::vector<PARAMETER<T> >()) {}
    PARAMETER(const PARAMETER<std::vector<PARAMETER<T> > >& p) :
      PARA_BASE(p), _v(p._v){ }

    //		void	print(OMSTREAM& o)const		{o << string();}
    //		void	print(ostream& o)const		{o << string();}

    std::string string()const;
    //std::vector<PARAMETER<T> >  _NOT_INPUT() const;
    void	operator=(const IString& s);
    void	operator=(const PARAMETER<std::vector<PARAMETER<T> > >& p){ untested();
      _v = p._v; _s = p._s;
    }
    void	operator=(const std::vector<PARAMETER<T> >& v) {
      _v = v; _s = "#";
    }
    bool operator==(const PARAMETER<std::vector<PARAMETER<double> > >& p)const{
		 incomplete();
		 return false;
	 }
    std::vector<PARAMETER<T> >	e_val(const std::vector<PARAMETER<T> >& def,
        const CARD_LIST* scope)const;
    std::string to_string(std::vector< PARAMETER<T> > n) const;

    operator std::string()const;
    size_t size()const{return _v.size();}

    bool has_good_value()const {incomplete(); return false;}
    void parse(CS&) {incomplete();}
}; // PARAMETER<std::vector<PARAMETER<T> > >
/*--------------------------------------------------------------------------*/
template <class T>
PARAMETER<std::vector<PARAMETER<T> > >::operator std::string()const
{
  return string();
}
/*--------------------------------------------------------------------------*/
template <class T>
inline std::string PARAMETER<std::vector<PARAMETER<T> > >::string()const
{
  std::string ret("");
  if (PARAMETER<std::vector<PARAMETER<T> > >::_s == "#") {
    ret+= "(";
  }else if (_s == "") {
    ret+= "NA(";
  }else{ untested();
    return std::string(_s);
  }
  for(unsigned  i=0; i<_v.size(); i++){
    ret+= (i)?",":"";
    ret+= _v[i].string();
  }
  ret+=")";
  return ret;
}
/*--------------------------------------------------------------------------*/
template <class T>
inline std::vector<PARAMETER<T> > PARAMETER<std::vector<PARAMETER<T> > >::_NOT_INPUT() const
{
  return std::vector<PARAMETER< T> > ();
}
/*--------------------------------------------------------------------------*/
template <class T>
inline std::vector<PARAMETER<T> >
PARAMETER<std::vector<PARAMETER<T> > >::e_val(const std::vector<PARAMETER<T> >& def, const CARD_LIST* scope)const
{
  trace2("PARAMETER dv::e_val", _s, _v.size());
  trace1("PARAMETER dv::e_val", (std::string)(*this));
  trace1("PARAMETER dv::e_val", def.size() );

  //  CS c(CS::_STRING,_s);
  // FIXME: accept strings and parse...
  for(unsigned  i=0; i<_v.size()  ; i++){
    PARAMETER<T> D;
    if (i < def.size()){
      D = def[i];
    }
    trace3("PARAMETER vector eval", i, _v[i], D);
    _v[i].e_val(D, scope);
    trace2("PARAMETER vector eval", i, _v[i]);
  }
  return _v;
}
/*--------------------------------------------------------------------------*/
template<class T>
void PARAMETER<std::vector<PARAMETER<T> > >::operator=(const IString& s)
{
  trace1("PARAMETER dv::operator=" , s);

  CS cmd(CS::_STRING, std::string(s));
  _v.clear();
  std::string compon;

  cmd.skipbl();

  for(;;){
    //if (! cmd.umatch("(")){
    //   untested();
    //   break;
    //}
    if(!cmd.more()) break;
    compon = cmd.ctos(",","({",")}",""); // More sorts of braces?

    PARAMETER<T> d;

    //d =  '(' + compon + ')';
    d = compon;
    _v.push_back( d );

    cmd.skip1b(')');
    trace1("PARAMETER vector loop pushed back ", d);
  }
  _s = "#";
  trace2("PARAMETER done vector loop", cmd.tail(), *this);
}
/*--------------------------------------------------------------------------*/
#ifdef UF_PARAM_HACKS
template<>
inline CS& CS::operator>>(std::vector<PARAMETER<std::vector<PARAMETER<double> > > >& x);
#else
inline bool operator==(std::vector<PARAMETER<double> >, double)
{
	incomplete();
	return false;
}
#endif
/*--------------------------------------------------------------------------*/
template <class S>
inline S& operator<<( S& o, const std::vector<PARAMETER<double> >  &m)
{
  o << "(";

  for ( std::vector<PARAMETER<double> >::const_iterator ci=m.begin();
      ci!=m.end();)
  {
    o << " " << *(ci) << " ";
    ++ci;
  }
  o << ")";
  return o;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
