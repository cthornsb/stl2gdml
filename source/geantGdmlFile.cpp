#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cmath>

#include "geantGdmlFile.hpp"
#include "polySolid.hpp"

///////////////////////////////////////////////////////////////////////////////
// splitFilename
///////////////////////////////////////////////////////////////////////////////

void splitFilename(const std::string &filename, std::string &name, std::string &prefix, std::string &extension){
	size_t index = filename.find_last_of('.');
	if(index != std::string::npos){
		prefix = filename.substr(0, index);
		extension = filename.substr(index+1);
	}
		
	index = prefix.find_last_of('/');
	if(index != std::string::npos){
		name = prefix.substr(index+1);
	}
}

///////////////////////////////////////////////////////////////////////////////
// readAST
///////////////////////////////////////////////////////////////////////////////

unsigned int readAST(const char *fname, gdmlEntry &entry, const double &unit=mm){
	std::ifstream file(fname);
	if(!file.good())
		return 0;
	
	std::vector<std::string> block;
	
	unsigned int linesRead = 0;
	
	std::string line;
	while(true){
		std::getline(file, line);
		if(file.eof()) break;
	
		linesRead++;
	
		// Read a new facet (triangle).
		if(line.find("endfacet") != std::string::npos){ // Finalize the facet.
			block.push_back(line);
			facet triangle(block);
			if(triangle.good){
				triangle *= unit;
				entry.solid.add(triangle);
			}
			block.clear();
		}
		else{
			block.push_back(line);
		}
	}

	file.close();
	
	return linesRead;
}

///////////////////////////////////////////////////////////////////////////////
// readSTL
///////////////////////////////////////////////////////////////////////////////

unsigned int readSTL(const char *fname, gdmlEntry &entry, const double &unit=mm){
	unsigned char header[80];
	unsigned int nTriangles;

	std::ifstream file(fname, std::ios::binary);
	if(!file.good()){
		return 0;
	}
	
	file.read((char*)header, 80);
	file.read((char*)&nTriangles, 4);

	float vect[12];
	unsigned short att;

	bool invalidRead = false;
	for(unsigned int i = 0; i < nTriangles; i++){
		file.read((char*)vect, 48);
		file.read((char*)&att, 2);
		if(file.eof()){
			invalidRead = true;
			break;
		}
		stlBlock block(vect, att);
		facet triangle(block);
		triangle *= unit;
		entry.solid.add(triangle);
	}
	
	file.close();	
	
	if(invalidRead){
		std::cout << " Warning: Failed to read all " << nTriangles << " triangles specified in header!\n";
	}
	
	return entry.solid.size();
}

///////////////////////////////////////////////////////////////////////////////
// class geantGdmlFile
///////////////////////////////////////////////////////////////////////////////

