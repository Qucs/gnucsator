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
