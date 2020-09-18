#ifndef ADAPTER_H
#define ADAPTER_H

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "search_engine.h"

#include "utils/pstream.h"

class DLRSAdapter : public ProbabilisticSearchEngine {
public:

    DLRSAdapter(std::string _name);

    //=== Game Control === //

    // Notify the search engine that the session starts
    void initSession() override;
    void finishSession(double totalReward) override;

    // Notify the search engine that a new round starts or ends
    void initRound() override;
    void finishRound(double roundReward) override;

    // Notify the search engine that a new step starts or ends
    void initStep(State const& current) override;
    void finishStep(double reward) override;
    void finishStep(std::vector<double> *successor, const double& reward) override;
	
    //=== Search ===//
    void estimateBestActions(State const& _rootState, std::vector<int>& bestActions);
	
    // === "Override" methods from superclass that are not needed for this adapter === //
    void estimateQValue(State const&, int, double&) override { assert(false); }
    void estimateQValues(State const&, std::vector<int> const&, std::vector<double>&) override { assert(false); }
    void printRoundStatistics(std::string) const override { assert(false); }
    void printStepStatistics(std::string) const override { assert(false); }

private:

    std::string in_pipe = "/home/roman/parser/PACTION";
    std::string out_pipe = "/home/roman/parser/PSTATE";

    void printState(State const& state);
    void printState(std::vector<double> *stateVariables);
    
    std::ifstream input;
    PStream pstream;
	
    int numberOfStateVariables;
};

#endif
