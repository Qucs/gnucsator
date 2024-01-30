#ifndef U_PARAMLIST_Q
#define U_PARAMLIST_Q
#include "u_parameter_uf.h"

class PARAM_INSTANCE {
private:
  PARA_BASE* _p;
public:
  explicit PARAM_INSTANCE() : _p(NULL){
  }
  /*explicit*/ PARAM_INSTANCE(PARAM_INSTANCE const& p) : _p(NULL){
    // BUG: need clone in PARA_BASE?
    if(auto x=dynamic_cast<PARAMETER<mystring> const*>(p._p)){
      trace1("clone mystring", x->string());
      _p = new PARAMETER<mystring>(*x);
    }else if(auto dd=dynamic_cast<PARAMETER<double> const*>(p._p)){
      trace2("clone dbl", dd->string(), *dd);
      _p = new PARAMETER<double>(*dd);
    }else{
      incomplete();
    }
  }

public:
  bool operator==(PARAM_INSTANCE const&p) const{
    // BUG: need == in PARA_BASE?
    if(&p==this){ untested();
      return true;
    }else if(auto x=dynamic_cast<PARAMETER<mystring> const*>(_p)){
      if(auto y=dynamic_cast<PARAMETER<mystring> const*>(p._p)){
	return *x==*y;
      }else{
	return false;
      }
    }else if(auto xx=dynamic_cast<PARAMETER<double> const*>(_p)){
      if(auto y=dynamic_cast<PARAMETER<double> const*>(p._p)){
	return *xx==*y;
      }else{
	return false;
      }
    }
    incomplete();
    return false;
  }
  PARAM_INSTANCE& operator=(PARAM_INSTANCE const&){
    assert(false);
    incomplete();
    return *this;
  }
  PARAM_INSTANCE& operator=(std::string const& v){
    if(auto x=dynamic_cast<PARAMETER<mystring>*>(_p)){
      *x = v;
      trace2("assigned", x->string(), v);
    }else if(auto dd=dynamic_cast<PARAMETER<double>*>(_p)){
      *dd = v;
    }else if(_p){ untested();
      incomplete();
      *_p = v;
      assert(false);
    }else{
      incomplete();
      assert(false);
    }
    return *this;
  }
  PARAM_INSTANCE& operator=(PARAMETER<mystring> const& v){
    if(auto x=dynamic_cast<PARAMETER<mystring>*>(_p)){ untested();
      *x = v;
    }else{
      delete _p;
      auto p = new PARAMETER<mystring>(v);
      _p = p;
    }
    return *this;
  }
  PARAM_INSTANCE& operator=(PARAMETER<double> const&v){
    if(auto x=dynamic_cast<PARAMETER<double>*>(_p)){ untested();
      *x = v;
    }else{
      delete _p;
      auto p = new PARAMETER<double>(v);
      _p = p;
    }
    return *this;
  }
  PARAM_INSTANCE& operator=(double const&){ untested();
    incomplete();
    return *this;
  }
public:
  std::string const string() const{
#if 0
    if(_p){
      return _p->string();
    }else{
      return "";
    }
#else
    if(auto x = dynamic_cast<PARAMETER<double> const*>(_p)){
      return x->string();
    } else if(auto xx = dynamic_cast<PARAMETER<mystring> const*>(_p)){
      return xx->string();
    } else { untested();
      return "";
    }
#endif
  }
  bool has_hard_value() const{
    if(_p){
      return _p->has_hard_value();
    }else{
      return false;
    }
  }
  double e_val(const double& def, const CARD_LIST* scope)const{
    if(auto x = dynamic_cast<PARAMETER<double> const*>(_p)){
      return x->e_val(def, scope);
    } else if(dynamic_cast<PARAMETER<mystring> const*>(_p)){
      unreachable();
      return NOT_VALID;
    }else{
      incomplete();
    }
    return NOT_VALID;
  }
  operator const char*() const{
    if(auto ps = dynamic_cast<PARAMETER<mystring> const*>(_p)){ untested();
      trace1("const char*", ps->string());
      std::string s = *ps;
      return strdup(s.c_str()); // BUG: memory leak
    }else if(dynamic_cast<PARAMETER<double> const*>(_p)){ untested();
      unreachable();
      return nullptr;
    }else{ untested();
      incomplete();
      return nullptr;
    }
  }
  operator double() const{
    if(auto ps = dynamic_cast<PARAMETER<double> const*>(_p)){
      return *ps;
    }else{
      incomplete();
      return NOT_VALID;
    }
  }
  operator PARAMETER<double> const&() const{
    incomplete();
    if(_p){
    }else{
    }
    static PARAMETER<double> x;
    return x;
  }
}; // PARAM_INSTANCE
/*--------------------------------------------------------------------------*/
#define PARAM_LIST PARAM_LIST_Q
class INTERFACE PARAM_LIST {
private:
  typedef std::map<std::string, PARAM_INSTANCE> map;
public:
  typedef map::const_iterator const_iterator;
  typedef map::iterator iterator;
private:
  map _pl;
  PARAM_LIST const* _try_again; // if you don't find it, also look here
  mutable const_iterator _previous;
public:
  explicit PARAM_LIST() :_try_again(NULL) {}
  explicit PARAM_LIST(const PARAM_LIST& p) :_pl(p._pl), _try_again(p._try_again) {}
  //explicit PARAM_LIST(PARAM_LIST* ta) :_try_again(ta) {untested();}
  ~PARAM_LIST() {}
  void	parse(CS& cmd);
  void	print(OMSTREAM&, LANGUAGE*)const;
  
