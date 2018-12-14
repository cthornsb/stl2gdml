#include "gdmlEntry.hpp"

void gdmlEntry::computeOffset(const double &sizeX_, const double &sizeY_, const double &sizeZ_){
	double xoffset = -sizeX_/2 + (sizeX_ - physSize.p[0])/2;
	double yoffset = -sizeY_/2 + (sizeY_ - physSize.p[1])/2;
	double zoffset = -sizeZ_/2 + (sizeZ_ - physSize.p[2])/2;
	offset = threeTuple(xoffset, yoffset, zoffset);
}
