#ifndef POLY_SOLID_HPP
#define POLY_SOLID_HPP

#include <vector>

#include "ySlice.hpp"
#include "facet.hpp"

class polySolid{
  public:
	polySolid(){ initialize(); }

	size_t size() const { return solid.size(); }

	void getSizeX(double &min, double &max) const ;

	void getSizeY(double &min, double &max) const ;

	void getSizeZ(double &min, double &max) const ;

	void getUniqueVertices(std::vector<threeTuple> &unique, const int &id);
	
	std::vector<ySlice> *getSlices(){ return &slices; }
	
	std::vector<facet>::iterator begin(){ return solid.begin(); }
	
	std::vector<facet>::iterator end(){ return solid.end(); }
	
	void add(const facet &poly);
	
	void clear();
	
	void compare(polySolid &other);
  
  private:
	std::vector<facet> solid;
	std::vector<ySlice> slices; // Bounding box for each y-slice of the model.
	
	double rmin[3];
	double rmax[3];
	
	void initialize();
	
	bool isInVector(const threeTuple &tuple, const std::vector<threeTuple> &vec);
	
	bool isInSlices(const double &y, std::vector<ySlice>::iterator &iter);
};

#endif
