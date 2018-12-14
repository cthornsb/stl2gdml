#include <sstream>

#include "facet.hpp"

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
