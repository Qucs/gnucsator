
#include <gnucap/globals.h>
#include <gnucap/c_comand.h>
#include <gnucap/io_trace.h>
#include <gnucap/u_lang.h>

#include "component.h"
#include "substrate.h"
#include "msline.h"

namespace{
qucs::substrate c0;
DISPATCHER<MODEL_CARD>::INSTALL d0(&model_dispatcher, "SUBST", &c0);

class SUBST : public CMD {
	void do_it(CS& cmd, CARD_LIST* scope) override{ untested();
		assert(scope);
		auto x = model_dispatcher.clone("SUBST");
		assert(x);
		assert(OPT::language);
		OPT::language->parse_paramset(cmd, x);
		// parse_model...
		incomplete();
		scope->push_back(x);
	}
}c1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "SUBST", &c1);
}
