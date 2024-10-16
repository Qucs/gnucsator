// (C) 2020 Felix Salfelder
//     GPLv3+
//
#ifndef QW_COMPONENT_H
#define QW_COMPONENT_H

#include <io_trace.h>
#include <e_compon.h>
#include <e_node.h>
#include <l_denoise.h> // dn_diff
#include <cmath>
#include <complex>
#include <string.h>
#include "../q_paramlist.h"
#include <e_paramlist.h> // "../q_paramlist.h"

typedef double nr_double_t;
typedef std::complex<double> nr_complex_t;

#define node_t qucs_node_t
#define NR_TINY 1e-12 // yikes.
#include "matrix.h"
#include "real.h"
#include "complex.h"
#include "constants.h"
#include "consts.h"
#include "netdefs.h"
#include "logging.h"
#undef node_t

// struct property_t required;
#include "microstrip/substrate.h" // BUG

#define CREATOR(a) \
public: \
	static define_t cirdef; \
	define_t* cd() const override {return &cirdef;} \
	a(); \
	a( const a& p) : circuit(p){incomplete();} \
	COMPONENT* clone()const override{ a* n=new a(*this); n->init(); return n; }


namespace qucs{

// these are not used. perhaps not useful.
static const int CIR_BONDWIRE = 0;
static const int CIR_CIRCULARLOOP = 0;
static const int CIR_CPWGAP = 0;
static const int CIR_CPWLINE = 0;
static const int CIR_CPWOPEN = 0;
static const int CIR_CPWSHORT = 0;
static const int CIR_CPWSTEP = 0;
static const int CIR_MSCORNER = 0;	
static const int CIR_MSCOUPLED = 0;
static const int CIR_MSCROSS = 0;	
static const int CIR_MSGAP = 0;	
static const int CIR_MSLANGE = 0;
static const int CIR_MSLINE = 0;
static const int CIR_MSMBEND = 0;
static const int CIR_MSOPEN = 0;
static const int CIR_MSRSTUB = 0;	
static const int CIR_MSSTEP = 0;	
static const int CIR_MSTEE = 0;
static const int CIR_MSVIA = 0;	
static const int CIR_SPIRALIND = 0;	

static const nr_double_t z0 = 50.0; // yikes.

static const int LOG_STATUS = bLOG;


typedef enum {
	NODE_1 = 0,
	NODE_2 = 1,
	NODE_3 = 2,
	NODE_4 = 3,
	NODE_5 = 4,
	NODE_6 = 5 } node_number;

typedef enum {
	VSRC_1 = 0,
	VSRC_2 = 1,
	VSRC_3 = 2,
	VSRC_4 = 3,
	VSRC_5 = 4 } vsrc_number;

static COMMON_PARAMLIST Default_PARAMS(CC_STATIC);

class circuit : public COMPONENT{
protected:
	circuit(circuit const& c) : COMPONENT(c), _num_ports(c._num_ports) {
		assert(_num_ports);
		assert(!_n);
		_n = new node_t[_num_ports];
		if(c._n){ untested();
			for(int i=0; i<_num_ports; ++i){ untested();
				_n[i] = c._n[i];
			}
		}else{
			// eek
		}
//		assert(!_matrix);
//		_matrix = new DPAIR[_num_ports*_num_ports];
//
	};
public:
	circuit(int n) : COMPONENT(), _num_ports(n) {
		assert(!_n);
		assert(n);
//		// BUG. init
//      attach_common(&Default_PARAMS);
	}
	~circuit() {
		// BUG: memory leak
		// incomplete();
//		for(auto i : _p){
//			delete i;
//		}
		delete [] _matrix;
		delete [] _n;
	}
private:
	virtual define_t* cd() const = 0;
public:
	void init();
protected: // qucsator globals
	std::string getName() const{ untested();
		return long_label();
	}
	int createInternal(std::string, std::string){ untested();
		incomplete();
		return -1;
	}
	void setNode(node_number, int){ untested();
		incomplete();
	}
	double getPropertyDouble(std::string const& s){
		COMMON_PARAMLIST const* c = prechecked_cast<COMMON_PARAMLIST const*>(common());
		assert(c);
		auto pp = c->_params.find(s);
		if(pp == c->_params.end()){ untested();
			incomplete();
			assert(false);
			return 0.;
		}else{
			trace3("getPropertyDouble", s, pp->second.string(), double(pp->second));
			return pp->second;
		}
		
#if 0
		auto i = _pn.find(s);
		if(i == _pn.end()){ untested();
			assert(false);
			return 0.;
		}else if(auto ps = dynamic_cast<PARAMETER<double> const*>(i->second)){
			assert(*ps == *ps);
			assert(ps->has_good_value());
			return *ps;
		}else{ untested();
			assert(false);
			return 0;
		}
#endif
	}
	const char* getPropertyString(std::string const& s){
		trace1("getPropertyString", s);
		COMMON_PARAMLIST const* c = prechecked_cast<COMMON_PARAMLIST const*>(common());
		assert(c);
		PARAM_LIST::const_iterator pp = c->_params.find(s);
		if(pp == c->_params.end()){ untested();
			unreachable();
			incomplete();
			return NULL;
		}else{
			return strdup(pp->second.string().c_str());
		}
	}
	void setCharacteristic (std::string const&, double const&){ untested();
		incomplete();
	}
	substrate /*const*/ * getSubstrate(){
		return _substrate;
	}
   void setVoltageSources(double) {incomplete();}
   void setInternalVoltageSource(double) {incomplete();}
   void allocMatrixMNA () {incomplete();}
   void allocMatrixS () {incomplete();}
   void allocMatrixN () {incomplete();}
	matrix getMatrixS() {incomplete(); return matrix();}
	matrix getMatrixY() {incomplete(); return matrix();}
	void setMatrixN (matrix) {incomplete();}
	void setMatrixS (matrix) {incomplete();}
	void setMatrixY (matrix const& y) {
		int r = y.getRows ();
		int c = y.getCols ();

		assert(r==_num_ports);
		assert(c==_num_ports);
		// copy matrix elements
		if (r > 0 && c > 0 && r * c == _num_ports * _num_ports) {
			memcpy (_matrix, y.getData (), sizeof (nr_complex_t) * r * c);
		}
	}
	void clearY (){incomplete();}
	int getSize() const {return _num_ports;}

// misc sources?
	void setB (node_number, vsrc_number, nr_complex_t){ incomplete(); }
	void setC (vsrc_number, node_number, nr_complex_t){ incomplete(); }
	void setD (vsrc_number, vsrc_number, nr_complex_t){ incomplete(); }
	void setE (vsrc_number, nr_complex_t){ incomplete(); }
// noise matrix?
	void setN (node_number, node_number, nr_complex_t){ incomplete(); }
//	load_ac?
	void setS (node_number, node_number, nr_complex_t){ incomplete(); }
// acrhs?
	void setY (node_number ii, node_number jj, nr_complex_t x){
		assert(ii<_num_ports);
		assert(jj<_num_ports);
		assert(_matrix);
		DPAIR& dp = _matrix[ii*_num_ports+jj];
		if(x==x){
		}else{ untested();
			trace3("setY", ii, jj, x);
		}
		assert(x==x);
		dp.first = x.real();
		dp.second = x.imag();
	}
   void voltageSource (vsrc_number, node_number, node_number){ incomplete(); }


private: // "circuit"
	virtual void calcAC(nr_double_t){ unreachable(); }
	virtual void initDC(){ unreachable(); }

public: // used in mstee.
	void initSP(){ untested();
		incomplete();
	}
	void initNoiseSP(){ untested();
		incomplete();
	}

private: // COMPONENT
	void precalc_first() override{
		trace2("precalc_first fwd", long_label(), common());
		COMPONENT::precalc_first();

		CARD_LIST* Scope=scope();
		mutable_common()->precalc_first(Scope);
		mutable_common()->precalc_last(Scope);

		//for(unsigned s=0; s<_p.size(); ++s){
		//	if(auto ps = dynamic_cast<PARAMETER<double>*>(_p[s])){
		//		ps->e_val(NOT_VALID, Scope);
		//		trace3("param", s, ps->string(), *ps);
		//	}else{
		//	}
		//}
	}
	void expand() override{
		trace2("expand fwd", long_label(), common());
		COMPONENT::expand();

		// this only makes sense for MS stuff.
		std::string s = getPropertyString("Subst");
		_substrate = dynamic_cast<substrate*>(find_in_my_scope(s));
		assert(_substrate || s==""); // for now.. TODO
	}
	void precalc_last() override{
		COMPONENT::precalc_last();
		CARD_LIST* Scope=scope();
		mutable_common()->precalc_last(Scope);
	}
	void tr_iwant_matrix() override {tr_iwant_matrix_extended();}
	void ac_iwant_matrix() override {ac_iwant_matrix_extended();}
	void tr_iwant_matrix_extended() {
		assert(is_device());
		assert(!subckt());
		trace3("iwant", ext_nodes(), int_nodes(), matrix_nodes());
		assert(ext_nodes() + int_nodes() == matrix_nodes());

		for (int ii = 0;  ii < matrix_nodes();  ++ii) {
			if (_n[ii].m_() >= 0) {
				for (int jj = 0;  jj < ii ;  ++jj) {
					_sim->_aa.iwant(_n[ii].m_(),_n[jj].m_());
					_sim->_lu.iwant(_n[ii].m_(),_n[jj].m_());
				}
			}else{itested();
				// node 1 is grounded or invalid
			}
		}
	}
	void ac_iwant_matrix_extended() {
		assert(is_device());
		assert(!subckt());
		trace3("iwant", ext_nodes(), int_nodes(), matrix_nodes());
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
	bool do_tr() override{ untested();
		incomplete();
		return true;
	}
	void tr_load() override{
		incomplete();
	}
	void ac_load() override{
		for (int ii=0; ii < matrix_nodes(); ++ii) {
			//		ac_load_source_point(_n[ii], COMPLEX(_i0[ni], _i1[ni])); // TODO
			for (int jj = 0; jj < matrix_nodes(); ++jj) {
				DPAIR& dp = _matrix[ii*_num_ports+jj];
				trace4("ac_load", ii, jj, dp.first, dp.second);
				ac_load_point(_n[ii], _n[jj], COMPLEX(dp.first, dp.second));
			}
		}
	}
   void ac_begin() override{
		trace1("circuit::ac_begin", _sim->_jomega.imag());
		trace1("circuit::ac_begin", _sim->_jomega.real());
		incomplete();
	}
   void tr_begin() override{
		initDC();
		incomplete();
	}
   void do_ac() override{
		double freq = _sim->_jomega.imag() / M_TWO_PI;
		trace1("circuit::do_ac", _sim->_jomega.imag());
		trace2("circuit::do_ac", _sim->_freq, freq);
		trace1("circuit::do_ac", _sim->_jomega.real());
		calcAC (freq);
	}
   int param_count()const override{
		// assert(_num_param = c->_params.size()); //?
		return COMPONENT::param_count(); //  + int(_num_param);
	}
#if 0
   bool param_is_printable(int i)const override{ untested();
		int s = circuit::param_count() - 1 - i;
		if(0 <=s && s < _num_param){
			return true;
		}else{
			return COMPONENT::param_is_printable(i);
		}
	}
#endif
	void set_param_by_index(int i, std::string& b, int j) override{ untested();
		trace3("circuit::set_param_by_index", i, b, j);
		int s = circuit::param_count() - 1 - i;
		if(s < _num_param){ untested();
			incomplete();
			// *_p[s] = b;
		}else{ untested();
			COMPONENT::set_param_by_index(i, b, j);
		}
	}
	int set_param_by_name(std::string a, std::string b)override {
		trace2("circuit::set_param_by_name", a, b);
		return COMPONENT::set_param_by_name(a, b);
	}
#if 0
	{
		auto i = _pn.find(a);
		if(i == _pn.end()){ untested();
			incomplete();
			// throw
		}else{
			*i->second = b;
		}
		trace2("spbn", a, b);
	}
	std::string param_name(int i)const{
		int s = circuit::param_count() - 1 - i;
		if(s < _num_param){
			assert(s < int(_pnames.size()));
			return *_pnames[s];
		}else{ untested();
			return COMPONENT::param_name(i);
		}
	}
	std::string param_name(int i,int j)const{ untested();
		if(j==0){ untested();
			return param_name(i);
		}else{ untested();
			incomplete();
			return "dunno";
		}
	}
#endif
#if 0
	std::string param_value(int i)const override{
		int s = circuit::param_count() - 1 - i;
		if(s >= int(_p.size())){ untested();
			return COMPONENT::param_name(i);
		}else if(auto ps = dynamic_cast<PARAMETER<double> const*>(_p[s])){
			assert(s < int(_p.size()));
			return ps->string();
		}else if(auto ps = dynamic_cast<PARAMETER<mystring> const*>(_p[s])){
			assert(s < int(_p.size()));
			return ps->string();
		}else{ untested();
			return "unreachable";
		}
	}
#endif

	std::string value_name()const override{
		incomplete();
		return "value_name_incomplete";
	}
	std::string port_name(int i)const override{
		return "p"+to_string(i);
	}
	bool print_type_in_spice()const override{ return false;}
//	void set_port_by_index(int a, std::string& b) override { incomplete();}
//	bool node_is_connected(int a) const override { incomplete(); return false; }
	std::string dev_type()const override {return cd()->type;}
	const std::string port_value(int i)const 
	{ untested();
		/// ????
		assert(_n);
		assert(i >= 0);
		assert(i < net_nodes());
		return _n[i].short_label();
	}

	int max_nodes()const override{
		return getSize();
	}
	int net_nodes()const override{
		return getSize();
	}
	int matrix_nodes()const override{
		return getSize();
	}
	int int_nodes()const override{
		return 0; // really?
	}
	int min_nodes()const override{
		return getSize();
	}

private: // should probably use ELEMENT
	double dampdiff(double* v0, const double& v1) { untested();
		//double diff = v0 - v1;
		assert(v0);
		assert(*v0 == *v0);
		assert(v1 == v1);
		double diff = dn_diff(*v0, v1);
		assert(diff == diff);
		if (!_sim->is_advance_or_first_iteration()) { untested();
			diff *= _sim->_damp;
			*v0 = v1 + diff;
		}else{ untested();
		}
		return mfactor() * ((_sim->is_inc_mode()) ? diff : *v0);
	}
	void tr_load_point(const node_t& no1, const node_t& no2,
	                   double* new_value, double* old_value){ untested();
		double d = dampdiff(new_value, *old_value);
		if (d != 0.) { untested();
			_sim->_aa.load_point(no1.m_(), no2.m_(), d);
		}else{ untested();
		}
		*old_value = *new_value;
	}
	void ac_load_point(const node_t& no1, const node_t& no2, COMPLEX value){
		_sim->_acx.load_point(no1.m_(), no2.m_(), mfactor() * value);
	}

protected: // qucsator globals
	int type;
//	static double C0; // ??
private:
	DPAIR* _matrix{nullptr};
	const int _num_ports;
	int _num_param{0};
//	std::vector<PARA_BASE*> _p;
//	std::vector<std::string const*> _pnames;
//	std::map<std::string, PARA_BASE*> _pn;
	substrate* _substrate;
}; // circuit

inline void circuit::init()
{
	if(!_num_ports){ untested();
	}else if(!_n){ untested();
		_n = new node_t[_num_ports];
	}else{
	}
	if(!_num_ports){ untested();
	}else if(!_matrix){
		_matrix = new DPAIR[_num_ports*_num_ports];
	}else{ untested();
	}

	if(common()){
		// BUG init once only. not here.
	}else{

		COMMON_PARAMLIST* mc = prechecked_cast<COMMON_PARAMLIST*>(Default_PARAMS.clone());
		assert(mc);
		unsigned i=0;
		_num_param = 0;
		for(; cd()->required[i].key; ++i){
			auto p = cd()->required[i];
			auto k = p.key;

			PARAM_INSTANCE pi;
			if(p.type == PROP_REAL){
				++_num_param;
				pi = PARAMETER<double>();
			}else if(p.type == PROP_STR){
				++_num_param;
				pi = PARAMETER<String>();
			}else{ untested();
				incomplete();
			}
			mc->_params.set(k, pi);
		}
		unsigned j=0;
		for(;;++j){
			if(char const* k=cd()->optional[j].key){
				++_num_param;
				mc->_params.set(k, PARAMETER<double>());
			}else{
				break;
			}
		}
		attach_common(mc);
		trace2("circuit::init params", i, j);
	}
}
} // namespace qucs
#endif
