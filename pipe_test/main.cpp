#include<vector>
#include<iostream>
#include<fstream>
#include<iterator>

#include "pstream.h"


void print_start(std::vector<std::vector<int>>*, int, PStream&);


int main(int argc, char** argv){
	std::cout << "Opening pipes ..." << std::endl;
	std::ofstream output("/home/roman/parser/PSTATE");
	std::ifstream input("/home/roman/parser/PACTION");
	PStream pstream("/home/roman/parser/PSTATE");

	std::vector<std::vector<int>> state_variables(3);
	state_variables[0].emplace_back(0);
	state_variables[0].emplace_back(1);
	state_variables[1].emplace_back(0);
	state_variables[1].emplace_back(1);
	state_variables[2].emplace_back(0);
	state_variables[2].emplace_back(1);
	
	std::vector<int> actions(6);

	print_start(&state_variables, actions.size(), pstream);
}


void print_start(std::vector<std::vector<int>>* state_variables, int num_actions, PStream& out){
	std::cout << "printing tasks" << std::endl;
	out << num_actions;
	std::cout << "printed: number of actions: " << num_actions << std::endl;

	for(int i = 0 ; i < num_actions ; ++i){
		out << i;
		std::cout << "printed: " << i << "-th action: " << i << std::endl;
	}
	
	std::cout << "printing vars" << std::endl;
	out << state_variables->size();
	std::cout << "printed: number of state_variables: " << state_variables->size() << std::endl;

	for(auto it = state_variables->begin() ; it != state_variables->end() ; it++){
		out << it->size();
		std::cout << "printed: " << std::distance(state_variables->begin(), it) << "-th state variable has " << it->size() << " domains" << std::endl;

		for(auto sit = it->begin() ; sit != it->end() ; sit++){
			out << *sit;
			std::cout << "printed: " << std::distance(state_variables->begin(), it) << "-th state, " << std::distance(it->begin(), sit) << "-th value: " << *sit << std::endl;
		}
	}
}

