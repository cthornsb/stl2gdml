#include <iostream>
#include <sstream>

#include "polySolid.hpp"

void polySolid::getSizeX(double &min, double &max) const {
	min = rmin[0]; max = rmax[0];
}

void polySolid::getSizeY(double &min, double &max) const {
	min = rmin[1]; max = rmax[1];
}

void polySolid::getSizeZ(double &min, double &max) const {
	min = rmin[2]; max = rmax[2];
}

void polySolid::getUniqueVertices(std::vector<threeTuple> &unique, const int &id) const {
	for(std::vector<facet>::const_iterator iter = solid.cbegin(); iter != solid.cend(); iter++){	
		for(size_t i = 0; i < 3; i++){
			if(!isInVector(iter->vertices[i], unique)){ // Add it to the vector
				std::stringstream stream;
				stream << "v" << id << "_" << unique.size();
				threeTuple vec(iter->vertices[i]);
				vec.name = stream.str();
				unique.push_back(vec);
				
				/*double y = vec.p[1];
				std::vector<ySlice>::iterator sliceIterator;
				if(isInSlices(y, sliceIterator)){
					sliceIterator->addPoint(vec);
				}
				else{ // Add the slice to the list.
					slices.push_back(ySlice(y));
				}*/
			}
		}
	}
}

void polySolid::getUniquePolygons(std::vector<facet> &unique, const threeTuple &offset) const {
	bool foundMatch;
	for(std::vector<facet>::const_iterator iter1 = solid.cbegin(); iter1 != solid.cend(); iter1++){	
		foundMatch = false;
		for(std::vector<facet>::const_iterator iter2 = unique.cbegin(); iter2 != unique.cend(); iter2++){
			if(iter2->compare((*iter1), threeTuple(0,0,0))){
				foundMatch = true;
				break;
			}
		}
		if(!foundMatch) // Add it to the vector
			unique.push_back((*iter1));
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

void polySolid::addOffset(const threeTuple &offset){
	for(std::vector<facet>::iterator iter = solid.begin(); iter != solid.end(); iter++){	
		iter->addOffset(offset);
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

bool polySolid::isInSlices(const double &y, std::vector<ySlice>::iterator &iter){
	for(iter = slices.begin(); iter != slices.end(); iter++){	
		if(iter->isInSlice(y)) return true;
	}
	return false;
}
