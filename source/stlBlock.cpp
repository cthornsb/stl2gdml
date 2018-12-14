#include "stlBlock.hpp"

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
