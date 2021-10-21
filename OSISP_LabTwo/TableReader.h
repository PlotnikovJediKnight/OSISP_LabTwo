#pragma once
#include <vector>
#include <string>
#include <stdexcept>
#include <fstream>

class TableReader {
public:
	void set_file_path(char* lp_file_path);
	std::vector<std::vector<std::string>> get_file_contents();
private:
	char* _lp_file_path;
};