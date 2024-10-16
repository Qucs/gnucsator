/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
 * Author: Felix Salfelder
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
 */
#define NR_DOUBLE_SIZE 8 // get from qucs config?
#include "qucsator/precision.h"
#include "qucsator/complex.h"
//#include "qucsator/object.h"
//#include "qucsator/vector.h"
#include "qucsator/matrix.h"


// /*--------------------------------------------------------------------------*/
// /*--------------------------------------------------------------------------*/
// bool operator<(std::pair<double, qucs::matrix> const& a,
//                std::pair<double, qucs::matrix> const& b)
// { untested();
//   return a.first<b.first;
// }
// /*--------------------------------------------------------------------------*/

#include "m_wave_.h"
#include <globals.h>
#include <e_compon.h>
#include <e_subckt.h>
#include <e_node.h>
#include <e_elemnt.h>
#include <u_nodemap.h>
#include <u_limit.h>
// #include <u_parameter_uf.h>

namespace qucs {

bool operator<(matrix const&, matrix const&)
{ untested();
  return false;
}

bool operator<(std::pair<double, matrix> const& a,
               std::pair<double, matrix> const& b)
{
  return a.first<b.first;
}

matrix zero(matrix const& m)
{
  int r = m.getRows();
  int c = m.getCols();
  trace2("zero", r, c);
  return matrix(r,c);
}

}

