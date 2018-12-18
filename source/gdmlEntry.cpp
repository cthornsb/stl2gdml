#include <iostream>

#include "gdmlEntry.hpp"

void gdmlEntry::computeOffset(const double &sizeX_, const double &sizeY_, const double &sizeZ_, bool debug/*=false*/){
	threeTuple anchor = solid.getMinimumPoint(); // Native, uncorrected geometry anchor point.
	if(debug){
		std::cout << "debug: worldX=" << sizeX_ << ", worldY=" << sizeY_ << ", worldZ=" << sizeZ_ << std::endl;
		std::cout << "debug: anchorX=" << anchor.p[0] << ", anchorY=" << anchor.p[1] << ", anchorZ=" << anchor.p[2] << std::endl;		
		std::cout << "debug: physSizeX=" << physSize.p[0] << ", physSizeY=" << physSize.p[1] << ", physSizeZ=" << physSize.p[2] << std::endl;
	}
	
	double xoffset = -sizeX_/2 + (sizeX_ - physSize.p[0])/2;
	double yoffset = -sizeY_/2 + (sizeY_ - physSize.p[1])/2;
	double zoffset = -sizeZ_/2 + (sizeZ_ - physSize.p[2])/2;
	
	bool axisIsOffset[3] = {false, false, false};
	threeTuple anchorOffset;
	for(size_t i = 0; i < 3; i++){
		if(anchor.p[i] >= 1E-3){
			axisIsOffset[i] = true;
			anchorOffset.p[i] = anchor.p[i];
		}
	}

	// Compute the offset.
	if(axisIsOffset[0] || axisIsOffset[1] || axisIsOffset[2]){ // Anchor point is not at the origin and needs to be corrected.
		std::cout << " Warning! Geometry anchor point is not at the origin as expected (x=" << anchorOffset.p[0] << " mm, y=" << anchorOffset.p[1] << " mm, z=" << anchorOffset.p[2] << " mm)!\n";
		offset = threeTuple(xoffset, yoffset, zoffset) - anchorOffset;
	}
	else{ // Anchor point is at the origin.
		offset = threeTuple(xoffset, yoffset, zoffset);
	}
}
