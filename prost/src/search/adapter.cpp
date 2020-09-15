#include "states.h"


DLRSAdapter::DLRSAdapter(_name) : ProbabilisticSearchEngine(_name){
	input.open(in_pipe);
	pstream.open(out_pipe);
}

void DLRSAdapter::initSession(){
	assert(input.good() && pstream.good());

	std::cout << "printing tasks" << std::endl;
    out << numberOfActions;
    std::cout << "printed: number of actions: " << num_actions << std::endl;

    for(int i = 0 ; i < numberOfActions ; ++i){
        out << i;
        std::cout << "printed: " << i << "-th action: " << i << std::endl;
    }

    std::cout << "printing vars" << std::endl;
    out << State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents;
    std::cout << "printed: number of state_variables: " << state_variables->size() << std::endl;

    for(int i = 0 ; i < State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents ; ++i){
        out << i;
        std::cout << "printed: " << i << "-th state variable has 2 domains" << std::endl;

        for(int j = 0 ; j < 2 ; ++j){
            out << j;
            std::cout << "printed: " << i << "-th state, " << j << "-th value: " << *sit << std::endl;
        }
    }
    /*
    for(auto it = state_variables->begin() ; it != state_variables->end() ; it++){
        out << it->size();
        std::cout << "printed: " << std::distance(state_variables->begin(), it) << "-th state variable has " << it->size() << " domains" << std::endl;

        for(auto sit = it->begin() ; sit != it->end() ; sit++){
            out << *sit;
            std::cout << "printed: " << std::distance(state_variables->begin(), it) << "-th state, " << std::distance(it->begin(), sit) << "-th value: " << *sit << std::endl;
        }
    }*/
}

void DLRSAdapter::initRound(){}

void DLRSAdapter::initStep(State const& current){
	printState(current);
}

void DLRSAdapter::finishStep(int immediateReward){
	// remember that the next method called from the IPCClient->Planner->SearchEngine is the finishRound
}

void DLRSAdapter::finishStep(vector<double> *successor, double immediateReward){
    pstream << immediateReward;
    printState(successor);
}

void DLRSAdapter::finishRound(double roundReward){
	out << roundReward;
	std::cout << "Round reward" << roundReward;
	out << "";
}

void DLRSAdapter::finishSession(double totalReward){
    input.flush();
    input.close();

    pstream.flush();
    pstream.close();
}

void DLRSAdapter::estimateBestActions(State const& _rootState, std::vector<int>& bestActions){
    // print executable actions
    out << numberOfActions;
    std::cout << "printed: number of actions: " << num_actions << std::endl;

    for(int i = 0 ; i < numberOfActions ; ++i){
        out << i;
        std::cout << "printed: " << i << "-th action: " << i << std::endl;
    }

	char action[256];
	input.getline(action, 256);
	bestActions.emplace_back(std::atoi(action));
}

void DLRSAdapter::printState(State const& state){
    std::vector<double> stateVariables();

    for (unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
        deterministicStateFluents.push_back(state.getStateFluent(i));
    }
    for (unsigned int i = 0; i < numberOfProbabilisticStateFluents; ++i) {
        probabilisticStateFluents.push_back(state.getStateFluent(i + numberOfDeterministicStateFluents));
    }

    printState(&stateVariables);
}

void DLRSAdapter::printState(vector<double> *stateVariables){
    std::cout << "printing state"
    out << stateVariables->size();
    std::cout << "printed: state size: " << stateVariables->size() << std::endl;

    for(auto it = stateVariables->begin() ; it != stateVariables->end() ; ++it){
        out << *it;
        std::cout << "printed state variable: " << *it << std::endl;
    }
}