
#include <globals.h>
#include <c_comand.h>
#include <io_trace.h>
#include <u_lang.h>

#include "component.h"
#include "microstrip/substrate.h"
#include "microstrip/msline.h"
#include "q_paramlist.cc"

namespace{
qucs::substrate c0;
DISPATCHER<MODEL_CARD>::INSTALL d0(&model_dispatcher, "SUBST", &c0);

class SUBST : public CMD {
	void do_it(CS& cmd, CARD_LIST* scope) override{
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

//std::string	Q_PARAMLIST::name()const	{untested();return "";}
