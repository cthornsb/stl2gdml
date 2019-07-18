#ifndef GDML_ENTRY_HPP
#define GDML_ENTRY_HPP

#include <string>

#include "threeTuple.hpp"
#include "polySolid.hpp"

class gdmlEntry{
  public:
	std::string filename;
	std::string filepath;
	std::string solidName;
	threeTuple physSize;
	threeTuple offset;

	polySolid solid; // 3d solids.
	
	gdmlEntry(){ }
	
	gdmlEntry(const std::string &fname_, const std::string &fpath_, const std::string &sname_, const threeTuple &size_) : filename(fname_), filepath(fpath_), solidName(sname_), physSize(size_) { }
	
	std::string getFullFilename() const { return (filepath + "/" + filename); }
	
	void computeOffset(const double &sizeX_, const double &sizeY_, const double &sizeZ_, bool debug=false);
};

#endif
