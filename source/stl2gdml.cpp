#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <string.h>
#include <algorithm>

const double in = 25.4;
const double ft = 304.8;
const double m = 1000;
const double dm = 100;
const double cm = 10;
const double mm = 1;

class threeTuple{
  public:
	double p[3];
	
	std::string name;
  
	threeTuple() : name() { zero(); }
	
	threeTuple(const double &x_, const double &y_, const double &z_) : name() { p[0] = x_; p[1] = y_; p[2] = z_; }
	
	threeTuple(const std::string &str_); 
	
	bool operator == (const threeTuple &other) const { return (p[0]==other.p[0] && p[1]==other.p[1] && p[2]==other.p[2]); }
	
	void operator *= (const double &val){ p[0] *= val; p[1] *= val; p[2] *= val; }
	
	std::string print() const ;
	
	void zero(){ p[0] = 0; p[1] = 0; p[2] = 0; }
};

threeTuple::threeTuple(const std::string &str_) : name() {
	zero();
	size_t index1, index2=0;
	std::string substr;
	for(size_t i = 0; i < 3; i++){
		index1 = str_.find_first_not_of(' ', index2);
		if(index1 == std::string::npos) break;
		index2 = str_.find_first_of(' ', index1);
		if(index2 == std::string::npos)
			substr = str_.substr(index1);
		else
			substr = str_.substr(index1, index2-index1);
		p[i] = strtod(substr.c_str(), NULL);
	}
}

std::string threeTuple::print() const {
	std::stringstream stream;
	stream << "<position name=\"" << name << "\" unit=\"mm\" x=\"" << p[0] << "\" y=\"" << p[1] << "\" z=\"" << p[2] << "\"/>";
	return stream.str();
}

class stlBlock{
  public:
	threeTuple normal;
	threeTuple vertices[3];

	unsigned short attribute;

	stlBlock(){ }

	stlBlock(float *array, const unsigned short &att=0);
	
	void operator *= (const double &val);
};

stlBlock::stlBlock(float *array, const unsigned short &att/*=0*/){
	normal = threeTuple(array[0], array[1], array[2]);
	for(size_t i = 1; i < 4; i++){
		vertices[i-1] = threeTuple(array[3*i], array[3*i+1], array[3*i+2]);
	}
	attribute = att;
}

void stlBlock::operator *= (const double &val){
	for(size_t i = 0; i < 3; i++){
		vertices[i] *= val;
	}
}

class facet : public stlBlock {
  public:
	std::string names[3];
	
	bool good;
	
	size_t vertex;
	
	facet() : stlBlock(), good(false), vertex(0) { }

	facet(const stlBlock &block) : stlBlock(block), good(true), vertex(3) { }
	
	facet(const std::vector<std::string> &block);

	bool readBlock(const std::vector<std::string> &block);
	
	bool checkNames() const ;
	
	std::string print() const ;
};

facet::facet(const std::vector<std::string> &block) : stlBlock() {
	this->readBlock(block);
}

bool facet::readBlock(const std::vector<std::string> &block){
	vertex = 0;
	size_t index;
	for(std::vector<std::string>::const_iterator iter = block.begin(); iter != block.end(); iter++){
		index = iter->find("normal");
		if(index != std::string::npos){ // Normal vector
			normal = threeTuple(iter->substr(index+6));
		}
		index = iter->find("vertex");
		if(index != std::string::npos){ // Vertex vector
			if(vertex >= 3)
				return false;
			vertices[vertex++] = threeTuple(iter->substr(index+6));
		}
	}
	good = (vertex == 3);
	return good;
}

bool facet::checkNames() const {
	return (!names[0].empty() && !names[1].empty() && !names[2].empty());
}

std::string facet::print() const {
	std::stringstream stream;
	stream << "<triangular vertex1=\"" << names[0] << "\" vertex2=\"" << names[1] << "\" vertex3=\"" << names[2] << "\"/>";
	return stream.str();
}

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
  
  private:
	std::vector<facet> solid;
	std::vector<ySlice> slices; // Bounding box for each y-slice of the model.
	
	double rmin[3];
	double rmax[3];
	
	void initialize();
	
	bool isInVector(const threeTuple &tuple, const std::vector<threeTuple> &solid);
	
	bool isInSlices(const double &y, std::vector<ySlice>::iterator &iter);
};

