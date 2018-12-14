#include <iostream>
#include <vector>
#include <stdlib.h>
#include <string.h>

#include "geantGdmlFile.hpp"

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