bool geantGdmlFile::process(const std::string &outputFilename, const std::vector<std::string> &filenames){
	for(size_t i = 0; i < 3; i++){
		rmin[i] = 1E10;
		rmax[i] = -1E10;
	}

	goodFiles.clear();

	// Read in 3d geometry.
	std::string gdmlFilename;
	for(std::vector<std::string>::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++){
		processFile((*iter));
	}

	// Compute the physical size of the solid.
	size_t nPoly = 0;
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){
		double solidmin[3], solidmax[3];
		iter->solid.getSizeX(solidmin[0], solidmax[0]);
		iter->solid.getSizeY(solidmin[1], solidmax[1]);
		iter->solid.getSizeZ(solidmin[2], solidmax[2]);
	
		// Get the physical size of the solid.
		threeTuple physicalSize = threeTuple(solidmax[0]-solidmin[0], solidmax[1]-solidmin[1], solidmax[2]-solidmin[2]);
	
		for(size_t i = 0; i < 3; i++){
			rmin[i]	= std::min(rmin[i], solidmin[i]); 
			rmax[i] = std::max(rmax[i], solidmax[i]);
		}
		
		iter->physSize = physicalSize;
		nPoly += iter->solid.size();
	}	

	// Compute the size of the world.
	for(size_t i = 0; i < 3; i++){
		worldSize[i] = rmax[i]-rmin[i];
	}

	std::cout << " Successfully loaded " << goodFiles.size() << " geometries and a total of " << nPoly << " polygons.\n";

	if(debug)
		std::cout << "debug: worldX=" << worldSize[0] << " mm, worldY=" << worldSize[1] << " mm, worldZ=" << worldSize[2] << " mm\n";
	
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){
		iter->computeOffset(worldSize[0], worldSize[1], worldSize[2]);
		if(debug)
			std::cout << "debug: offsetX=" << iter->offset.p[0] << " mm, offsetY=" << iter->offset.p[1] << " mm, offsetZ=" << iter->offset.p[2] << " mm\n";
		
		// Offset all polygons so that the origin is at the center. 
		iter->solid.addOffset(iter->offset);

		// Identify unique vertices.
		iter->solid.getUniqueVertices(uniqueVert, solidCount++);
		
		if(debug)
			std::cout << "debug: uniqueVert.size()=" << uniqueVert.size() << std::endl;

		// Match all facet vertices with one of the unique vertices.
		for(std::vector<facet>::iterator poly = iter->solid.begin(); poly != iter->solid.end(); poly++){	
			for(size_t i = 0; i < 3; i++){
				for(size_t j = 0; j < uniqueVert.size(); j++){
					if(poly->vertices[i] == uniqueVert[j]){
						poly->names[i] = uniqueVert[j].name;
						break;
					}
				}
			}
		}
		
		// Identify unique polygons.
		iter->solid.getUniquePolygons(uniquePoly, iter->offset);

		if(debug){ // Output slice information.
			std::vector<ySlice> *slices = iter->solid.getSlices();
			std::cout << "debug: slices->size()=" << slices->size() << std::endl;
			for(size_t i = 0; i < slices->size(); i++){
				if(!slices->at(i).empty()){
					std::cout << "debug:  y=" << slices->at(i).getY() << ", x=" << slices->at(i).getSizeX() << ", z=" << slices->at(i).getSizeZ() << "\n";
				}
			}
		}
	}	

	std::cout << "  Identified " << uniqueVert.size() << " unique vertices and " << uniquePoly.size() << " unique polygons.\n";

	// Enforce symmetry requirements.
	double tempMin[3] = {1E10, 1E10, 1E10};
	double tempMax[3] = {-1E10, -1E10, -1E10};	
	for(std::vector<threeTuple>::iterator iter = uniqueVert.begin(); iter != uniqueVert.end(); iter++){
		for(size_t j = 0; j < 3; j++){ // Over all three axes.
			tempMin[j] = std::min(tempMin[j], iter->p[j]); 
			tempMax[j] = std::max(tempMax[j], iter->p[j]);
		}
	}

	for(size_t i = 0; i < 3; i++){
		if(debug)
			std::cout << "debug: i=" << i << ", min=" << tempMin[i] << ", max=" << tempMax[i] << ", offset=" << std::fabs(tempMax[i]+tempMin[i])*um << " microns\n";
		if(std::fabs(tempMax[i]+tempMin[i]) >= 1E-3){ // Check for offset of more than 1 um
			std::cout << " Warning! Axis " << i << " offset mismatch of (" << std::fabs(tempMax[i]+tempMin[i]) << " mm). Correcting...\n";
		}
	}

	// Generate the union solid.
	std::string masterSolidName;
	std::string masterObjectFilename;
	std::string extension;
	
	splitFilename(outputFilename, masterSolidName, masterObjectFilename, extension);
	masterObjectFilename += "_geom.gdml";

	// Copy the unique polys into the union.	
	gdmlEntry entry(masterObjectFilename, masterSolidName, threeTuple(worldSize[0], worldSize[1], worldSize[2]));
	for(std::vector<facet>::iterator poly = uniquePoly.begin(); poly != uniquePoly.end(); poly++){
		entry.solid.add((*poly));
	}
	entry.computeOffset(worldSize[0], worldSize[1], worldSize[2]);
	//goodFiles.push_back(entry); // This geometry does not work currently. Do not write it to the output.

	// Write the files.
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){
		writeGeometry((*iter));
	}
	
	// Write the parent file.
	generateMasterFile(outputFilename);
	
	return true;
}
	
