
#include <gnucap/io_trace.h>
#include <cmath>
#include <complex>
#include <string.h>

typedef double nr_double_t;
typedef std::complex<double> nr_complex_t;

#define NR_TINY 1e-12 // yikes.
#include "matrix.h"
#include "real.h"
#include "constants.h"
#include "netdefs.h"


namespace qucs{

static const int CIR_MSCOUPLED=3;
static const double pi = M_PI;
static const double one_over_pi = 1./pi;
static const nr_double_t z0 = 50.0; // yikes.
static const double euler = 2.7182818284590452353602874713526625;

typedef enum {NODE_1 = 0,
		NODE_2 = 1,
		NODE_3 = 2,
		NODE_4 = 3} node_number;

typedef enum { VSRC_1 = 0, VSRC_2 = 1} vsrc_number;

double exp(double y)
{
	return ::exp(y);
}
double pow(double x, double y)
{
	return ::pow(x, y);
}
double log(double x)
{
	return ::log(x);
}
double cosh(double x)
{
	return ::cosh(x);
}
double sinh(double x)
{
	return ::sinh(x);
}
nr_complex_t sinh(nr_complex_t x)
{
	return std::sinh(x);
}
double sqrt(double x)
{
	return ::sqrt(x);
}
double sqr(double x)
{
	return x*x;
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

class circuit{
protected:
	circuit(circuit const& c){ incomplete(); }
public:
	circuit(int){ incomplete(); }
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
   void setVoltageSources(double);
   void setInternalVoltageSource(double);
   void allocMatrixMNA ();
	matrix getMatrixS();
	matrix getMatrixY();
	void setMatrixN (matrix);
	void clearY ();
	int getSize(); // number of ports??

//	load_ac?
	void setS (node_number, node_number, nr_complex_t);
	void setD (vsrc_number, vsrc_number, nr_complex_t);
// acrhs?
	void setY (node_number, node_number, nr_complex_t);
   void voltageSource (vsrc_number, node_number, node_number);

protected: // qucsator globals
	int type;
	double C0; // ??
};
}

#define CREATOR(a) public: static define_t cirdef; a(); a( const a& p) : circuit(p){incomplete();}