void polySolid::getSizeX(double &min, double &max) const {
	min = rmin[0]; max = rmax[0];
}

void polySolid::getSizeY(double &min, double &max) const {
	min = rmin[1]; max = rmax[1];
}

void polySolid::getSizeZ(double &min, double &max) const {
	min = rmin[2]; max = rmax[2];
}

void polySolid::getUniqueVertices(std::vector<threeTuple> &unique, const int &id){
	slices.clear();
	for(std::vector<facet>::iterator iter = solid.begin(); iter != solid.end(); iter++){	
		for(size_t i = 0; i < 3; i++){
			if(!isInVector(iter->vertices[i], unique)){ // Add it to the vector
				std::stringstream stream;
				stream << "v" << id << "_" << unique.size();
				threeTuple vec(iter->vertices[i]);
				vec.name = stream.str();
				unique.push_back(vec);
				
				double y = vec.p[1];
				std::vector<ySlice>::iterator sliceIterator;
				if(isInSlices(y, sliceIterator)){
					sliceIterator->addPoint(vec);
				}
				else{ // Add the slice to the list.
					slices.push_back(ySlice(y));
				}
			}
		}
	}
}

void polySolid::add(const facet &poly){ 
	solid.push_back(poly); 
	for(size_t i = 0; i < 3; i++){
		threeTuple vec(poly.vertices[i]);
		for(size_t j = 0; j < 3; j++){
			if(vec.p[j] < rmin[j]) rmin[j] = vec.p[j];
			if(vec.p[j] > rmax[j]) rmax[j] = vec.p[j];
		}
	}
}

void polySolid::clear(){ 
	solid.clear(); 
	initialize();
}

void polySolid::initialize(){ 
	for(size_t i = 0; i < 3; i++){
		rmin[i] = 1E10;
		rmax[i] = -1E10;
	}
}

bool polySolid::isInVector(const threeTuple &tuple, const std::vector<threeTuple> &solid){
	for(std::vector<threeTuple>::const_iterator iter = solid.begin(); iter != solid.end(); iter++){	
		if(tuple == (*iter)) return true;
	}
	return false;
}

bool polySolid::isInSlices(const double &y, std::vector<ySlice>::iterator &iter){
	for(iter = slices.begin(); iter != slices.end(); iter++){	
		if(iter->isInSlice(y)) return true;
	}
	return false;
}

unsigned int readAST(const char *fname, polySolid &solid, const double &unit=mm){
	std::ifstream file(fname);
	if(!file.good())
		return 0;
	
	solid.clear();
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
				solid.add(triangle);
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

unsigned int readSTL(const char *fname, polySolid &solid, const double &unit=mm){
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

	solid.clear();
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
		solid.add(triangle);
	}
	
	file.close();	
	
	if(invalidRead){
		std::cout << " Warning: Failed to read all " << nTriangles << " triangles specified in header!\n";
	}
	
	return solid.size();
}

class gdmlEntry{
  public:
	std::string filename;
	std::string solidName;
	threeTuple physSize;
	threeTuple offset;
	
	gdmlEntry(){ }
	
	gdmlEntry(const std::string &fname_, const std::string &sname_, const threeTuple &size_) : filename(fname_), solidName(sname_), physSize(size_) { }
	
	void computeOffset(const double &sizeX_, const double &sizeY_, const double &sizeZ_);
};

void gdmlEntry::computeOffset(const double &sizeX_, const double &sizeY_, const double &sizeZ_){
	double xoffset = -sizeX_/2 + (sizeX_ - physSize.p[0])/2;
	double yoffset = -sizeY_/2 + (sizeY_ - physSize.p[1])/2;
	double zoffset = -sizeZ_/2 + (sizeZ_ - physSize.p[2])/2;
	offset = threeTuple(xoffset, yoffset, zoffset);
}

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

	std::ofstream ofile;

	double rmin[3];
	double rmax[3];

	std::vector<gdmlEntry> goodFiles; // List of good gdml files.
	
	polySolid solid;

	bool processFile(const std::string &filename);

	unsigned int generateUniqueVertices(const std::string &name, threeTuple &size);

	bool generateMasterFile(const std::string &outputFilename);
};

