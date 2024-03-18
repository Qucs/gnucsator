
inline OMSTREAM& operator<<(OMSTREAM& o, COMPLEX c)
{ untested();
	o << c.real();
	if(c.imag() <0){ untested();
		o << "-" << -c.imag();
	}else{ untested();
		o << "+" << c.imag();
	}
	return  o	<< "* i";
}

inline OMSTREAM& operator<<(OMSTREAM& o, long unsigned c)
{
	o << (int)c;
	return o;
}
