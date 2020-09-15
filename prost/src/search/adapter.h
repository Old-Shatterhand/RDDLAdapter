#ifndef ADAPTER_H
#define ADAPTER_H

#include <fstream>
#include <string>
#include <vector>

#include "search_engine.h"

#include "utils/pstream.h"

class DRLSAdapter : public ProbabilisticSearchEngine {
public:

	DRLSAdapter(std::string _name, std::string in_pipe, std::string out_pipe);

	//=== Game Control === //

	// Notify the search engine that the session starts
    void initSession() override;
    void finishSession(double totalReward) override;

    // Notify the search engine that a new round starts or ends
    void initRound() override;
    void finishRound(int roundReward) override;

    // Notify the search engine that a new step starts or ends
    void initStep(State const& current) override;
	void finishStep(double reward) override;
	void finishStep(vector<double> *successor, double reward) override;
	
	//=== Search ===//
	void estimateBestActions(State const& _rootState, std::vector<int>& bestActions);
	
	// === "Override" methods from superclass that are not needed for this adapter === //
	void estimateQValue(State const&, int, double&) override { assert(false); }
    void estimateQValues(State const&, std::vector<int> const&, std::vector<double>&) override { assert(false); }
    void printRoundStatistics(std::string indent) const override { assert(false); }
    void printStepStatistics(std::string indent) const override { assert(false); }

private:

	void printState(State const& state);
	void printState(vector<double> *stateVariables);

	std::ifstream input("/home/roman/parser/PACTION");
	PStream pstream("/home/roman/parser/PSTATE");
	
	int nnumberOfStateVariables;
};

#endif