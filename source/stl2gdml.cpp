#include <iostream>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <string.h>

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

unsigned int readAST(const char *fname, std::vector<facet> &solid, const double &unit=mm){
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
				solid.push_back(triangle);
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

unsigned int readSTL(const char *fname, std::vector<facet> &solid, const double &unit=mm){
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
		solid.push_back(triangle);
	}
	
	file.close();	
	
	if(invalidRead){
		std::cout << " Warning: Failed to read all " << nTriangles << " triangles specified in header!\n";
	}
	
	return solid.size();
}

bool isInVector(const threeTuple &tuple, const std::vector<threeTuple> &solid){
	for(std::vector<threeTuple>::const_iterator iter = solid.begin(); iter != solid.end(); iter++){	
		if(tuple == (*iter)) return true;
	}
	return false;
}

void generateHeader(std::ostream &ofile){
	ofile << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n";
	ofile << "<gdml xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd\">\n\n";
}

unsigned int generateUniqueVertices(std::ostream &ofile, std::vector<facet> &solid, const std::string &name, const int &id){
	std::vector<threeTuple> unique;
	for(std::vector<facet>::iterator iter = solid.begin(); iter != solid.end(); iter++){	
		for(int i = 0; i < 3; i++){
			if(!isInVector(iter->vertices[i], unique)){ // Add it to the vector
				std::stringstream stream;
				stream << "v" << id << "_" << unique.size();
				threeTuple vec(iter->vertices[i]);
				vec.name = stream.str();
				unique.push_back(vec);
			}
		}
	}

	ofile << "    <define>\n";

	for(size_t j = 0; j < unique.size(); j++){
		ofile << "             " << unique[j].print() << std::endl;
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

void generateFooter(std::ostream &ofile, const std::string &name){
	ofile << "    <structure>\n";
	ofile << "        <volume name=\"" << name << ".gdml\">\n";
	ofile << "            <materialref ref=\"Vacuum\"/>\n";
	ofile << "            <solidref ref=\"" << name << "\"/>\n";
	ofile << "        </volume>\n";
	ofile << "    </structure>\n\n";

	ofile << "    <setup name=\"Default\" version=\"1.0\">\n";
	ofile << "        <world ref=\"" << name << ".gdml\"/>\n";
	ofile << "    </setup>\n";
	ofile << "</gdml>\n";
}

void generateMasterFileHeader(std::ofstream &ofile, const double &x=10000.0, const double &y=10000.0, const double &z=10000.0){
	generateHeader(ofile);
	ofile << "    <materials>\n";
	ofile << "        <!--          -->\n";
	ofile << "        <!-- elements -->\n";
	ofile << "        <!--          -->\n";
	ofile << "        <element name=\"videRef\" formula=\"VACUUM\" Z=\"1\"> <atom value=\"1.\"/> </element>\n\n";

	ofile << "        <!--          -->\n";
	ofile << "        <!-- vacuum   -->\n";
	ofile << "        <!--          -->\n";
	ofile << "        <material formula=\" \" name=\"Vacuum\">\n";
	ofile << "            <D value=\"1.e-25\" unit=\"g/cm3\"/>\n";
	ofile << "            <fraction n=\"1.0\" ref=\"videRef\"/>\n";
	ofile << "        </material>\n";
	ofile << "    </materials>\n\n";

	ofile << "    <solids>\n";
	ofile << "        <box lunit=\"mm\" name=\"world_solid\" x=\"" << x << "\" y=\"" << y << "\" z=\"" << z << "\"/>\n";
	ofile << "    </solids>\n\n";

	ofile << "    <structure>\n";
	ofile << "        <volume name=\"world_volume\">\n";
	ofile << "            <materialref ref=\"Vacuum\"/>\n";
	ofile << "            <solidref ref=\"world_solid\"/>\n\n";
}

void generateMasterFileGDML(std::ofstream &ofile, const std::string &filename){
	// <file name="/path/to/file/file.gdml"/>
	ofile << "            <physvol>\n";
	ofile << "                <file name=\"" << filename << "\"/>\n";
	ofile << "            </physvol>\n\n";
}

void generateMasterFileFooter(std::ofstream &ofile){
	ofile << "        </volume>\n";
	ofile << "    </structure>\n\n";

	ofile << "    <setup name=\"Default\" version=\"1.0\">\n";
	ofile << "        <world ref=\"world_volume\"/>\n";
	ofile << "    </setup>\n";
	ofile << "</gdml>\n";
}

void help(char * prog_name_){
	std::cout << "  SYNTAX: " << prog_name_ << " <output> <input> [input2 input3 ...]\n";
	std::cout << "   Available options:\n";
	std::cout << "    --help (-h)              | Display this dialogue.\n";
	std::cout << "    --unit <unit>            | Specify the name of the size unit [e.g. in, ft, m, dm, cm, mm] (default is mm).\n";
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
	std::vector<std::string> inputFilenames;
	double drawingUnit = mm;

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

	std::ofstream masterFile(outputFilename.c_str());
	generateMasterFileHeader(masterFile);

	int solidCount = 0;
	for(std::vector<std::string>::iterator iter = inputFilenames.begin(); iter != inputFilenames.end(); iter++){
		std::string solidName = "Thingy";
		std::string gdmlFilename = (*iter);
		std::string extension = "";
		size_t index = iter->find_last_of('.');
		if(index != std::string::npos){
			solidName = iter->substr(0, index);
			gdmlFilename = iter->substr(0, index) + ".gdml";
			extension = iter->substr(index+1);
		}
			
		index = solidName.find_last_of('/');
		if(index != std::string::npos){
			solidName = solidName.substr(index+1);
		}
			
		std::cout << " Processing file \"" << (*iter) << "\", solid=" << solidName << "\n";
		
		std::vector<facet> solid;
		if(extension == "stl"){ // Binary STL file
			readSTL(iter->c_str(), solid, drawingUnit);
			std::cout << "  Read " << solid.size() << " polygons\n";
		}
		else{
			if(extension != "ast")
				std::cout << " Warning: Unknown file type (" << extension << "), assuming AST format.\n";
			unsigned int lines = readAST(iter->c_str(), solid, drawingUnit);
			std::cout << "  Read " << lines << " lines and " << solid.size() << " polygons\n";
		}
		
		std::ofstream ofile(gdmlFilename.c_str());
		generateHeader(ofile);
		
		std::cout << "  Identified " << generateUniqueVertices(ofile, solid, solidName, solidCount++) << " unique vertices\n";
		
		generateFooter(ofile, solidName);
		ofile.close();

		std::cout << "  Generated output file \"" << gdmlFilename << "\"\n";
		
		generateMasterFileGDML(masterFile, gdmlFilename);
	}
	
	generateMasterFileFooter(masterFile);
	masterFile.close();

	std::cout << " Generated master output file \"" << outputFilename << "\"\n";

	return 0;
}