bool geantGdmlFile::process(const std::string &outputFilename, const std::vector<std::string> &filenames){
	for(size_t i = 0; i < 3; i++){
		rmin[i] = 1E10;
		rmax[i] = -1E10;
	}

	goodFiles.clear();

	std::string gdmlFilename;
	for(std::vector<std::string>::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++){
		if(processFile((*iter))){
			std::cout << "  Generated output file \"" << gdmlFilename << "\"\n";
		}
	}
	
	generateMasterFile(outputFilename);
	
	return true;
}
		
bool geantGdmlFile::processFile(const std::string &filename){	
	solid.clear();

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
	
	if(extension == "stl"){ // Binary STL file
		readSTL(filename.c_str(), solid, drawingUnit);
		std::cout << "  Read " << solid.size() << " polygons\n";
	}
	else{ // Ascii STL file
		if(extension != "ast")
			std::cout << " Warning: Unknown file type (" << extension << "), assuming AST format.\n";
		unsigned int lines = readAST(filename.c_str(), solid, drawingUnit);
		std::cout << "  Read " << lines << " lines and " << solid.size() << " polygons\n";
	}
	
	ofile.open(gdmlFilename.c_str());
	if(!ofile.good())
		return false;
	
	ofile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
	ofile << "<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd\">\n\n";
	
	threeTuple physicalSize;
	std::cout << "  Identified " << generateUniqueVertices(solidName, physicalSize) << " unique vertices\n";
	
	ofile << "    <structure>\n";
	ofile << "        <volume name=\"" << solidName << ".gdml\">\n";
	ofile << "            <materialref ref=\"" << materialName << "\"/>\n";
	ofile << "            <solidref ref=\"" << solidName << "\"/>\n";
	ofile << "            <positionref ref=\"" << solidName << ".gdml_pos\"/>\n";
	ofile << "            <rotationref ref=\"" << solidName << ".gdml_rot\"/>\n";
	ofile << "        </volume>\n";
	ofile << "    </structure>\n\n";

	ofile << "    <setup name=\"Default\" version=\"1.0\">\n";
	ofile << "        <world ref=\"" << solidName << ".gdml\"/>\n";
	ofile << "    </setup>\n";
	ofile << "</gdml>\n";
	
	ofile.close();
	
	if(debug){
		std::vector<ySlice> *slices = solid.getSlices();
		std::cout << "debug: slices->size()=" << slices->size() << std::endl;
		for(size_t i = 0; i < slices->size(); i++){
			if(!slices->at(i).empty()){
				std::cout << "debug:  y=" << slices->at(i).getY() << ", x=" << slices->at(i).getSizeX() << ", z=" << slices->at(i).getSizeZ() << "\n";
			}
		}
	}
	
	goodFiles.push_back(gdmlEntry(gdmlFilename, solidName, physicalSize));
	
	return true;
}

