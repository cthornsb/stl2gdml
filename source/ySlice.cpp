#include "ySlice.hpp"

void ySlice::addPoint(const double &x, const double &z){
	xmin = std::min(xmin, x); 
	xmax = std::max(xmax, x);
	zmin = std::min(zmin, z); 
	zmax = std::max(zmax, z);
	count++;
}

void ySlice::addPoint(const threeTuple &vec){
	this->addPoint(vec.p[0], vec.p[2]);
}