bool geantGdmlFile::processFile(const std::string &filename){
	std::string solidName = "Thingy";
	std::string gdmlFilename = filename;
	std::string extension = "";
	
	// Get the solid name and file extension from the filename.
	splitFilename(filename, solidName, gdmlFilename, extension);
	
	// Get the output gdml filename.
	gdmlFilename += ".gdml";
		
	std::cout << " Processing file \"" << filename << "\", solid=" << solidName << "\n";

	gdmlEntry entry(gdmlFilename, solidName, threeTuple());
	
	if(extension == "stl"){ // Binary STL file
		readSTL(filename.c_str(), entry, drawingUnit);
		std::cout << "  Read " << entry.solid.size() << " polygons\n";
	}
	else{ // Ascii STL file
		if(extension != "ast")
			std::cout << " Warning: Unknown file type (" << extension << "), assuming AST format.\n";
		unsigned int lines = readAST(filename.c_str(), entry, drawingUnit);
		std::cout << "  Read " << lines << " lines and " << entry.solid.size() << " polygons\n";
	}
	
	goodFiles.push_back(entry);
	
	return true;
}

bool geantGdmlFile::writeGeometry(const gdmlEntry &entry){
	std::ofstream ofile(entry.filename.c_str());
	if(!ofile.good())
		return false;
	
	ofile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
	ofile << "<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd\">\n\n";

	ofile << "    <define>\n";
	for(size_t i = 0; i < uniqueVert.size(); i++){ // Write vertex position definitions.
		for(std::vector<facet>::const_iterator iter = entry.solid.cbegin(); iter != entry.solid.cend(); iter++){
			if(iter->usesVertex(uniqueVert[i].name)){ // Set precision to 1E-3 (1 um)
				ofile << "             " << uniqueVert[i].print(4) << std::endl;
				break;
			}
		}
	}
	ofile << "    </define>\n\n";

	ofile << "    <solids>\n";
	ofile << "        <tessellated aunit=\"deg\" lunit=\"mm\" name=\"" << entry.solidName << "\">\n";

	// Print the triangular definitions to the output file.
	for(std::vector<facet>::const_iterator iter = entry.solid.cbegin(); iter != entry.solid.cend(); iter++){
		if(!iter->checkNames()){
			std::cout << " ERROR\n";
		}
		else{
			ofile << "             " << iter->print() << std::endl;
		}
	}
	
	ofile << "        </tessellated>\n";
	ofile << "    </solids>\n\n";
	
	ofile << "    <structure>\n";
	ofile << "        <volume name=\"" << entry.solidName << ".gdml\">\n";
	ofile << "            <materialref ref=\"" << materialName << "\"/>\n";
	ofile << "            <solidref ref=\"" << entry.solidName << "\"/>\n";
	ofile << "            <positionref ref=\"" << entry.solidName << ".gdml_pos\"/>\n";
	ofile << "            <rotationref ref=\"" << entry.solidName << ".gdml_rot\"/>\n";
	ofile << "        </volume>\n";
	ofile << "    </structure>\n\n";

	ofile << "    <setup name=\"Default\" version=\"1.0\">\n";
	ofile << "        <world ref=\"" << entry.solidName << ".gdml\"/>\n";
	ofile << "    </setup>\n";
	ofile << "</gdml>\n";
	
	std::cout << "  Generated output file \"" << entry.filename << "\"\n";
	
	ofile.close();
	
	return true;
}