/*--------------------------------------------------------------------------*/
// components with one node are unlikely.
const size_t node_capacity_floor = 2;
/*--------------------------------------------------------------------------*/
static void grow_nodes(size_t Index, node_t*& n, size_t& capacity, size_t capacity_floor)
{
  if(Index < capacity){
  }else{
    size_t new_capacity = std::max(capacity, capacity_floor);
    while(new_capacity <= Index) {
      assert(new_capacity < new_capacity * 2);
      new_capacity *= 2;
    }
    node_t* new_nodes = new node_t[new_capacity];
    for(size_t i=0; i<capacity; ++i){
      new_nodes[i] = n[i];
    }
    delete[] n;
    n = new_nodes;
    capacity = new_capacity;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
template <class Iterator>
qucs::matrix interpolate(Iterator begin, Iterator end, double x)
//  ( double below, double above)
{
  qucs::matrix m = 0;
  qucs::matrix f1 = 0;
  if (begin == end) { untested();
    untested();
    throw Exception("interpolate table is empty");
  }else{
  }
  --end;
  if (begin == end) { // only 1 entry -- constant
    m = (*begin).second;
  }else{
    ++begin;

    // x is the search key
    // xx is the search key converted to a "pair" as required by upper_bound
    // upper might point to a match for the key, exact match
    // if not, it points just above it, no exact match
    // lower points just below where a match would go
    // so the real match is between upper and lower
    std::pair<double, qucs::matrix> xx(x,m);
    Iterator upper = upper_bound(begin, end, xx);
    Iterator lower = upper-1;

    // set f1 (derivative)
    if ((upper == end) && (x > (*upper).first)) {
      // x is out of bounds, above
      f1 = (*begin).second;
      f1 = zero(f1);
    }else if ((upper == begin) && (x < (*lower).first)) { itested();
      // x is out of bounds, below
      f1 = (*begin).second;
      f1 = zero(f1);
    }else if ((*upper).first <= (*lower).first) {itested();
      throw Exception("interpolate table is not sorted or has duplicate keys");
    }else{
      // ordinary interpolation
      assert((*upper).first != (*lower).first);
      f1 = ((*upper).second-(*lower).second) / ((*upper).first-(*lower).first);
    }

    m = (*lower).second + (x - (*lower).first) * f1;
  }
  return m;
}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class SPEMBED;
class COMMON_SPEMBED : public COMMON_COMPONENT {
  std::string _filename;
  WAVE_<qucs::matrix> _spwave;
  typedef enum{
    c_UNKNOWN = 0,
    c_RIR = 1,
    c_MA = 2
  } column_type;
  column_type _column{c_UNKNOWN};
public:
  explicit COMMON_SPEMBED(const COMMON_SPEMBED& p);
  explicit COMMON_SPEMBED(int c=0);
           ~COMMON_SPEMBED();
  bool     operator==(const COMMON_COMPONENT&)const override;
  COMMON_SPEMBED* clone()const override {return new COMMON_SPEMBED(*this);}
  void     set_param_by_index(int, std::string&, int)override;
  int      set_param_by_name(std::string, std::string)override;
  bool     param_is_printable(int)const override;
  std::string param_name(int)const override;
  std::string param_name(int,int)const override;
  std::string param_value(int)const override;
  int param_count()const override {
	  return 5 + COMMON_COMPONENT::param_count();
  }
  void precalc_first(const CARD_LIST*)override;
  void expand(const COMPONENT*)override;
  void precalc_last(const CARD_LIST*)override;
  void tr_eval_analog(SPEMBED*)const;
  void ac_eval_(SPEMBED*)const;
//  void precalc_analog(SPEMBED*)const;
  std::string name()const override {itested();return "SPembed";}
public: // input parameters
//  PARAMETER<mystring> _filename;
}cl; //COMMON_SPEMBED
/*--------------------------------------------------------------------------*/
class SPEMBED : public COMPONENT {
public:
  qucs::matrix _m;
private:
  size_t _node_capacity{0};
private: // construct
  explicit SPEMBED(SPEMBED const&);
public:
  explicit SPEMBED();
  ~SPEMBED() {
    delete [] _n;
  }
  CARD* clone()const override;
  std::string dev_type()const override{assert(has_common()); return common()->name();}
private:
  void tr_iwant_matrix()override { }
  void ac_iwant_matrix()override;
private: // COMPONENT
//  void	  tr_begin()override;
//  void	  tr_restore()override;
  void	  dc_advance()override;
//  void	  tr_advance()override;
//  void	  tr_regress()override;
// TIME_PAIR tr_review()override;
private: // overrides
  void precalc_first()override;
  void expand()override;
  void precalc_last()override;
  void	  ac_begin()override;
  void	  do_ac()override;
  void    ac_load() override;
//  double tr_probe_num(std::string const&)const override;
    //XPROBE  ac_probe_ext(CS&)const;//CKT_BASE/nothing
  int int_nodes()const override { return 0; }
  int min_nodes()const override { return 0; }
  int max_nodes()const override { return 10; }
  int net_nodes()const override {return _net_nodes;}
  int ext_nodes()const override { return _net_nodes;}
  int matrix_nodes()const override { return _net_nodes; }
  std::string value_name()const override {itested(); return "";}
  bool print_type_in_spice()const override {itested(); return false;}
  std::string port_name(int i)const override;
  void set_port_by_index(int Index, std::string& Value)override {
    grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
    trace2("spbn", Index, Value);
    COMPONENT::set_port_by_index(Index, Value);
  }
}; // SPEMBED
/*--------------------------------------------------------------------------*/
COMMON_SPEMBED::COMMON_SPEMBED(int c)
  :COMMON_COMPONENT(c)
{
}
/*--------------------------------------------------------------------------*/
COMMON_SPEMBED::COMMON_SPEMBED(const COMMON_SPEMBED& p)
  :COMMON_COMPONENT(p),
   _filename(p._filename), _spwave(p._spwave), _column(p._column)
{
  trace2("copy", _spwave.size(), p._spwave.size());
}
/*--------------------------------------------------------------------------*/
COMMON_SPEMBED::~COMMON_SPEMBED()
{
}
/*--------------------------------------------------------------------------*/
bool COMMON_SPEMBED::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_SPEMBED* p = dynamic_cast<const COMMON_SPEMBED*>(&x);
  return (p
    && _filename == p->_filename
    && _spwave.size() == p->_spwave.size()
    && COMMON_COMPONENT::operator==(x));
}
/*--------------------------------------------------------------------------*/
void COMMON_SPEMBED::set_param_by_index(int I, std::string& Value, int Offset)
{ untested();
	incomplete();
  switch (COMMON_SPEMBED::param_count() - 1 - I) {
  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);
  }
}
/*--------------------------------------------------------------------------*/
int COMMON_SPEMBED::set_param_by_name(std::string Name, std::string Value)
{
  trace2("spbn", Name, Value);
//   if(Name == "$mfactor"){ untested();
//     incomplete();
//     Name = "m";
//   }else{ untested();
//   }
  if(Name == "File") {
    _filename = Value;
    trace2("spbn", Name, std::string(_filename));
  }else{
    incomplete();
  }

  return 0; // incomplete();
}
/*--------------------------------------------------------------------------*/
bool COMMON_SPEMBED::param_is_printable(int i)const
{
  size_t idx = COMMON_SPEMBED::param_count() - 1 - i;
  if(idx == 0){
    return true;
  }else{
    incomplete();
    return COMMON_COMPONENT::param_is_printable(i);
  }
}
/*--------------------------------------------------------------------------*/
std::string COMMON_SPEMBED::param_name(int i)const
{
  size_t idx = COMMON_SPEMBED::param_count() - 1 - i;
  switch(idx){
  case 0:
    return "File";
  case 1:
    return "Data";
  case 2:
    return "Interpolator";
  case 3:
    return "Temp";
  case 4:
    return "duringDC";
  }
  return COMMON_COMPONENT::param_name(i);
}
/*--------------------------------------------------------------------------*/
std::string COMMON_SPEMBED::param_name(int i, int j)const
{ untested();
  if(j==0){ untested();
    return param_name(i);
  }else{ untested();
    return "";
  }
}
/*--------------------------------------------------------------------------*/
std::string COMMON_SPEMBED::param_value(int i)const
{
  size_t idx = COMMON_SPEMBED::param_count() - 1 - i;
  switch(idx){
  case 0:
    return "File";
  case 1:
    return "Data";
  case 2:
    return "Interpolator";
  case 3:
    return "Temp";
  case 4:
    return "duringDC";
  }
  return COMMON_COMPONENT::param_value(i);
}
/*--------------------------------------------------------------------------*/
void skipcom(CS& f)
{
  while(f.match1("!") || !f.more()){
    f.get_line("spfile>");
  }
}
/*--------------------------------------------------------------------------*/
void COMMON_SPEMBED::expand(const COMPONENT* d)
{
  double fscale = 1.;
  COMMON_COMPONENT::expand(d);
  trace1("common_spembed", std::string(_filename));
  CS f(CS::_INC_FILE, _filename);
  f.get_line("spfile>");

  skipcom(f);

// # HZ S RI R 50
  if(!(f>>"#")){ untested();
    f.check(bWARNING, "need '#'");
  }else{
  }
  if(f>>"HZ"){
  }else if(f>>"Hz"){
  }else if(f>>"GHz"){
    fscale = 1e9;
  }else{ untested();
    f.check(bWARNING, "need 'Hz' or 'GHz'");
  }
  if(!(f>>"S")){ untested();
    incomplete(); // Y, Z?
    f.check(bWARNING, "need 'S'");
  }else{
  }
  if(f>>"RI"){
    if(f>>"R"){
      _column = c_RIR;
    }else{ untested();
      f.check(bWARNING, "need 'R'");
    }
  }else if(f>>"MA"){
    if(f>>"R"){
      _column = c_MA;
    }else{ untested();
      f.check(bWARNING, "need 'R'");
    }
  }else{ untested();
    f.check(bWARNING, "need 'RI' or 'MA'"); // what else?
  }
  double impedance;
  if(!(f >> impedance)){ untested();
    f.check(bWARNING, "need impedance");
  }else if(!impedance){ untested();
    f.check(bWARNING, "need nonzero impedance");
  }else{
  }

  double key, real, arg1;
  int ref = d->net_nodes()-1;
  try{
    while(1){
      f.get_line("spfile>");
      skipcom(f);
      trace2("spembed", d->net_nodes(), f.fullstring().substr(0,20));

      f >> key;
      qucs::matrix m(ref);
      for(int ii=0; ii<ref; ++ii){
	for(int jj=0; jj<ref; ++jj){
	  if(!f.more()){
	    f.get_line("spfile-contd>");
	  }else{
	  }
	  f >> real;
	  if(!f.more()){ untested();
	    f.get_line("spfile-contd>");
	  }else{
	  }
	  f >> arg1;
	  nr_complex_t value(real, arg1);
	  if(_column == c_RIR){
	  }else if(_column == c_MA){
	    value = std::polar(real, arg1);
	  }else{ untested();
	    incomplete();
	  }
	  m.set(ii, jj, value);
	}
      }
      if(f.more()){ itested();
	throw Exception_CS("Trailing stuff. Need " +
	    to_string(2*ref*ref + 1) + " values.\n", f);
      }else{
      }
      _spwave.push(fscale*key, stoy(m, impedance));
      trace2("spembed", d->net_nodes(), _spwave.size());

    }
  }catch (Exception_End_Of_Input& e) {
  }

}
/*--------------------------------------------------------------------------*/
void COMMON_SPEMBED::precalc_first(const CARD_LIST* par_scope)
{
  assert(par_scope);
  trace1("cpf", std::string(_filename));
  if(_filename.size()<2){ untested();
  }else if(_filename[0] == '"'){
    _filename = _filename.substr(1, _filename.size()-2);
  }else{
  }
  COMMON_COMPONENT::precalc_first(par_scope);
}
/*--------------------------------------------------------------------------*/
void COMMON_SPEMBED::precalc_last(const CARD_LIST* par_scope)
{
  assert(par_scope);
  COMMON_COMPONENT::precalc_last(par_scope);
}
/*--------------------------------------------------------------------------*/
void COMMON_SPEMBED::ac_eval_(SPEMBED* p) const
{
  double f = std::imag(_sim->_jomega) / 2. / M_PI;
  trace2("ac_eval_", f, _spwave.size());
  qucs::matrix m = interpolate(_spwave.begin(), _spwave.end(), f);
  p->_m = m;
}
/*--------------------------------------------------------------------------*/
static COMMON_SPEMBED Default_spembed(CC_STATIC);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
//double SPEMBED::tr_probe_num(std::string const& n) const
//{ untested();
//  return NOT_VALID;
//}
/*--------------------------------------------------------------------------*/
SPEMBED::SPEMBED() : COMPONENT()
{
  attach_common(&Default_spembed);
}
/*--------------------------------------------------------------------------*/
// seq blocks
/*--------------------------------------------------------------------------*/
//bool SPEMBED::tr_needs_eval()const
//{ untested();
//  incomplete();
//  return false;
//};
/*--------------------------------------------------------------------------*/
//bool SPEMBED::do_tr()
//{ untested();
//  incomplete();
//  return true;
//}
/*--------------------------------------------------------------------------*/
// typedef SPEMBED::ddouble ddouble;
/*--------------------------------------------------------------------------*/
inline void SPEMBED::dc_advance()
{
  set_not_converged();
  return COMPONENT::dc_advance();
}
/*--------------------------------------------------------------------------*/
// inline void SPEMBED::tr_advance()
// { untested();
//   set_not_converged();
//   return ELEMENT::tr_advance();
// }
/*--------------------------------------------------------------------------*/
std::string SPEMBED::port_name(int i)const
{
  assert(i >= 0);
  assert(i < max_nodes());
  return "p" + to_string(i);
}
/*--------------------------------------------------------------------------*/
SPEMBED m_spembed;
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "SPfile", &m_spembed);
/*--------------------------------------------------------------------------*/
CARD* SPEMBED::clone()const
{
  return new SPEMBED(*this);
}
/*--------------------------------------------------------------------------*/
SPEMBED::SPEMBED(SPEMBED const&p) : COMPONENT(p)
{
  _node_capacity = net_nodes();
  if(_node_capacity){ untested();
    _n = new node_t[_node_capacity];
  }else{
    assert(_n == NULL);
  }
  if(p.is_device()){
    for (int ii = 0;  ii < net_nodes();  ++ii) { untested();
      _n[ii] = p._n[ii];
    }
  }else{ untested();
    for (int ii = 0;  ii < net_nodes();  ++ii) { untested();
      assert(!_n[ii].n_());
    }
  }
  assert(!subckt());
}
/*--------------------------------------------------------------------------*/
void SPEMBED::precalc_first()
{
  COMPONENT::precalc_first();
}
/*--------------------------------------------------------------------------*/
void SPEMBED::expand()
{
  trace1("expand", common());
  assert(common());
  COMPONENT::expand();
  trace1("expanded", common());
}
/*--------------------------------------------------------------------------*/
void SPEMBED::precalc_last()
{
  COMPONENT::precalc_last();
}
/*--------------------------------------------------------------------------*/
void SPEMBED::ac_begin()
{
  incomplete();
}
/*--------------------------------------------------------------------------*/
void SPEMBED::do_ac()
{
  auto cc = prechecked_cast<COMMON_SPEMBED const*>(common());
  assert(cc);
  cc->ac_eval_(this);
}
/*--------------------------------------------------------------------------*/
void SPEMBED::ac_load()
{
  int ref = net_nodes()-1;
  for (int ii=0; ii<ref; ++ii) {
    for (int jj=0; jj<ref; ++jj) {
      COMPLEX new_value = _m.get(ii, jj);
      // ELEMENT::ac_load_point
      _sim->_acx.load_point(_n[ii].m_(), _n[jj].m_(), mfactor() * new_value);
    }
  }

  if(_n[ref].is_grounded()){
  }else{
    COMPLEX sum=0.;
    COMPLEX sum2=0.;
    for (int ii=0; ii<ref; ++ii) {
      sum = 0;
      for (int jj=0; jj<ref; ++jj) {
	sum += _m.get(ii, jj);
	// ELEMENT::ac_load_point
      }
      _sim->_acx.load_point(_n[ii].m_(), _n[ref].m_(), - mfactor() * sum);
      sum2 += sum;
    }
    for (int jj=0; jj<ref; ++jj) {
      sum = 0;
      for (int ii=0; ii<ref; ++ii) {
	sum += _m.get(ii, jj);
	// ELEMENT::ac_load_point
      }
      _sim->_acx.load_point(_n[ref].m_(), _n[jj].m_(), - mfactor() * sum);
      sum2 += sum;
    }
    _sim->_acx.load_point(_n[ref].m_(), _n[ref].m_(), mfactor() * sum2);

  }
}
/*--------------------------------------------------------------------------*/
// ELEMENT::ac_iwant_matrix_extended()
void SPEMBED::ac_iwant_matrix()
{
  assert(is_device());
  assert(!subckt());
  assert(ext_nodes() + int_nodes() == matrix_nodes());

  for (int ii = 0;  ii < matrix_nodes();  ++ii) {
    if (_n[ii].m_() >= 0) {
      for (int jj = 0;  jj < ii ;  ++jj) {
	_sim->_acx.iwant(_n[ii].m_(),_n[jj].m_());
      }
    }else{itested();
      // node 1 is grounded or invalid
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
