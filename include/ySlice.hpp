#ifndef Y_SLICE_HPP
#define Y_SLICE_HPP

#include "threeTuple.hpp"

class ySlice{
  public:
	ySlice() : xmin(1E10), xmax(-1E10), zmin(1E10), zmax(-1E10), y(0), count(0), thickness(0.1) { }
	
	ySlice(const double &y_) : xmin(1E10), xmax(-1E10), zmin(1E10), zmax(-1E10), y(y_), count(0), thickness(0.1) { }
	
	bool isInSlice(const double &y_) const { return (y_ > y-thickness/2 && y_ <= y+thickness/2); }
	
	bool empty() const { return (count == 0); }
	
	double getY() const { return y; }
	
	double getSizeX() const { return (xmax-xmin); }
	
	double getSizeZ() const { return (zmax-zmin); }
	
	void addPoint(const double &x, const double &z);
	
	void addPoint(const threeTuple &vec);

  private:	
	double xmin, xmax;
	double zmin, zmax;
	double y;
	
	size_t count;
	
	double thickness; // Thickness of slice.	
};

#endif