bool geantGdmlFile::generateMasterFile(const std::string &outputFilename){ // Generate the master file.
	std::ofstream masterFile(outputFilename.c_str());
	if(!masterFile.good())
		return false;
	
	std::string objName;
	size_t index1, index2;
	index1 = outputFilename.find_last_of('/');
	index2 = outputFilename.find_last_of('.');
	
	// Get the object name from the output path.
	if(index1 != std::string::npos){
		if(index2 != std::string::npos)
			objName = outputFilename.substr(index1+1, index2-(index1+1));
		else
			objName = outputFilename.substr(index1+1);
	}
	else{
		if(index2 != std::string::npos)
			objName = outputFilename.substr(0, index2);
		else
			objName = outputFilename;
	}
		
	masterFile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
	masterFile << "<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd\">\n\n";
	
	masterFile << "    <define>\n";
	/*for(size_t i = 0; i < uniqueVert.size(); i++){ // Write vertex position definitions.
		for(std::vector<facet>::iterator iter = uniquePoly.begin(); iter != uniquePoly.end(); iter++){
			if(iter->usesVertex(uniqueVert[i].name)){
				masterFile << "        " << uniqueVert[i].print() << std::endl;
				break;
			}
		}
	}*/
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){ // Daughter positions (currently un-used).
		// <position name="offsetpos" unit="mm" x="19" y="0" z="19"/>
		masterFile << "        <position name=\"" << iter->solidName << ".gdml_pos\" unit=\"mm\" x=\"" << iter->offset.p[0] << "\" y=\"" << iter->offset.p[1] << "\" z=\"" << iter->offset.p[2] << "\"/>\n";
	}
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){ // Daughter rotations (currently un-used).
		// <rotation name="offsetpos" unit="deg" x="0" y="0" z="0"/>
		masterFile << "        <rotation name=\"" << iter->solidName << ".gdml_rot\" unit=\"deg\" x=\"0\" y=\"0\" z=\"0\"/>\n";
	}
	masterFile << "    </define>\n\n";
	
	masterFile << "    <solids>\n";
	masterFile << "        <box lunit=\"mm\" name=\"" << objName << "_solid\" x=\"" << worldSize[0] << "\" y=\"" << worldSize[1] << "\" z=\"" << worldSize[2] << "\"/>\n";
	/*masterFile << "        <tessellated aunit=\"deg\" lunit=\"mm\" name=\"" << objName << "_solid\">\n";
	for(std::vector<facet>::iterator iter = uniquePoly.begin(); iter != uniquePoly.end(); iter++){ // Print the triangular definitions to the output file.
		if(!iter->checkNames()){
			std::cout << " ERROR\n";
		}
		else{
			masterFile << "             " << iter->print() << std::endl;
		}
	}
	masterFile << "        </tessellated>\n";*/
	masterFile << "    </solids>\n\n";

	masterFile << "    <structure>\n";
	masterFile << "        <volume name=\"" << objName << "\">\n";
	masterFile << "            <materialref ref=\"G4_AIR\"/>\n";
	masterFile << "            <solidref ref=\"" << objName << "_solid\"/>\n\n";
	
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){
		// <file name="/path/to/file/file.gdml"/>
		// <positionref ref="position"/>
		masterFile << "            <physvol>\n";
		masterFile << "                <file name=\"" << iter->filename << "\"/>\n";
		masterFile << "            </physvol>\n\n";
	}
	
	masterFile << "        </volume>\n";
	masterFile << "    </structure>\n\n";

	masterFile << "    <setup name=\"Default\" version=\"1.0\">\n";
	masterFile << "        <world ref=\"" << objName << "\"/>\n";
	masterFile << "    </setup>\n";
	masterFile << "</gdml>\n";
	
	masterFile.close();
	
	return true;
}
