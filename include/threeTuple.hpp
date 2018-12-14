#ifndef THREE_TUPLE_HPP
#define THREE_TUPLE_HPP

#include <string>

class threeTuple{
  public:
	double p[3];
	
	std::string name;
  
	threeTuple() : name() { zero(); }
	
	threeTuple(const double &x_, const double &y_, const double &z_) : name() { p[0] = x_; p[1] = y_; p[2] = z_; }
	
	threeTuple(const std::string &str_); 
	
	bool operator == (const threeTuple &other) const { return (p[0]==other.p[0] && p[1]==other.p[1] && p[2]==other.p[2]); }
	
	void operator *= (const double &val){ p[0] *= val; p[1] *= val; p[2] *= val; }

	threeTuple operator + (const threeTuple &other) const ;

	threeTuple operator - (const threeTuple &other) const ;
	
	std::string print() const ;
	
	double length() const ;	
	
	void zero(){ p[0] = 0; p[1] = 0; p[2] = 0; }
};

#endif
