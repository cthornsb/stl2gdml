#ifndef POLY_SOLID_HPP
#define POLY_SOLID_HPP

#include <vector>

#include "ySlice.hpp"
#include "facet.hpp"

template <typename T>
bool isInVector(const T &val, const std::vector<T> &vec){
	for(typename std::vector<T>::const_iterator iter = vec.begin(); iter != vec.end(); iter++){	
		if(val == (*iter)) return true;
	}
	return false;
}

class polySolid{
  public:
	polySolid(){ initialize(); }

	size_t size() const { return solid.size(); }

	void getSizeX(double &min, double &max) const ;

	void getSizeY(double &min, double &max) const ;

	void getSizeZ(double &min, double &max) const ;

	void getUniqueVertices(std::vector<threeTuple> &unique, const int &id) const ;
	
	void getUniquePolygons(std::vector<facet> &unique, const threeTuple &offset) const ;
	
	std::vector<ySlice> *getSlices(){ return &slices; }
	
	std::vector<facet>::const_iterator cbegin() const { return solid.cbegin(); }
	
	std::vector<facet>::const_iterator cend() const { return solid.cend(); }

	std::vector<facet>::iterator begin(){ return solid.begin(); }
	
	std::vector<facet>::iterator end(){ return solid.end(); }
	
	void add(const facet &poly);
	
	void addOffset(const threeTuple &offset);
	
	void clear();
	
  private:
	std::vector<facet> solid;
	std::vector<ySlice> slices; // Bounding box for each y-slice of the model.
	
	double rmin[3];
	double rmax[3];
	
	void initialize();
	
	bool isInSlices(const double &y, std::vector<ySlice>::iterator &iter);
};

#endif
