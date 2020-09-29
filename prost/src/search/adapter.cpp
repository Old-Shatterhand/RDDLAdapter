#include "adapter.h"

#include "../states.h"


DLRSAdapter::DLRSAdapter(std::string _name) : ProbabilisticSearchEngine(_name){
	std::cout << "[ADAPT.] Initializing: Opening pipes..." << std::endl;
	pstream.open(out_pipe);
	input.open(in_pipe);
}

void DLRSAdapter::initSession(){
    std::cout << "[ADAPT.] Init session: Sending problem data" << std::endl;
    assert(input.good() && pstream.good());
    pstream << numberOfActions;

    for(int i = 0 ; i < numberOfActions ; ++i){
        pstream << i;
    }

    pstream << State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents;

    for(int i = 0 ; i < State::numberOfDeterministicStateFluents + State::numberOfProbabilisticStateFluents ; ++i){
        pstream << 2;

        for(int j = 0 ; j < 2 ; ++j){
            pstream << j;
        }
    }
}

void DLRSAdapter::initRound(){}

void DLRSAdapter::initStep(State const& current){
	std::cout << "[ADAPT.] Initialize step: Sending state" << std::endl;
	printState(current);
}

void DLRSAdapter::finishStep(double/* immediateReward*/){
	// remember that the next method called from the IPCClient->Planner->SearchEngine is the finishRound
}

void DLRSAdapter::finishStep(std::vector<double> * /*successor*/, const double& immediateReward){
    std::cout << "[ADAPT.] Finish step: Sending successor state" << std::endl;
    pstream << immediateReward;
    pstream << immediateReward;
    //printState(successor);
}

void DLRSAdapter::finishRound(double roundReward){
	std::cout << "[ADAPT.] Finish round: Sending total reward" << std::endl;
	pstream << roundReward;
	pstream << "";
}

void DLRSAdapter::finishSession(double/* totalReward*/){
    std::cout << "[ADAPT.] Finish session: Closing pipes..." << std::endl;
    input.close();

    pstream.flush();
    pstream.close();
}

void DLRSAdapter::estimateBestActions(State const&/* _rootState*/, std::vector<int>& bestActions){
    std::cout << "[ADAPT.] Estimate best action" << std::endl;
    // print executable actions
    pstream << numberOfActions;

    for(int i = 0 ; i < numberOfActions ; ++i){
        pstream << i;
    }

	char action[256];
	input.getline(action, 256);
	bestActions.emplace_back(std::atoi(action));
	std::cout << "[ADAPT.] Next Action selected: " << std::atoi(action) << std::endl;
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
    pstream << stateVariables->size();

    for(auto it = stateVariables->begin() ; it != stateVariables->end() ; ++it){
        pstream << *it;
    }
}
