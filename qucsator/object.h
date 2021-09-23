#include "netdefs.h"
#include <e_model.h>

namespace qucs{

// BUG: only substrate??
struct object : public MODEL_CARD{
	object() : MODEL_CARD(nullptr) {}
	object(std::string const& x) : MODEL_CARD(nullptr) {}
	std::string getName() const{return "noname";}
	double getPropertyDouble(std::string const& s){ untested();
		trace1("getPropertyDouble", s);
		auto i = _pn.find(s);
		if(i == _pn.end()){ untested();
			assert(false);
			return 0.;
		}else if(auto ps = dynamic_cast<PARAMETER<double> const*>(i->second)){ untested();
			assert(*ps == *ps);
			assert(ps->has_good_value());
			return *ps;
		}else{
			assert(false);
			return 0;
		}
	}
	const char* getPropertyString(std::string const&){ untested();
		incomplete();
		return nullptr;
	}
	virtual define_t* cd() const{return nullptr;}
   int param_count()const override{
		return MODEL_CARD::param_count() + _p.size();
	}
	void set_param_by_name(std::string a, std::string b) override{
		trace2("object::spbn", a, b);
		auto i = _pn.find(a);
		if(i == _pn.end()){
			incomplete();
		}else{
			*i->second = b;
		}
	}
	std::string param_name(int i)const{
		int s = object::param_count() - 1 - i;
		if(s < int(_p.size())){
			assert(s<int(_pnames.size()));
			return *_pnames[s];
		}else{ untested();
			return MODEL_CARD::param_name(i);
		}
	}
	std::string param_value(int i)const override{
		int s = object::param_count() - 1 - i;
		if(s >= int(_p.size())){
			return MODEL_CARD::param_name(i);
		}else if(auto ps = dynamic_cast<PARAMETER<double> const*>(_p[s])){ untested();
			assert(s<int(_p.size()));
			return ps->string();
		}else{ untested();
			return "unreachable";
		}
	}
   bool param_is_printable(int i)const override{
		int s = object::param_count() - 1 - i;
		if(s < int(_p.size())){
			return true;
		}else{
			return MODEL_CARD::param_is_printable(i);
		}
	}
	void init(){
		trace1("init", cd());
		trace1("init", cd()->type);
		incomplete();
		unsigned i=0;
		for(;;++i){ untested();
			if(char const* k=cd()->required[i].key){
				_p.push_back(new PARAMETER<double>);
				auto p = _pn.insert(std::make_pair(k, _p.back()));
				_pnames.push_back(&p.first->first);
			}else{
				break;
			}
		}
	};
	std::string dev_type()const override {return cd()->type;}
	MODEL_CARD* clone() const{return nullptr;}

private: // MODEL_CARD
	void precalc_first() override{
		CARD_LIST* Scope=scope();
		MODEL_CARD::precalc_first();

		for(unsigned s=0; s<_p.size(); ++s){
			if(auto ps = dynamic_cast<PARAMETER<double>*>(_p[s])){ untested();
				ps->e_val(NOT_VALID, Scope);
				trace3("param", s, ps->string(), *ps);
			}else{
			}
		}
	}

private:
	std::vector<PARA_BASE*> _p;
	std::vector<std::string const*> _pnames;
	std::map<std::string, PARA_BASE*> _pn;
};
}

#define MCREATOR(a) \
public: \
	static define_t miscdef; \
	define_t* cd() const override {untested(); return &miscdef;} \
	a(); \
	MODEL_CARD* clone() const override{ untested(); a* n=new a(*this); n->init(); return n; }

