#include "TableReader.h"

void TableReader::set_file_path(char* lp_file_path) { this->_lp_file_path = lp_file_path; }

std::vector<std::vector<std::string>> 
TableReader::get_file_contents() {
	std::ifstream in(_lp_file_path);
	
	size_t N, M;
	in >> N >> M;

	std::vector<std::vector<std::string>> toReturn;
	toReturn.resize(N);
	for (auto& it : toReturn) it.resize(M);

	for (size_t i = 0; i < N; ++i) {
		for (size_t j = 0; j < M; ++j) {
			getline(in, toReturn[i][j], ';');
			size_t index = toReturn[i][j].find('\n');
			if (index < toReturn[i][j].length()) toReturn[i][j].erase(index, 1);
		}
	}

	return toReturn;
}