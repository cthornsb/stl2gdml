#ifndef STL_BLOCK_HPP
#define STL_BLOCK_HPP

#include "threeTuple.hpp"

class stlBlock{
  public:
	threeTuple normal;
	threeTuple vertices[3];

	unsigned short attribute;

	stlBlock(){ }

	stlBlock(float *array, const unsigned short &att=0);
	
	void operator *= (const double &val);
	
	bool compare(const stlBlock &other) const ;
	
	bool usesVertex(const threeTuple &vertex) const ;
};

#endif
