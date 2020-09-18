#include "adapter.h"

#include "../states.h"


DLRSAdapter::DLRSAdapter(std::string _name) : ProbabilisticSearchEngine(_name){
	input.open(in_pipe);
	pstream.open(out_pipe);
}

void DLRSAdapter::initSession(){
    assert(input.good() && pstream.good());
    std::cout << "[ADAPT.]: printing tasks" << std::endl;
    pstream << numberOfActions;
    std::cout << "[ADAPT.]: printed: number of actions: " << numberOfActions << std::endl;

    for(int i = 0 ; i < numberOfActions ; ++i){
        pstream << i;
        std::cout << "[ADAPT.]: printed: " << i << "-th action: " << i << std::endl;
    }

    std::cout << "[ADAPT.]: printing vars" << std::endl;
    pstream << State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents;
    std::cout << "[ADAPT.]: printed: number of state_variables: " << State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents << std::endl;

    for(int i = 0 ; i < State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents ; ++i){
        pstream << i;
        std::cout << "[ADAPT.]: printed: " << i << "-th state variable has 2 domains" << std::endl;

        for(int j = 0 ; j < 2 ; ++j){
            pstream << j;
            std::cout << "[ADAPT.]: printed: " << i << "-th state, " << j << "-th value: " << j << std::endl;
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

void DLRSAdapter::finishStep(double/* immediateReward*/){
	// remember that the next method called from the IPCClient->Planner->SearchEngine is the finishRound
}

void DLRSAdapter::finishStep(std::vector<double> *successor, const double& immediateReward){
    pstream << immediateReward;
    printState(successor);
}

void DLRSAdapter::finishRound(double roundReward){
	pstream << roundReward;
	std::cout << "[ADAPT.]: Round reward" << roundReward;
	pstream << "";
}

void DLRSAdapter::finishSession(double/* totalReward*/){
    input.close();

    pstream.flush();
    pstream.close();
}

void DLRSAdapter::estimateBestActions(State const&/* _rootState*/, std::vector<int>& bestActions){
    // print executable actions
    pstream << numberOfActions;
    std::cout << "[ADAPT.]: printed: number of actions: " << numberOfActions << std::endl;

    for(int i = 0 ; i < numberOfActions ; ++i){
        pstream << i;
        std::cout << "[ADAPT.]: printed: " << i << "-th action: " << i << std::endl;
    }

	char action[256];
	input.getline(action, 256);
	bestActions.emplace_back(std::atoi(action));
}

void DLRSAdapter::printState(State const&/* state*/){
    std::vector<double> stateVariables;

    for (unsigned int i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
        //stateVariables.emplace_back(state.getStateFluent(i));
    }
    for (unsigned int i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
        //stateVariables.emplace_back(state.getStateFluent(i + State::numberOfDeterministicStateFluents));
    }

    printState(&stateVariables);
}

void DLRSAdapter::printState(std::vector<double> *stateVariables){
    std::cout << "[ADAPT.]: printing state";
    pstream << stateVariables->size();
    std::cout << "[ADAPT.]: printed: state size: " << stateVariables->size() << std::endl;

    for(auto it = stateVariables->begin() ; it != stateVariables->end() ; ++it){
        pstream << *it;
        std::cout << "[ADAPT.]: printed state variable: " << *it << std::endl;
    }
}
