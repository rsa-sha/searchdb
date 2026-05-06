#include<iostream>
#include <fstream>


#include "common/mmap_file.hpp"
#include "common/result.hpp"
#include "common/timer.hpp"

#include "crawler/http_client.hpp"


int main() {
	std::cout<<"searchdb v0.0.1"<<std::endl;
	
	// Mmap basic-sanity check
	std::ofstream("test.txt") << "hello.mmap";

	MmapFile file("test.txt");

	std::string content(
			reinterpret_cast<const char*>(file.data()),
			file.size()
	);

	// Crawler fetch sanity test
	std::cout<< content <<std::endl;
	std::string url = "https://en.wikipedia.org/wiki/Web_crawler";
	auto res = fetch(url);
	std::cout<<"Fetching data from url -> "<<url<<std::endl;

	if (res) {
		auto& result = res.value();
		std::cout << result.status_code <<std::endl;
		std::cout << result.body.size() <<std::endl;
		std::cout << "Fetched data successfully"<<std::endl;
	} else {
		std::cout << "Failed to fetch data from "<<url<<" | Reason - ";
		std::cout << to_string(res.error())<<std::endl;
	}
	return 0;
}
