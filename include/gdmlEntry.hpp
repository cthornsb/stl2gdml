#ifndef GDML_ENTRY_HPP
#define GDML_ENTRY_HPP

#include <string>

#include "threeTuple.hpp"
#include "polySolid.hpp"

class gdmlEntry{
  public:
	std::string filename;
	std::string solidName;
	threeTuple physSize;
	threeTuple offset;

	polySolid solid; // 3d solids.
	
	gdmlEntry(){ }
	
	gdmlEntry(const std::string &fname_, const std::string &sname_, const threeTuple &size_) : filename(fname_), solidName(sname_), physSize(size_) { }
	
	void computeOffset(const double &sizeX_, const double &sizeY_, const double &sizeZ_);
};

#endif
