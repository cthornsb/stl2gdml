#include <iostream>
#include <fstream>

#include "geantGdmlFile.hpp"
#include "polySolid.hpp"

///////////////////////////////////////////////////////////////////////////////
// readAST
///////////////////////////////////////////////////////////////////////////////

unsigned int readAST(const char *fname, gdmlEntry &entry, polySolid* &current, const double &unit=mm){
	std::ifstream file(fname);
	if(!file.good())
		return 0;
	
	std::vector<std::string> block;
	
	// Update the current solid pointer.
	current = &entry.solid;
	
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
				current->add(triangle);
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

unsigned int readSTL(const char *fname, gdmlEntry &entry, polySolid* &current, const double &unit=mm){
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

	// Update the current solid pointer.
	current = &entry.solid;
	
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
		current->add(triangle);
	}
	
	file.close();	
	
	if(invalidRead){
		std::cout << " Warning: Failed to read all " << nTriangles << " triangles specified in header!\n";
	}
	
	return current->size();
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
	
	// Identify shared polygons.
	if(debug) std::cout << "debug: goodFiles.size()=" << goodFiles.size() << std::endl;
	for(std::vector<gdmlEntry>::iterator iter1 = goodFiles.begin(); iter1 != goodFiles.end(); iter1++){
		for(std::vector<gdmlEntry>::iterator iter2 = iter1; iter2 != goodFiles.end(); iter2++){
			if(iter1 == iter2) continue; // Do not compare with itself.
			iter1->solid.compare(iter2->solid);
		}
	}	

	// Write the files.
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){
		solid = &iter->solid;
		writeGeometry((*iter));
	}
		
	generateMasterFile(outputFilename);
	
	return true;
}
		
bool geantGdmlFile::processFile(const std::string &filename){
	std::string solidName = "Thingy";
	std::string gdmlFilename = filename;
	std::string extension = "";
	size_t index = filename.find_last_of('.');
	if(index != std::string::npos){
		solidName = filename.substr(0, index);
		gdmlFilename = filename.substr(0, index) + ".gdml";
		extension = filename.substr(index+1);
	}
		
	index = solidName.find_last_of('/');
	if(index != std::string::npos){
		solidName = solidName.substr(index+1);
	}
		
	std::cout << " Processing file \"" << filename << "\", solid=" << solidName << "\n";

	gdmlEntry entry(gdmlFilename, solidName, threeTuple());
	
	if(extension == "stl"){ // Binary STL file
		readSTL(filename.c_str(), entry, solid, drawingUnit);
		std::cout << "  Read " << solid->size() << " polygons\n";
	}
	else{ // Ascii STL file
		if(extension != "ast")
			std::cout << " Warning: Unknown file type (" << extension << "), assuming AST format.\n";
		unsigned int lines = readAST(filename.c_str(), entry, solid, drawingUnit);
		std::cout << "  Read " << lines << " lines and " << solid->size() << " polygons\n";
	}
	
	threeTuple physicalSize;
	std::cout << "  Identified " << generateUniqueVertices(solidName, physicalSize) << " unique vertices\n";
	
	if(debug){
		std::vector<ySlice> *slices = solid->getSlices();
		std::cout << "debug: slices->size()=" << slices->size() << std::endl;
		for(size_t i = 0; i < slices->size(); i++){
			if(!slices->at(i).empty()){
				std::cout << "debug:  y=" << slices->at(i).getY() << ", x=" << slices->at(i).getSizeX() << ", z=" << slices->at(i).getSizeZ() << "\n";
			}
		}
	}
	
	entry.physSize = physicalSize;
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
		for(std::vector<facet>::iterator iter = solid->begin(); iter != solid->end(); iter++){
			if(iter->usesVertex(uniqueVert[i].name)){
				ofile << "             " << uniqueVert[i].print() << std::endl;
				break;
			}
		}
	}
	ofile << "    </define>\n\n";

	ofile << "    <solids>\n";
	ofile << "        <tessellated aunit=\"deg\" lunit=\"mm\" name=\"" << entry.solidName << "\">\n";

	// Print the triangular definitions to the output file.
	for(std::vector<facet>::iterator iter = solid->begin(); iter != solid->end(); iter++){
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

unsigned int geantGdmlFile::generateUniqueVertices(const std::string &name, threeTuple &size){
	solid->getUniqueVertices(uniqueVert, solidCount++);
	
	// Get the physical size of the solid->
	double solidmin[3], solidmax[3];
	solid->getSizeX(solidmin[0], solidmax[0]);
	solid->getSizeY(solidmin[1], solidmax[1]);
	solid->getSizeZ(solidmin[2], solidmax[2]);
	
	size = threeTuple(solidmax[0]-solidmin[0], solidmax[1]-solidmin[1], solidmax[2]-solidmin[2]);
	
	for(size_t i = 0; i < 3; i++){
		rmin[i]	= std::min(rmin[i], solidmin[i]); 
		rmax[i] = std::max(rmax[i], solidmax[i]);
	}

	// Match all facet vertices with one of the unique vertices.
	for(std::vector<facet>::iterator iter = solid->begin(); iter != solid->end(); iter++){	
		for(size_t i = 0; i < 3; i++){
			for(size_t j = 0; j < uniqueVert.size(); j++){
				if(iter->vertices[i] == uniqueVert[j]){
					iter->names[i] = uniqueVert[j].name;
					break;
				}
			}
		}
	}
	
	return uniqueVert.size();
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
	
	double worldSize[3];
	for(size_t i = 0; i < 3; i++){
		worldSize[i] = rmax[i]-rmin[i];
	}
	
	masterFile << "    <define>\n";
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){ // Positions
		// <position name="offsetpos" unit="mm" x="19" y="0" z="19"/>
		iter->computeOffset(worldSize[0], worldSize[1], worldSize[2]);
		masterFile << "        <position name=\"" << iter->solidName << ".gdml_pos\" unit=\"mm\" x=\"" << iter->offset.p[0] << "\" y=\"" << iter->offset.p[1] << "\" z=\"" << iter->offset.p[2] << "\"/>\n";
	}
	for(std::vector<gdmlEntry>::iterator iter = goodFiles.begin(); iter != goodFiles.end(); iter++){ // Rotations
		// <rotation name="offsetpos" unit="deg" x="0" y="0" z="0"/>
		masterFile << "        <rotation name=\"" << iter->solidName << ".gdml_rot\" unit=\"deg\" x=\"0\" y=\"0\" z=\"0\"/>\n";
	}
	masterFile << "    </define>\n\n";
	
	masterFile << "    <solids>\n";
	masterFile << "        <box lunit=\"mm\" name=\"" << objName << "_solid\" x=\"" << worldSize[0] << "\" y=\"" << worldSize[1] << "\" z=\"" << worldSize[2] << "\"/>\n";
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