  size_t size()const {return _pl.size();}
  //bool is_empty()const {untested();return _pl.empty();}
  bool	 is_printable(int)const;
  std::string name(int)const;
  std::string value(int)const;

  void	eval_copy(PARAM_LIST const&, const CARD_LIST*);
  bool  operator==(const PARAM_LIST& p)const{ return _pl == p._pl;}
  const PARAMETER<double>& deep_lookup(std::string)const;
  const PARAMETER<double>& operator[](std::string i)const {return deep_lookup(i);}
  template<class T>
  void set(std::string, const T&);
  void set_try_again(PARAM_LIST const* t) {_try_again = t;}

  iterator begin() {return _pl.begin();}
  iterator end() {return _pl.end();}
  const_iterator begin()const {itested(); return _pl.begin();}
  const_iterator end()const {itested(); return _pl.end();}
  const_iterator find(std::string const& k) const { return _pl.find(k); }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
inline std::string PARAM_LIST::name(int i)const
{
  //BUG// ugly linear search
  int i_try = 0;
  for (const_iterator ii = _pl.begin(); ii != _pl.end(); ++ii) {
    if (i_try++ == i) {
      return ii->first;
    }else{
    }
  }
  return "";
}
/*--------------------------------------------------------------------------*/
inline std::string PARAM_LIST::value(int i)const
{
  //BUG// ugly linear search
  int i_try = 0;
  for (const_iterator ii = _pl.begin(); ii != _pl.end(); ++ii) {
    if (i_try++ == i) {
      return ii->second.string();
    }else{
    }
  }
  return "";
}
/*--------------------------------------------------------------------------*/
inline bool PARAM_LIST::is_printable(int i)const
{ untested();
  //BUG// ugly linear search
  int i_try = 0;
  for (const_iterator ii = _pl.begin(); ii != _pl.end(); ++ii) {
    if (i_try++ == i) {
      return ii->second.has_hard_value();
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
inline const PARAMETER<double>& PARAM_LIST::deep_lookup(std::string Name)const
{
  if (OPT::case_insensitive) {
    notstd::to_lower(&Name);
  }else{
  }
  const_iterator i = _pl.find(Name);
  if (i!=_pl.end() && i->second.has_hard_value()) {
    // found a value, return it
    return i->second;
  }else if (_try_again) {
    // didn't find one, look in enclosing scope
    return _try_again->deep_lookup(Name);
  }else{
    // no enclosing scope to look in
    // really didn't find it, give up
    // return garbage value (NOT_INPUT)
    static PARAMETER<double> garbage;
    return garbage;
  }
}
/*--------------------------------------------------------------------------*/
template<class T>
inline void PARAM_LIST::set(std::string Name, const T& Value)
{
  if (OPT::case_insensitive) { untested();
    notstd::to_lower(&Name);
  }else{
  }

  //trace2("PL::set", Name, Value);
  _pl[Name] = Value;
}
/*--------------------------------------------------------------------------*/
#undef PARAM_LIST

#endif
// vim:ts=8:sw=2:noet:
