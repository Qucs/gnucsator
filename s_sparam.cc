/*                            -*- C++ -*-
 * Copyright (C) 2018 Felix Salfelder
 * Author: Felix Salfelder <felix@salfelder.org>
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
 * s-param analysis
 */
#include "u_sim_data.h"
#include "u_status.h"
#include "u_parameter.h"
#include "u_prblst.h"
#include "s__.h"
#include "io.h"
#include "globals.h"
#include "e_subckt.h"
#include "s__init.cc"

typedef unsigned needed_t;

// #include "io_matrix.h"
#include "e_node.h"
#include "e_aux.h"
#include "e_elemnt.h"
#include <gsl/gsl_blas.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_linalg.h>
// #include "io.h"

static bool hack_no_load=false;
/*--------------------------------------------------------------------------*/
#ifndef HAVE_UINT_T
typedef int uint_t;
#endif
/*--------------------------------------------------------------------------*/
void SIM::outdata(double, int){ incomplete(); }
void SIM::head(double, double, const std::string&){ }
void SIM::print_results(double) {}
void SIM::alarm() {}
void SIM::store_results(double) {}
//void SIM::plot_results(double) {}
void SIM::setup(CS&){}
//void SIM::sweep(){}
//void SIM::reset_timers(){}
// SIM::~SIM(){}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class PAC : public ELEMENT {
private:
  explicit PAC(const PAC& p) : ELEMENT(p), _pwr(p._pwr), _num(p._num) {}
public:
  explicit PAC()		:ELEMENT(), _num(-1u) {}
private: // override virtual
  char	   id_letter()const override{return '\0';}
  std::string value_name()const override{return "Z";}
  std::string dev_type()const override{return "pac_";}
  uint_t	   max_nodes()const override{return 2;}
  uint_t	   min_nodes()const override{return 2;}
  uint_t	   matrix_nodes()const override{return 2;}
  uint_t	   net_nodes()const override{return 2;}
  CARD*	   clone()const	override{return new PAC(*this);}
  void	   tr_iwant_matrix()override	{}
  void	   ac_iwant_matrix()override	{}
  void     precalc_last()override;
  double   tr_involts()const override   {return dn_diff(_n[IN1].v0(), _n[IN2].v0());}
  double   tr_involts_limited()const override{return tr_involts();}
  double   tr_probe_num(const std::string&)const override;

  std::string port_name(uint_t i)const override{ untested();
    assert(i < 2);
    static std::string names[] = {"p", "n"};
    return names[i];
  }
private:
  int set_param_by_name(std::string a, std::string b) override{
    trace2("PAC::set_param_by_name", a, b);
    if(a=="Z"){
      set_value(b);
    }else if(a=="P"){
      _pwr = b;
    }else if(a=="Num"){
      _num = b;
    }else{ untested();
    }
    return 0; // TODO
  }
public:
  unsigned num() const{
    return _num;
  }
  COMPLEX  ac_involts()const override{return -ac_outvolts();}
  void stamp_rhs(){
    _acg = 1;
    ac_load_source();
  }

  void ac_load() override{
    double Z = value();
    double g = 1/Z;
    double I = std::sqrt(8 * _pwr / Z);
    trace3("PAC::ac_load", I, g, _pwr);
    _acg = I;
    ac_load_source();
    _acg = g;

    if(hack_no_load){
    }else{
      ac_load_passive();
    }
//    Iac #(.I(I)) .. (n, p);
//    R #(.r({Z})) R1(i 2);
  }
  double impedance() const{
    trace1("imped", value());
    return value();
  }
private:
  PARAMETER<double> _pwr;
  PARAMETER<unsigned> _num;
}pp;
static DISPATCHER<CARD>::INSTALL d(&device_dispatcher, "pac_", &pp);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void PAC::precalc_last()
{
  ELEMENT::precalc_last();
  set_constant(true);
  set_converged();
  if(value()<=0){ untested();
    error(bPICKY, long_label()+": setting default impedance, 50Ohm\n");
    set_value(50);
  }else{
    trace0("have value");
  }
  _num.e_val(0., scope());
  _pwr.e_val(1., scope());
  trace1("PAC::precalc_last", _pwr);
  assert(_num!=-1u);
}
/*--------------------------------------------------------------------------*/
double PAC::tr_probe_num(const std::string& x)const
{ untested();
  if (Umatch(x, "gain ")) { untested();
    return tr_outvolts() / tr_involts();
  }else{ untested();
    return ELEMENT::tr_probe_num(x);
  }
}
/*--------------------------------------------------------------------------*/
class SPARAM : public SIM {
private:
  enum TYPE {
    tZ      = 0,
    tY      = 1,
    tS      = 2
  };
public:
  void do_it(CS&, CARD_LIST*) override;

  explicit SPARAM():
    SIM(),
    _start(),
    _stop(),
    _step_in(),
    _step(0.),
    _linswp(false),
    _prevopppoint(false),
    _stepmode(ONE_PT),
    _dump_matrix(0),
    _type(tS),
    _Z(NULL)
  {}

  ~SPARAM() {}
private:
  explicit SPARAM(const SPARAM&):SIM() {unreachable(); incomplete();}
  void sweep() override;
  void first();
  bool next();
  bool next_freq();
  void solve();
  void clear();
  void hack_findall( CARD_LIST* scope);
  void setup(CS&) override;
  void outmatrix(gsl_matrix_complex const* M);
  void outdata(double, int) override;
  void store_results(double x) override;
  void flush();
private:
  OMSTREAM _out; // tmp hack
  std::vector<PAC*> _ports;
  PARAMETER<double> _start;	// sweep start frequency
  PARAMETER<double> _stop;	// sweep stop frequency
  PARAMETER<double> _step_in;	// step size, as input
  double _step;			// printed step size
  bool	_linswp;		// flag: use linear sweep (vs log sweep)
  bool	_prevopppoint;  	// flag: use previous op point
  enum {ONE_PT, LIN_STEP, LIN_PTS, TIMES, OCTAVE, DECADE} _stepmode;
  bool _dump_matrix; // dump matrix after ac
  struct output_t{
    std::string label;
    CKT_BASE* brh[2];
  };
private:
  TYPE _type;
  gsl_matrix_complex *_Z;
  std::vector<std::vector<COMPLEX> > _data;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void SPARAM::do_it(CS& Cmd, CARD_LIST* Scope)
{
  _scope = Scope;
  _sim->set_command_ac();
  reset_timers();
  ::status.ac.reset().start();

  _sim->init(_scope);
  _sim->alloc_vectors();
  _sim->_acx.reallocate();
  _sim->_acx.set_min_pivot(OPT::pivtol);

  setup(Cmd);
  ::status.set_up.stop();
  switch (ENV::run_mode) {
  case rPRE_MAIN:	unreachable();	break;
  case rBATCH:
  case rINTERACTIVE:
  case rSCRIPT:		sweep();	break;
  case rPRESET:		/*nothing*/	break;
  }

  if(_Z){
    gsl_matrix_complex_free(_Z);
    _Z = NULL;
  }else{ untested();
  }
  _sim->_acx.unallocate();
  _sim->unalloc_vectors();

  ::status.ac.stop();
  ::status.total.stop();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static int needslinfix;	// flag: lin option needs patch later (spice compat)
/*--------------------------------------------------------------------------*/
void SPARAM::hack_findall( CARD_LIST* scope){
  for (CARD_LIST::iterator i = scope->begin(); i != scope->end(); ++i) {
    if ( PAC* c = dynamic_cast< PAC*>(*i) ) {
      trace1("found port", c->long_label());
      _ports.push_back( c );
    }else if (!(*i)->is_device()){
        // model, perhaps
    } else if ( BASE_SUBCKT* s = dynamic_cast< BASE_SUBCKT*>(*i) ) {
      trace1("going down", s->long_label());
      hack_findall( s->subckt() );
    }
  }
}
/*--------------------------------------------------------------------------*/
void SPARAM::setup(CS& Cmd)
{
  _out = IO::mstdout;
  _out.reset(); //BUG// don't know why this is needed
  _ports.clear();

// FIXME: freq-range?
//
  std::string output;

  size_t here = Cmd.cursor();
  do{
    if (Cmd.match1("'\"({") || Cmd.is_float()) {
      Cmd >> _start;
      trace1("SPARAM got start", _start);
      if (Cmd.match1("'\"({") || Cmd.is_float()) {
        Cmd >> _stop;
      }else{ untested();
        _stop = _start;
        trace1("SPARAM got stop", _stop);
      }
      if (Cmd.match1("'\"({") || Cmd.is_float()) { untested();
        _stepmode = LIN_STEP;
        Cmd >> _step_in;
        trace1("SPARAM got step", _step);
      }else{
      }
    }else{
      incomplete();
    }

    std::string port;
    if (Cmd.umatch("port")){
      trace2("findbranch", port, Cmd.tail());
      size_t arg1 = Cmd.cursor();
      CARD_LIST::fat_iterator ci = findbranch(Cmd, &CARD_LIST::card_list);
#if 1
      hack_findall(&CARD_LIST::card_list);
      assert(_ports.size());
      (void) arg1;
      (void) ci;
#else
      if (ci.is_end()){ untested();
        Cmd.reset(arg1);
        throw Exception_CS("cannot find port", Cmd);
      }else{ untested();
        unsigned next = Cmd.cursor();
        for(;!ci.is_end();){ untested();
          trace1("have", (*ci)->long_label());
          if(PAC* P=dynamic_cast<PAC*>(*ci)){ untested();
            error(bDANGER, (*ci)->long_label() + " is a port\n");
            _ports.push_back(P);
          }else{ untested();
            error(bDANGER, (*ci)->long_label() + " is not a port, skipping\n");
          }

          Cmd.reset(arg1);
          ci = findbranch(Cmd, ci);
        }
        Cmd.reset(next);
      }
#endif
      trace2("done ports", Cmd.tail(), _ports.size());
    }else{

    }
    //try{ untested();
    //  _output.add_list(Cmd);
    //}
    //catch(Exception_Cant_Find)
    //{}
    ONE_OF
      || (Get(Cmd, "*",		  &_step_in) && (_stepmode = TIMES))
      || (Get(Cmd, "+",		  &_step_in) && (_stepmode = LIN_STEP))
      || (Get(Cmd, "by",	  &_step_in) && (_stepmode = LIN_STEP))
      || (Get(Cmd, "step",	  &_step_in) && (_stepmode = LIN_STEP))
      || (Get(Cmd, "d{ecade}",	  &_step_in) && (_stepmode = DECADE))
      || (Get(Cmd, "ti{mes}",	  &_step_in) && (_stepmode = TIMES))
      || (Get(Cmd, "lin",	  &_step_in) && (_stepmode = LIN_PTS))
      || (Get(Cmd, "o{ctave}",	  &_step_in) && (_stepmode = OCTAVE))
      || Get(Cmd, "sta{rt}",	  &_start)
      || Get(Cmd, "sto{p}",	  &_stop)
      || Get(Cmd, "dm",	          &_dump_matrix)
      || (Cmd.umatch("paramtype {=}") &&
          (ONE_OF
           || Set(Cmd, "z", &_type, tZ)
           || Set(Cmd, "y", &_type, tY)
           || Set(Cmd, "s", &_type, tS)
           || Cmd.warn(bWARNING, "need z, y, s")
          )
         )
      || outset(Cmd, &_out);
    ;
  }while (Cmd.more() && !Cmd.stuck(&here));
  Cmd.check(bWARNING, "what's this??");

  switch (_stepmode) {
  case ONE_PT:
  case LIN_STEP:
    needslinfix = false;
    _linswp = true;
    break;
  default:
    break;
  }

  size_t size=_ports.size();
  assert(!_Z);
  _Z = gsl_matrix_complex_alloc(size, size);
  _data.resize(0);
  _data.resize(size*size+1);

  _start.e_val(0., _scope);
  trace1("eval start", _start);
  _stop.e_val(0., _scope);
  _step_in.e_val(0., _scope);
  _step = _step_in;

  if (needslinfix) {untested();		// LIN option is # of points.
    assert(_step >= 2);			// Must compute step after 
    _step=(_stop-_start)/(_step-1.);	// reading start and stop,
    needslinfix = false;		// but step must be read first
  }else{			// for Spice compatibility
  }		
  if (_step==0.) { untested();
    _step = _stop - _start;
    _linswp = true;
  }else{
  }

  incomplete();
//  initio(_out);

//  if(!_output.size()){ untested();
//    throw(Exception("no output specified"));
//  }
  trace2("eval done", _start, _stop);
}
/*--------------------------------------------------------------------------*/
void SPARAM::solve()
{
  _sim->_acx.zero();
  std::fill_n(_sim->_ac, _sim->_total_nodes+1, 0.);

  ::status.load.start();
  _sim->count_iterations(iTOTAL);
  CARD_LIST::card_list.do_ac();
  CARD_LIST::card_list.ac_load();
  ::status.load.stop();

  if (_dump_matrix){ untested();
    _out.setfloatwidth(0,0);
    incomplete();
    // _out << _sim->_acx << "\n" ;
  }else{
  }
  ::status.lud.start();
  _sim->_acx.lu_decomp();
  ::status.lud.stop();

  return;
}
/*--------------------------------------------------------------------------*/
void SPARAM::outdata(double x, int)
{
  store_results(x);
}
/*--------------------------------------------------------------------------*/
void SPARAM::store_results(double x)
{
  unsigned n=1;
  _data[0].push_back(x);
  for(unsigned i=0; i< _Z->size1; ++i){
    for(unsigned j=0; j< _Z->size2; ++j){
      assert(n<_data.size());
      gsl_complex vv=gsl_matrix_complex_get(_Z, i, j);
      COMPLEX v=COMPLEX(GSL_REAL(vv), GSL_IMAG(vv));
      trace4("..", n, _data.size(), x, v);
      _data[n].push_back(v);
      ++n;
    }
  }
}
/*--------------------------------------------------------------------------*/
template<class T, class O>
void qfl(T const& t, O& o, size_t){
  for( auto i : t){
//    o << i << "\n";
    char s='+';
    double v=i.imag();
    if(v<0){
      s = '-';
      v = -v;
    }
    o << i.real() << s << "j" << v << "\n";
  }
}
/*--------------------------------------------------------------------------*/
void SPARAM::flush()
{
  trace1("flush", _data.size());
  _out << "<indep frequency " << _data[0].size() << ">\n";
  for( auto i : _data[0]){
    _out << i.real() << "\n";
  }
  _out << "</indep>\n";
  size_t size=_ports.size();

  for(unsigned i=1; i<_data.size(); ++i) {
    size_t a=(i-1)/_ports.size();
    size_t b=(i-1)%_ports.size();
    a = _ports[a]->num();
    b = _ports[b]->num();
    _out << "<dep S[" << a << "," << b << "] frequency" << ">\n";
    qfl(_data[i], _out, size);
    _out << "</dep>\n";
  }

}
/*--------------------------------------------------------------------------*/
void SPARAM::outmatrix(gsl_matrix_complex const* M)
{ untested();
  for(unsigned i=0; i< M->size1; ++i){ untested();
    for(unsigned j=0; j< M->size2; ++j){ untested();
      gsl_complex vv=gsl_matrix_complex_get(M, i, j);
//      COMPLEX x=COMPLEX(GSL_REAL(vv), GSL_IMAG(vv));
//      _out << x << " ";
      _out << GSL_REAL(vv) << " + j" << GSL_IMAG(vv) << " ";
    }
    _out <<"\n";
  }
}
/*--------------------------------------------------------------------------*/
static void mul_rows(gsl_matrix_complex* M, double const* s)
{
  for(unsigned i=0; i< M->size1; ++i){
    for(unsigned j=0; j< M->size2; ++j){
      gsl_complex vv=gsl_matrix_complex_get(M, i, j);
      vv = gsl_complex_mul_real(vv, s[i]);
      gsl_matrix_complex_set(M, i, j, vv);
    }
  }
}
/*--------------------------------------------------------------------------*/
static void mul_cols(gsl_matrix_complex* M, double const* s)
{
  for(unsigned i=0; i< M->size1; ++i){
    for(unsigned j=0; j< M->size2; ++j){
      gsl_complex vv=gsl_matrix_complex_get(M, i, j);
      vv = gsl_complex_mul_real(vv, s[j]);
      gsl_matrix_complex_set(M, i, j, vv);
    }
  }
}
/*--------------------------------------------------------------------------*/
static void add_dia(gsl_matrix_complex* M, double const s)
{
  for(unsigned i=0; i< M->size1; ++i){
    gsl_complex vv=gsl_matrix_complex_get(M, i, i);
    vv = gsl_complex_add_real(vv, s);
    gsl_matrix_complex_set(M, i, i, vv);
  }
}
/*--------------------------------------------------------------------------*/
void SPARAM::sweep()
{
  int width = std::min(OPT::numdgt+5, BIGBUFLEN-10);
  char format[20];
  //sprintf(format, "%%c%%-%u.%us", width, width);
  sprintf(format, "%%c%%-%us", width);

//  _out.form(format, '*', "param");
  sprintf(format, "%%-%us", width);
  head(_start, _stop, "@freq");
  first();
  trace3("eval done", _start, _stop, _sim->_freq);
  CARD_LIST::card_list.ac_begin();

  do {
//    _out << "...." << _sim->_freq << "\n";
    _sim->_jomega = COMPLEX(0., _sim->_freq * M_TWO_PI);
    hack_no_load = true;
    solve();
    hack_no_load = false;
    unsigned ii = 0;
    for(auto& in : _ports){
      std::fill_n(_sim->_ac, _sim->_total_nodes+1, 0.);
      in->stamp_rhs();

      ::status.back.start();
      _sim->_acx.fbsub(_sim->_ac);
      ::status.back.stop();

      unsigned j=0;
      for(auto const& out: _ports){
        COMPLEX v=out->ac_involts();
        gsl_complex vv;
        GSL_SET_COMPLEX(&vv, v.real(), v.imag());
        gsl_matrix_complex_set(_Z, ii, j, vv);
        if(_type==tZ){ untested();
          _out << v << " ";
        }else{
        }
        ++j;
      }
      if(_type==tZ){ untested();
        std::cout << "\n";
      }else{
      }
      ++ii;
    }
    if(_type==tY){ untested();
      unreachable();
      size_t size=_ports.size();
      gsl_permutation *p = gsl_permutation_alloc(size); // FIXME: not here
      gsl_matrix_complex *Y = gsl_matrix_complex_alloc(size, size); // FIXME: not here
      int s;

      gsl_linalg_complex_LU_decomp (_Z, p, &s);
      gsl_linalg_complex_LU_invert (_Z, p, Y); // FIXME. compute row by row.
      // output Z^-1 e_1 ... Z^-1 e_k ...

      outmatrix(Y);

      gsl_matrix_complex_free(Y);
      gsl_permutation_free(p);

    }else if(_type==tS){
      size_t size=_ports.size();
      trace1("S", size);
      std::vector<double> sy0(size);
      unsigned i=0;
      for(auto p : _ports){
        if(!p->impedance()){
          error(bDANGER, "zero impedance in " + p->long_label() + "\n");
        }else{
        }
        double yy = 1./p->impedance();
        assert(yy>0);
        sy0[i++] = sqrt(yy);
      }

      mul_rows(_Z, sy0.data());
      mul_cols(_Z, sy0.data());
      // outmatrix(_Z);

      // sY = sqrt(Y_0)
      // S = ( sY Z sY - 1 ) * ( sY Z sY + 1 )^-1
      //   = ( sY Z sY + 1 )^-1 * ( sY Z sY - 1 )
      //   = A^-1 * B
      //
      gsl_permutation *p = gsl_permutation_alloc(size); // FIXME: not here
      gsl_matrix_complex *A = gsl_matrix_complex_alloc(size, size); // FIXME: not here
      gsl_matrix_complex *B = gsl_matrix_complex_alloc(size, size); // FIXME: not here
      gsl_matrix_complex_memcpy(A, _Z); // A=_Z;
      gsl_matrix_complex_memcpy(B, _Z); // B=_Z;
      add_dia(A, 1.);
      add_dia(B, -1.);

      int s;
      gsl_linalg_complex_LU_decomp (A, p, &s);

      gsl_matrix_complex* S=_Z;

      for(unsigned k=0; k< _Z->size1; ++k){
        gsl_vector_complex_const_view b = gsl_matrix_complex_const_column(B, k);
        gsl_vector_complex_view v = gsl_matrix_complex_row(S, k);
        gsl_linalg_complex_LU_solve (A, p, &b.vector, &v.vector); // FIXME. compute row by row.
      }

      //outmatrix(S);
      outdata(_sim->_freq, 0);

    }else{ untested();
    }
  } while (next_freq());

  flush();
}
/*--------------------------------------------------------------------------*/
void SPARAM::first()
{
  trace1("SPARAM::first", _start);
  _sim->_freq = _start;
}
/*--------------------------------------------------------------------------*/
bool SPARAM::next_freq()
{
  double realstop = (_linswp)
    ? _stop - _step/100.
    : _stop / pow(_step,.01);
  if (!in_order(double(_start), _sim->_freq, realstop)) {
    return false;
  }else{
  }

  _sim->_freq = (_linswp)
    ? _sim->_freq + _step
    : _sim->_freq * _step;

  trace2("SPARAM::next_freq", _sim->_freq, _linswp);
  if (in_order(_sim->_freq, double(_start), double(_stop))) { untested();
    return false;
  }else{
    return true;
  }
}
/*--------------------------------------------------------------------------*/
static SPARAM p1;
static DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "sparam|sp", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:et:
