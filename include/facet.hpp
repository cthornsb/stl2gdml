#ifndef FACET_HPP
#define FACET_HPP

#include <string>
#include <vector>

#include "stlBlock.hpp"

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
	
	bool usesVertex(const std::string &vertexName) const ;
	
	std::string print() const ;
};

#endif
