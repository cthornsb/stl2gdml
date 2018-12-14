#include <sstream>

#include "threeTuple.hpp"

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
