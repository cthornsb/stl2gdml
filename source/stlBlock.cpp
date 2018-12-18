#include <iostream>

#include "stlBlock.hpp"

stlBlock::stlBlock(float *array, const unsigned short &att/*=0*/){
	normal = threeTuple(array[0], array[1], array[2]);
	for(size_t i = 1; i < 4; i++){
		vertices[i-1] = threeTuple(array[3*i], array[3*i+1], array[3*i+2]);
	}
	attribute = att;
}

void stlBlock::addOffset(const threeTuple &offset){
	for(size_t i = 0; i < 3; i++){
		//std::cout << "debug: PRE  x=" << vertices[i].p[0] << ", y=" << vertices[i].p[1] << ", z=" << vertices[i].p[2] << std::endl;
		vertices[i] = vertices[i] + offset;
		//std::cout << "debug: POST x=" << vertices[i].p[0] << ", y=" << vertices[i].p[1] << ", z=" << vertices[i].p[2] << std::endl;
	}
}

void stlBlock::operator *= (const double &val){
	for(size_t i = 0; i < 3; i++){
		vertices[i] *= val;
	}
}

bool stlBlock::operator == (const stlBlock &other) const {
	bool validMatch[3] = {false, false, false};
	for(size_t i = 0; i < 3; i++){
		for(size_t j = 0; j < 3; j++){
			//std::cout << vertices[i].dist(other.vertices[j]) << " mm\n";
			if(vertices[i].dist(other.vertices[j]) == 0){
				validMatch[i] = true;
				break;
			}
		}
	}
	return (validMatch[0] && validMatch[1] && validMatch[2]);
}

bool stlBlock::compare(const stlBlock &other) const {
	threeTuple dV[3];
	for(size_t i = 0; i < 3; i++){
		dV[i] = vertices[i] - other.vertices[i];
	}
	return (dV[0].length() == 0 && dV[1].length() == 0 && dV[2].length() == 0);
}

bool stlBlock::usesVertex(const threeTuple &vertex) const {
	return (vertex == vertices[0] || vertex == vertices[1] || vertex == vertices[2]);
}
