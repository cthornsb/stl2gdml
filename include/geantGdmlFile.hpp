#ifndef GEANT_GDML_FILE_HPP
#define GEANT_GDML_FILE_HPP

#include <string>
#include <vector>

#include "gdmlEntry.hpp"
#include "threeTuple.hpp"

const double in = 25.4;
const double ft = 304.8;
const double m = 1000;
const double dm = 100;
const double cm = 10;
const double mm = 1;

class polySolid;

class geantGdmlFile{
  public:
	geantGdmlFile() : solidCount(0), drawingUnit(mm), debug(false), materialName("G4_AIR") { }

	bool toggleDebug(){ return (debug = !debug); }

	void setDrawingUnit(const double &unit){ drawingUnit = unit; }

	void setMaterialName(const std::string &mat){ materialName = mat; }

	bool process(const std::string &outputFilename, const std::vector<std::string> &filenames);

  private:
	int solidCount;

	double drawingUnit;
	
	bool debug;
	
	std::string materialName;

	double rmin[3];
	double rmax[3];

	std::vector<gdmlEntry> goodFiles; // List of good gdml files.
	
	std::vector<threeTuple> uniqueVert; // List of unique vertices.
	
	std::vector<facet> uniquePoly; // List of unique polygons.
	
	polySolid *solid; // Pointer to the current 3d solid.

	bool processFile(const std::string &filename);

	bool writeGeometry(const gdmlEntry &entry);

	unsigned int generateUniqueVertices(const std::string &name, threeTuple &size);

	bool generateMasterFile(const std::string &outputFilename);
};

#endif
