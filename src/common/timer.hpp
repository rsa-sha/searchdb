#pragma once
/* Performance Visibility Tool */
#include <chrono>
#include <iostream>
#include <string>

class ScopedTimer {
public:
	explicit ScopedTimer(std::string label)
		: label_(std::move(label)),
		start_(std::chrono::high_resolution_clock::now()) {}
	
	~ScopedTimer() {
		auto end = std::chrono::high_resolution_clock::now();
		auto ms= std::chrono::duration_cast<std::chrono::milliseconds>(end-start_).count();
		std::cout<<"["<<label_<<"] elapsed: "<<ms<<std::endl;
	}

private:
	std::string label_;
	std::chrono::high_resolution_clock::time_point start_;
};
