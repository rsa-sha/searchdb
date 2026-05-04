#include<iostream>
#include <fstream>


#include "common/mmap_file.hpp"
#include "common/result.hpp"
#include "common/timer.hpp"



int main() {
	std::cout<<"searchdb v0.0.1"<<std::endl;
	
	// Mmap basic-sanity check
	std::ofstream("test.txt") << "hello.mmap";

	MmapFile file("test.txt");

	std::string content(
			reinterpret_cast<const char*>(file.data()),
			file.size()
	);

	std::cout<< content <<std::endl;

	return 0;
}
