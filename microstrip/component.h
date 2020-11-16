// (C) 2020 Felix Salfelder
//     GPLv3+

#include <gnucap/io_trace.h>
#include <gnucap/e_compon.h>
#include <gnucap/e_node.h>
#include <cmath>
#include <complex>
#include <string.h>

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

namespace qucs{

static const int CIR_MSCOUPLED = 3;
static const int CIR_MSLINE = 2;
static const nr_double_t z0 = 50.0; // yikes.

#if 0
static const double euler = 2.7182818284590452353602874713526625;
static const double pi = M_PI;
static const double one_over_pi = 1./pi;
static const double two_over_pi = 2./pi;
static const double pi_over_2 = pi*.5;
#endif

typedef enum {NODE_1 = 0,
		NODE_2 = 1,
		NODE_3 = 2,
		NODE_4 = 3} node_number;

typedef enum { VSRC_1 = 0, VSRC_2 = 1} vsrc_number;

// some other header?
inline double sin(double y)
{
	return ::sin(y);
}
inline double exp(double y)
{
	return ::exp(y);
}
inline double pow(double x, double y)
{
	return ::pow(x, y);
}
inline double log(double x)
{
	return ::log(x);
}
inline double coth(double x)
{
	return 1./::tanh(x);
}
inline nr_complex_t coth (const nr_complex_t z)
{
    nr_double_t r = 2.0 * std::real (z);
    nr_double_t i = 2.0 * std::imag (z);
    return 1.0 + 2.0 / (std::polar (std::exp (r), i) - 1.0);
}
inline double quadr(double x)
{
	incomplete();
	return 0.;
}
inline double log10(double x)
{
	return std::log10(x);
}
inline double cosh(double x)
{
	return ::cosh(x);
}
inline double atan(double x)
{
	return ::atan(x);
}
inline double sinh(double x)
{
	return ::sinh(x);
}
inline nr_complex_t sinh(nr_complex_t x)
{
	return std::sinh(x);
}
inline nr_complex_t cosech (const nr_complex_t z)
{
    return (1.0 / std::sinh(z));
}
inline nr_complex_t cosh (const nr_complex_t z)
{
    return std::cosh(z);
}
inline double sqrt(double x)
{
	return ::sqrt(x);
}
inline double sqr(double x)
{
	return x*x;
}
inline matrix operator*(double, matrix)
{
	incomplete();
	return matrix();
}
inline matrix::matrix()
{
	incomplete();
}
inline matrix::matrix(matrix const&)
{
	incomplete();
}
inline matrix::~matrix()
{
	incomplete();
}
inline matrix conj(matrix)
{
	incomplete();
	return matrix();
}
inline matrix real(matrix)
{
	incomplete();
	return matrix();
}
inline matrix transpose(matrix)
{
	incomplete();
	return matrix();
}
inline matrix operator*(matrix, matrix)
{
	incomplete();
	return matrix();
}

struct substrate{
	double getPropertyDouble(std::string const&){
		incomplete();
		return 0.;
	}
	const char* getPropertyString(std::string const&){
		incomplete();
		return nullptr;
	}
};

class circuit : public COMPONENT{
protected:
	circuit(circuit const& c) : COMPONENT(c),  _num_ports(c._num_ports){
		assert(_num_ports);
		_n = new node_t[_num_ports];
	};
public:
	circuit(int n) :  COMPONENT(), _num_ports(n) {
		assert(n);
		_n = new node_t[_num_ports];
	}
	~circuit() {
		incomplete();
		// delete _n;
	}
protected: // qucsator globals

	double getPropertyDouble(std::string const&){
		incomplete();
		return 0.;
	}
	const char* getPropertyString(std::string const&){
		incomplete();
		return nullptr;
	}
	void setCharacteristic (std::string const& name, double const& value){
		incomplete();
	}
	substrate /*const*/ * getSubstrate(){
		incomplete();
		return nullptr;
	}
   void setVoltageSources(double) {incomplete();}
   void setInternalVoltageSource(double) {incomplete();}
   void allocMatrixMNA () {incomplete();}
	matrix getMatrixS() {incomplete(); return matrix();}
	matrix getMatrixY() {incomplete(); return matrix();}
	void setMatrixN (matrix) {incomplete();}
	void clearY (){incomplete();}
	int getSize() const {return _num_ports;}

//	load_ac?
	void setS (node_number, node_number, nr_complex_t){ incomplete(); }
	void setD (vsrc_number, vsrc_number, nr_complex_t){ incomplete(); }
// acrhs?
	void setY (node_number, node_number, nr_complex_t){ incomplete(); }
   void voltageSource (vsrc_number, node_number, node_number){ incomplete(); }

private: // COMPONENT
	void set_param_by_name(std::string a, std::string b) override{
		trace2("spbn", a, b);
	}
	std::string value_name()const override{incomplete(); return "incomplete";}
	std::string port_name(int i)const override{ untested();
		return "p"+to_string(i);
	}
	bool print_type_in_spice()const override{ untested(); return false;}
//	void set_port_by_index(int a, std::string& b) override { incomplete();}
//	bool node_is_connected(int a) const override { incomplete(); return false; }
	std::string dev_type()const override {return "some_ms";}
	const std::string port_value(int i)const 
	{
		/// ????
		assert(_n);
		assert(i >= 0);
		assert(i < net_nodes());
		return _n[i].short_label();
	}

	int max_nodes()const override{ untested();
		return getSize();
	}
	int net_nodes()const override{ untested();
		return getSize();
	}
	int min_nodes()const override{ untested();
		return getSize();
	}
protected: // qucsator globals
	int type;
//	static double C0; // ??
private:
	int _num_ports;
};
}

#define CREATOR(a) \
public: \
	static define_t cirdef; \
	a(); \
	a( const a& p) : circuit(p){incomplete();} \
	COMPONENT* clone() const{ return new a(*this); }