unsigned int geantGdmlFile::generateUniqueVertices(const std::string &name, threeTuple &size){
	std::vector<threeTuple> unique;
	solid.getUniqueVertices(unique, solidCount++);
	
	// Get the physical size of the solid.
	double solidmin[3], solidmax[3];
	solid.getSizeX(solidmin[0], solidmax[0]);
	solid.getSizeY(solidmin[1], solidmax[1]);
	solid.getSizeZ(solidmin[2], solidmax[2]);
	
	size = threeTuple(solidmax[0]-solidmin[0], solidmax[1]-solidmin[1], solidmax[2]-solidmin[2]);
	
	for(size_t i = 0; i < 3; i++){
		rmin[i]	= std::min(rmin[i], solidmin[i]); 
		rmax[i] = std::max(rmax[i], solidmax[i]);
	}
	
	ofile << "    <define>\n";

	for(size_t i = 0; i < unique.size(); i++){
		ofile << "             " << unique[i].print() << std::endl;
	}

	ofile << "    </define>\n\n";
	
	// Match all facet vertices with one of the unique vertices.
	for(std::vector<facet>::iterator iter = solid.begin(); iter != solid.end(); iter++){	
		for(size_t i = 0; i < 3; i++){
			for(size_t j = 0; j < unique.size(); j++){
				if(iter->vertices[i] == unique[j]){
					iter->names[i] = unique[j].name;
					break;
				}
			}
		}
	}

	ofile << "    <solids>\n";
	ofile << "        <tessellated aunit=\"deg\" lunit=\"mm\" name=\"" << name << "\">\n";

	// Print the triangular definitions to the output file.
	for(std::vector<facet>::iterator iter = solid.begin(); iter != solid.end(); iter++){
		if(!iter->checkNames()){
			std::cout << " ERROR\n";
		}
		else{
			ofile << "             " << iter->print() << std::endl;
		}
	}
	
	ofile << "        </tessellated>\n";
	ofile << "    </solids>\n\n";
	
	return unique.size();
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
	
	/*masterFile << "    <materials>\n";
	masterFile << "        <!--          -->\n";
	masterFile << "        <!-- elements -->\n";
	masterFile << "        <!--          -->\n";
	masterFile << "        <element name=\"videRef\" formula=\"VACUUM\" Z=\"1\"> <atom value=\"1.\"/> </element>\n\n";

	masterFile << "        <!--          -->\n";
	masterFile << "        <!-- vacuum   -->\n";
	masterFile << "        <!--          -->\n";
	masterFile << "        <material formula=\" \" name=\"Vacuum\">\n";
	masterFile << "            <D value=\"1.e-25\" unit=\"g/cm3\"/>\n";
	masterFile << "            <fraction n=\"1.0\" ref=\"videRef\"/>\n";
	masterFile << "        </material>\n";
	masterFile << "    </materials>\n\n";*/

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

void help(char * prog_name_){
	std::cout << "  SYNTAX: " << prog_name_ << " <output> <input> [input2 input3 ...]\n";
	std::cout << "   Available options:\n";
	std::cout << "    --help (-h)              | Display this dialogue.\n";
	std::cout << "    --unit <unit>            | Specify the name of the size unit [e.g. in, ft, m, dm, cm, mm] (default is mm).\n";
	std::cout << "    --material <name>        | Specify the material of the model (default is \"G4_AIR\").\n";
	std::cout << "    --debug                   | Enable debug mode.\n";
}

int main(int argc, char* argv[]){
	if(argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)){
		help(argv[0]);
		return 1;
	}
	else if(argc < 3){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 2, received " << argc-1 << ".\n";
		help(argv[0]);
		return 1;
	}
	
	std::string outputFilename;
	std::string materialName;
	std::vector<std::string> inputFilenames;
	double drawingUnit = mm;
	bool debugMode = false;

	int argCount = 1;
	int index = 1;
	while(index < argc){
		if(strcmp(argv[index], "--unit") == 0){
			if(index + 1 >= argc){
				std::cout << " Error: Missing required argument to '--unit'!\n";
				help(argv[0]);
				return 1;
			}
			std::string arg(argv[++index]);
			if(arg == "in")
				drawingUnit = in;
			else if(arg == "ft")
				drawingUnit = ft;
			else if(arg == "m")
				drawingUnit = m;
			else if(arg == "dm")
				drawingUnit = dm;
			else if(arg == "cm")
				drawingUnit = cm;
			else if(arg == "mm")
				drawingUnit = mm;
			else{
				std::cout << " Error: Invalid size unit (" << arg << ")!\n";
				return 2;
			}
		}
		else if(strcmp(argv[index], "--material") == 0){
			if(index + 1 >= argc){
				std::cout << " Error: Missing required argument to '--material'!\n";
				help(argv[0]);
				return 1;
			}
			materialName = std::string(argv[++index]);
		}
		else if(strcmp(argv[index], "--debug") == 0){
			debugMode = true;
		}
		else if(argCount++ == 1)
			outputFilename = std::string(argv[index]);
		else
			inputFilenames.push_back(std::string(argv[index]));
		index++;
	}
	
	if(outputFilename.empty()){ // Check for missing output filename.
		std::cout << " Error: No output filename specified!\n";
		return 3;
	}

	if(inputFilenames.empty()){ // Check for missing input filenames.
		std::cout << " Error: No input filename(s) specified!\n";
		return 4;
	}

	std::cout << " Using 1 size unit = " << drawingUnit << " mm.\n";

	geantGdmlFile handler;
	handler.setDrawingUnit(drawingUnit);
	if(debugMode) handler.toggleDebug();
	
	if(!materialName.empty()){ // Set the name of the material.
		std::cout << " Using material = \"" << materialName << "\".\n";
		handler.setMaterialName(materialName);
	}
	
	handler.process(outputFilename, inputFilenames);

	std::cout << " Generated master output file \"" << outputFilename << "\"\n";

	return 0;
}
