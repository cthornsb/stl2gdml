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

void polySolid::compare(polySolid &other){
	for(std::vector<facet>::iterator iter1 = solid.begin(); iter1 != solid.end(); iter1++){	
		for(std::vector<facet>::iterator iter2 = other.begin(); iter2 != other.end(); iter2++){	
			if(iter1->compare((*iter2)))
				std::cout << " BLAH\n";
		}
	}
}

void polySolid::initialize(){ 
	for(size_t i = 0; i < 3; i++){
		rmin[i] = 1E10;
		rmax[i] = -1E10;
	}
}

bool polySolid::isInVector(const threeTuple &tuple, const std::vector<threeTuple> &vec){
	for(std::vector<threeTuple>::const_iterator iter = vec.begin(); iter != vec.end(); iter++){	
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
