#include "search_engine.h"

#include "prost_planner.h"

#include "utils/math_utils.h"
#include "utils/string_utils.h"
#include "utils/system_utils.h"

using namespace std;

/******************************************************************
                    Static variable definitions
******************************************************************/

int State::numberOfDeterministicStateFluents = 0;
int State::numberOfProbabilisticStateFluents = 0;

int State::numberOfStateFluentHashKeys = 0;
bool State::stateHashingPossible = true;

vector<vector<long>> State::stateHashKeysOfDeterministicStateFluents;
vector<vector<long>> State::stateHashKeysOfProbabilisticStateFluents;

vector<vector<pair<int, long>>>
    State::stateFluentHashKeysOfDeterministicStateFluents;
vector<vector<pair<int, long>>>
    State::stateFluentHashKeysOfProbabilisticStateFluents;

int KleeneState::stateSize = 0;
int KleeneState::numberOfStateFluentHashKeys = 0;
bool KleeneState::stateHashingPossible = true;
vector<long> KleeneState::hashKeyBases;
vector<vector<pair<int, long>>> KleeneState::indexToStateFluentHashKeyMap;

string SearchEngine::taskName;
vector<State> SearchEngine::trainingSet;

vector<ActionState> SearchEngine::actionStates;

vector<ActionFluent*> SearchEngine::actionFluents;
vector<StateFluent*> SearchEngine::stateFluents;

vector<Evaluatable*> SearchEngine::allCPFs;
vector<DeterministicCPF*> SearchEngine::deterministicCPFs;
vector<ProbabilisticCPF*> SearchEngine::probabilisticCPFs;

vector<DeterministicCPF*> SearchEngine::determinizedCPFs;

RewardFunction* SearchEngine::rewardCPF = nullptr;
vector<DeterministicEvaluatable*> SearchEngine::actionPreconditions;

bool SearchEngine::taskIsDeterministic = true;
State SearchEngine::initialState;
int SearchEngine::horizon = numeric_limits<int>::max();
double SearchEngine::discountFactor = 1.0;
int SearchEngine::numberOfActions = -1;
SearchEngine::FinalRewardCalculationMethod
    SearchEngine::finalRewardCalculationMethod = NOOP;
vector<int> SearchEngine::candidatesForOptimalFinalAction;

bool SearchEngine::cacheApplicableActions = true;
bool SearchEngine::rewardLockDetected = true;
int SearchEngine::goalTestActionIndex = -1;
bdd SearchEngine::cachedDeadEnds = bddfalse;
bdd SearchEngine::cachedGoals = bddfalse;

bool ProbabilisticSearchEngine::hasUnreasonableActions = true;
bool DeterministicSearchEngine::hasUnreasonableActions = true;

SearchEngine::ActionHashMap ProbabilisticSearchEngine::applicableActionsCache(
    520241);
SearchEngine::ActionHashMap DeterministicSearchEngine::applicableActionsCache(
    520241);

SearchEngine::StateValueHashMap ProbabilisticSearchEngine::stateValueCache(
    62233);
SearchEngine::StateValueHashMap DeterministicSearchEngine::stateValueCache(
    520241);

/******************************************************************
                     Search Engine Creation
******************************************************************/

void SearchEngine::printConfig(string indent) const {
    Logger::logLine(indent + "Configuration of " + name + ":",
                    Verbosity::VERBOSE);
    indent += "  ";
    if (cachingEnabled) {
        Logger::logLine(indent + "Caching: enabled", Verbosity::VERBOSE);
    } else {
        Logger::logLine(indent + "Caching: disabled", Verbosity::VERBOSE);
    }
    if (useRewardLockDetection) {
        Logger::logLine(indent + "Reward lock detection: enabled",
                        Verbosity::VERBOSE);
    } else {
        Logger::logLine(indent + "Reward lock detection: disabled",
                        Verbosity::VERBOSE);
    }
    if (cacheRewardLocks) {
        Logger::logLine(indent + "Reward lock caching: enabled",
                        Verbosity::VERBOSE);
    } else {
        Logger::logLine(indent + "Reward lock caching: enabled",
                        Verbosity::VERBOSE);
    }
}

void ProbabilisticSearchEngine::printStateValueCacheUsage(
        std::string indent, Verbosity verbosity) const {
    long entriesProbStateValue =
            ProbabilisticSearchEngine::stateValueCache.size();
    long bucketsProbStateValue =
            ProbabilisticSearchEngine::stateValueCache.bucket_count();
    Logger::logLine(
            indent + "Entries in probabilistic state value cache: " +
            std::to_string(entriesProbStateValue), verbosity);
    Logger::logLine(
            indent + "Buckets in probabilistic state value cache: " +
            std::to_string(bucketsProbStateValue), verbosity);
}

void ProbabilisticSearchEngine::printApplicableActionCacheUsage(
        std::string indent, Verbosity verbosity) const {
    long entriesProbApplActions =
            ProbabilisticSearchEngine::applicableActionsCache.size();
    long bucketsProbApplActions =
            ProbabilisticSearchEngine::applicableActionsCache.bucket_count();
    Logger::logLine(
            indent + "Entries in probabilistic applicable actions cache: " +
            std::to_string(entriesProbApplActions), verbosity);
    Logger::logLine(
            indent + "Buckets in probabilistic applicable actions cache: " +
            std::to_string(bucketsProbApplActions), verbosity);
}

void DeterministicSearchEngine::printStateValueCacheUsage(
        std::string indent, Verbosity verbosity) const {
    long entriesDetStateValue =
            DeterministicSearchEngine::stateValueCache.size();
    long bucketsDetStateValue =
            DeterministicSearchEngine::stateValueCache.bucket_count();
    Logger::logLine(
            indent + "Entries in deterministic state value cache: " +
            to_string(entriesDetStateValue), verbosity);
    Logger::logLine(
            indent + "Buckets in deterministic state value cache: " +
            to_string(bucketsDetStateValue), verbosity);
}

void DeterministicSearchEngine::printApplicableActionCacheUsage(
        std::string indent, Verbosity verbosity) const {
    long entriesDetApplActions =
            DeterministicSearchEngine::applicableActionsCache.size();
    long bucketsDetApplActions =
            DeterministicSearchEngine::applicableActionsCache.bucket_count();
    Logger::logLine(
            indent + "Entries in deterministic applicable actions cache: " +
            to_string(entriesDetApplActions), verbosity);
    Logger::logLine(
            indent + "Buckets in deterministic applicable actions cache: " +
            to_string(bucketsDetApplActions), verbosity);
}

/******************************************************************
                       Main Search Functions
******************************************************************/

void SearchEngine::estimateBestActions(State const& _rootState,
                                       std::vector<int>& bestActions) {
    vector<double> qValues(numberOfActions);
    vector<int> actionsToExpand = getApplicableActions(_rootState);

    estimateQValues(_rootState, actionsToExpand, qValues);
    double stateValue = -numeric_limits<double>::max();
    for (size_t index = 0; index < qValues.size(); ++index) {
        if (actionsToExpand[index] == index) {
            if (MathUtils::doubleIsGreater(qValues[index], stateValue)) {
                stateValue = qValues[index];
                bestActions.clear();
                bestActions.push_back(index);
            } else if (MathUtils::doubleIsEqual(qValues[index], stateValue)) {
                bestActions.push_back(index);
            }
        }
    }
}

void SearchEngine::estimateStateValue(State const& _rootState,
                                      double& stateValue) {
    vector<double> qValues(numberOfActions);
    vector<int> actionsToExpand = getApplicableActions(_rootState);

    estimateQValues(_rootState, actionsToExpand, qValues);
    stateValue = -numeric_limits<double>::max();
    for (size_t index = 0; index < qValues.size(); ++index) {
        if (actionsToExpand[index] == index) {
            stateValue = std::max(stateValue, qValues[index]);
        }
    }
}

/******************************************************************
            Reward Lock Detection (including BDD Stuff)
******************************************************************/

// Currently, we only consider goals and dead ends (i.e., reward locks with min
// or max reward). This makes sense on the IPC 2011 domains, yet we might want
// to change it in the future so keep an eye on it. Nevertheless, isARewardLock
// is sound as is (and incomplete independently from this decision).
bool ProbabilisticSearchEngine::isARewardLock(State const& current) const {
    if (!useRewardLockDetection) {
        return false;
    }

    assert(goalTestActionIndex >= 0);

    // Calculate the reference reward
    double reward = 0.0;
    calcReward(current, goalTestActionIndex, reward);

    if (MathUtils::doubleIsEqual(rewardCPF->getMinVal(), reward)) {
        // Check if current is known to be a dead end
        if (cacheRewardLocks && BDDIncludes(cachedDeadEnds, current)) {
            return true;
        }

        // Convert to Kleene state
        KleeneState currentInKleene(current);
        KleeneState::calcStateHashKey(currentInKleene);
        KleeneState::calcStateFluentHashKeys(currentInKleene);

        // Check reward lock on Kleene state
        return checkDeadEnd(currentInKleene);
    } else if (MathUtils::doubleIsEqual(rewardCPF->getMaxVal(), reward)) {
        // Check if current is known to be a goal
        if (cacheRewardLocks && BDDIncludes(cachedGoals, current)) {
            return true;
        }

        // Convert to Kleene state
        KleeneState currentInKleene(current);
        KleeneState::calcStateHashKey(currentInKleene);
        KleeneState::calcStateFluentHashKeys(currentInKleene);

        // Logger::logLine("Checking state: ", Verbosity::DEBUG);
        // Logger::logLine(currentInKleene.toString(), Verbosity::DEBUG);

        return checkGoal(currentInKleene);
    }
    return false;
}

bool ProbabilisticSearchEngine::checkDeadEnd(KleeneState const& state) const {
    // TODO: We do currently not care about action applicability. Nevertheless,
    // the results remain sound, as we only check too many actions (it might be
    // the case that we think some state is not a dead end even though it is.
    // This is because the action that would make us leave the dead end is
    // actually inapplicable).

    // Apply noop
    KleeneState mergedSuccs;
    set<double> reward;
    calcKleeneSuccessor(state, 0, mergedSuccs);
    calcKleeneReward(state, 0, reward);

    // If reward is not minimal with certainty this is not a dead end
    if ((reward.size() != 1) ||
        !MathUtils::doubleIsEqual(*reward.begin(), rewardCPF->getMinVal())) {
        return false;
    }

    for (size_t index = 1; index < numberOfActions; ++index) {
        reward.clear();
        // Apply action index
        KleeneState succ;
        calcKleeneSuccessor(state, index, succ);
        calcKleeneReward(state, index, reward);

        // If reward is not minimal this is not a dead end
        if ((reward.size() != 1) ||
            !MathUtils::doubleIsEqual(*reward.begin(),
                                      rewardCPF->getMinVal())) {
            return false;
        }

        // Merge with previously computed successors
        mergedSuccs |= succ;
    }

    // Calculate hash keys
    KleeneState::calcStateHashKey(mergedSuccs);
    KleeneState::calcStateFluentHashKeys(mergedSuccs);

    // Check if nothing changed, otherwise continue dead end check
    if ((mergedSuccs == state) || checkDeadEnd(mergedSuccs)) {
        if (cacheRewardLocks) {
            cachedDeadEnds |= stateToBDD(state);
        }
        return true;
    }
    return false;
}

// We underapproximate the set of goals, as we only consider those where
// applying goalTestActionIndex makes us stay in the reward lock.
bool ProbabilisticSearchEngine::checkGoal(KleeneState const& state) const {
    // Apply action goalTestActionIndex
    KleeneState succ;
    set<double> reward;
    calcKleeneSuccessor(state, goalTestActionIndex, succ);
    calcKleeneReward(state, goalTestActionIndex, reward);

    // If reward is not maximal with certainty this is not a goal
    if ((reward.size() > 1) ||
        !MathUtils::doubleIsEqual(rewardCPF->getMaxVal(), *reward.begin())) {
        return false;
    }

    // Add parent to successor
    succ |= state;

    // Calculate hash keys
    KleeneState::calcStateHashKey(succ);
    KleeneState::calcStateFluentHashKeys(succ);

    // Check if nothing changed, otherwise continue goal check
    if ((succ == state) || checkGoal(succ)) {
        if (cacheRewardLocks) {
            cachedGoals |= stateToBDD(state);
        }
        return true;
    }
    return false;
}

inline bdd ProbabilisticSearchEngine::stateToBDD(
    KleeneState const& state) const {
    bdd res = bddtrue;
    for (size_t i = 0; i < KleeneState::stateSize; ++i) {
        bdd tmp = bddfalse;
        for (set<double>::iterator it = state[i].begin(); it != state[i].end();
             ++it) {
            tmp |= fdd_ithvar(i, *it);
        }
        res &= tmp;
    }
    return res;
}

inline bdd ProbabilisticSearchEngine::stateToBDD(State const& state) const {
    bdd res = bddtrue;
    for (size_t i = 0; i < State::numberOfDeterministicStateFluents; ++i) {
        res &= fdd_ithvar(i, state.deterministicStateFluent(i));
    }
    for (size_t i = 0; i < State::numberOfProbabilisticStateFluents; ++i) {
        res &= fdd_ithvar(State::numberOfDeterministicStateFluents + i,
                          state.probabilisticStateFluent(i));
    }
    return res;
}

inline bool ProbabilisticSearchEngine::BDDIncludes(
    bdd BDD, KleeneState const& state) const {
    return (BDD & stateToBDD(state)) != bddfalse;
}

/******************************************************************
               Calculation of Final Reward and Action
******************************************************************/

void SearchEngine::calcOptimalFinalRewardWithFirstApplicableAction(
    State const& current, double& reward) const {
    // Get applicable actions
    vector<int> applicableActions = getApplicableActions(current);

    // If no action fluent occurs in the reward, the reward is the same for all
    // applicable actions, so we only need to find an applicable action
    for (size_t index = 0; index < applicableActions.size(); ++index) {
        if (applicableActions[index] == index) {
            return calcReward(current, index, reward);
        }
    }
    assert(false);
}

void SearchEngine::calcOptimalFinalRewardAsBestOfCandidateSet(
    State const& current, double& reward) const {
    // Get applicable actions
    vector<int> applicableActions = getApplicableActions(current);

    reward = -numeric_limits<double>::max();
    double tmpReward = 0.0;

    for (int index : candidatesForOptimalFinalAction) {
        if (applicableActions[index] == index) {
            calcReward(current, index, tmpReward);
            reward = std::max(reward, tmpReward);
        }
    }
}

int SearchEngine::getOptimalFinalActionIndex(State const& current) const {
    if (finalRewardCalculationMethod == NOOP) {
        return 0;
    }

    // Get applicable actions
    vector<int> applicableActions = getApplicableActions(current);

    if (finalRewardCalculationMethod == FIRST_APPLICABLE) {
        // If no action fluent occurs in the reward, all rewards are the
        // same and we only need to find an applicable action
        for (size_t index = 0; index < applicableActions.size(); ++index) {
            if (applicableActions[index] == index) {
                return index;
            }
        }
        assert(false);
        return -1;
    }

    // Otherwise we compute which action in the candidate set yields the highest
    // reward
    assert(finalRewardCalculationMethod == BEST_OF_CANDIDATE_SET);
    double reward = -numeric_limits<double>::max();
    double tmpReward = 0.0;
    int optimalFinalActionIndex = -1;

    for (int index : candidatesForOptimalFinalAction) {
        if (applicableActions[index] == index) {
            calcReward(current, index, tmpReward);

            if (MathUtils::doubleIsGreater(tmpReward, reward)) {
                reward = tmpReward;
                optimalFinalActionIndex = index;
            }
        }
    }

    return optimalFinalActionIndex;
}

/******************************************************************
                   Statistics and Prints
******************************************************************/

void SearchEngine::printTask() {
    std::cout.unsetf(std::ios::floatfield);
    std::cout.precision(std::numeric_limits<double>::digits10);

    Logger::logLine("----------------Actions---------------");
    Logger::logLine();
    Logger::logLine("Action fluents:");
    for (ActionFluent const* af : actionFluents) {
        Logger::logLine(af->name);
    }
    Logger::logSmallSeparator();
    Logger::logLine();
    Logger::logLine("Legal Action Combinations:");
    for (ActionState const& state : actionStates) {
        Logger::logLine(state.toString());
        Logger::logSmallSeparator();
    }

    Logger::logLine();
    Logger::logLine("-----------------CPFs-----------------");
    Logger::logLine();

    int numDetStateFluents = State::numberOfDeterministicStateFluents;
    for (size_t index = 0; index < numDetStateFluents; ++index) {
        printDeterministicCPFInDetail(index);
        Logger::logSmallSeparator();
    }

    int numProbStateFluents = State::numberOfProbabilisticStateFluents;
    for (size_t index = 0; index < numProbStateFluents; ++index) {
        printProbabilisticCPFInDetail(index);
        Logger::logSmallSeparator();
    }
    Logger::logLine();

    Logger::logLine("Reward CPF:");
    printRewardCPFInDetail();

    Logger::logLine();
    Logger::logLine("------State Fluent Hash Key Map-------");
    Logger::logLine();

    for (size_t index = 0; index < numDetStateFluents; ++index) {
        Logger::log("a change of deterministic state fluent " +
                    to_string(index) + " influences variables ");
        vector<pair<int, long>> const& influencedVars =
            State::stateFluentHashKeysOfDeterministicStateFluents[index];

        for (pair<int, long> const& influencedVar : influencedVars) {
            Logger::log(to_string(influencedVar.first) + " (" +
                        to_string(influencedVar.second) + ") ");
        }
        Logger::logLine();
    }
    Logger::logLine();

    for (size_t index = 0; index < numProbStateFluents; ++index) {
        Logger::log("a change of probabilistic state fluent " +
                    to_string(index) + " influences variables ");
        vector<pair<int, long>> const& influencedVars =
            State::stateFluentHashKeysOfProbabilisticStateFluents[index];

        for (pair<int, long> const& influencedVar : influencedVars) {
            Logger::log(to_string(influencedVar.first) + " (" +
                        to_string(influencedVar.second) + ") ");
        }
        Logger::logLine();
    }
    Logger::logLine();

    for (size_t index = 0; index < KleeneState::stateSize; ++index) {
        Logger::log("a change of variable " + to_string(index) +
                    " influences variables in Kleene states ");
        vector<pair<int, long>> const& influencedVars =
            KleeneState::indexToStateFluentHashKeyMap[index];

        for (pair<int, long> const& influencedVar : influencedVars) {
            Logger::log(to_string(influencedVar.first) + " (" +
                        to_string(influencedVar.second) + ") ");
        }
        Logger::logLine();
    }
    Logger::logLine();

    Logger::logLine();
    Logger::logLine("---------Action Preconditions---------");
    Logger::logLine();

    for (size_t index = 0; index < actionPreconditions.size(); ++index) {
        printActionPreconditionInDetail(index);
        Logger::logSmallSeparator();
    }

    Logger::logLine();
    Logger::logLine("----------Initial State---------------");
    Logger::logLine();
    Logger::logLine(initialState.toString());

    if (State::stateHashingPossible) {
        Logger::logLine("Hashing of States is possible.");
    } else {
        Logger::logLine("Hashing of States is not possible.");
    }
    if (KleeneState::stateHashingPossible) {
        Logger::logLine("Hashing of KleeneStates is possible.");
    } else {
        Logger::logLine("Hashing of KleeneStates is not possible.");
    }


    if (rewardLockDetected) {
        if (goalTestActionIndex >= 0) {
            Logger::logLine(
                "A goal and a dead end were found in the training phase.");
        } else {
            Logger::logLine(
                "A dead end but no goal was found in the training phase.");
        }
    } else {
        Logger::logLine("No reward locks detected in the training phase.");
    }

    if (ProbabilisticSearchEngine::hasUnreasonableActions &&
        DeterministicSearchEngine::hasUnreasonableActions) {
        Logger::logLine("This task contains unreasonable actions.");
    } else if (ProbabilisticSearchEngine::hasUnreasonableActions) {
        assert(false);
    } else if (DeterministicSearchEngine::hasUnreasonableActions) {
        Logger::logLine(
            "Only the determinization contains unreasonable actions.");
    } else {
        Logger::logLine("This task does not contain unreasonable actions.");
    }

    Logger::log("The final reward is determined ");
    switch (finalRewardCalculationMethod) {
    case NOOP:
        Logger::logLine("by applying NOOP.");
        break;
    case FIRST_APPLICABLE:
        Logger::logLine("by applying the first applicable action.");
        break;
    case BEST_OF_CANDIDATE_SET:
        Logger::logLine("as the maximum over the candidate set: ");
        for (int candidate : candidatesForOptimalFinalAction) {
            Logger::logLine("  " + actionStates[candidate].toCompactString());
        }
        break;
    }
    Logger::logLine();
}

void SearchEngine::printDeterministicCPFInDetail(int index) {
    printEvaluatableInDetail(deterministicCPFs[index]);
    Logger::logLine();

    Logger::log("  Domain: ");
    for (std::string const& val : deterministicCPFs[index]->head->values) {
        Logger::log(val + " ");
    }
    Logger::logLine();

    if (State::stateHashingPossible) {
        Logger::log("  HashKeyBase: ");
        std::vector<long> const& stateHashKeys =
            State::stateHashKeysOfDeterministicStateFluents[index];
        for (size_t i = 0; i < stateHashKeys.size(); ++i) {
            Logger::log(to_string(i) + ": " + to_string(stateHashKeys[i]));
            if (i != stateHashKeys.size() - 1) {
                Logger::log(", ");
            } else {
                Logger::logLine();
            }
        }
    }

    if (KleeneState::stateHashingPossible) {
        Logger::logLine("  KleeneHashKeyBase: " +
                    to_string(KleeneState::hashKeyBases[index]));
    }
}

void SearchEngine::printProbabilisticCPFInDetail(int index) {
    printEvaluatableInDetail(probabilisticCPFs[index]);
    Logger::logLine();

    Logger::logLine("  Determinized formula: ");

    // TODO: Replace print() method with toString()
    stringstream ss;
    determinizedCPFs[index]->formula->print(ss);
    ss << endl;
    Logger::logLine(ss.str());

    Logger::log("  Domain: ");
    for (std::string const& val : probabilisticCPFs[index]->head->values) {
        Logger::log(val + " ");
    }
    Logger::logLine();

    if (State::stateHashingPossible) {
        Logger::log("  HashKeyBase: ");
        std::vector<long> const& stateHashKeys =
            State::stateHashKeysOfProbabilisticStateFluents[index];
        for (size_t i = 0; i < stateHashKeys.size(); ++i) {
            Logger::log(to_string(i) + ": " + to_string(stateHashKeys[i]));
            if (i != stateHashKeys.size() - 1) {
                Logger::log(", ");
            } else {
                Logger::logLine();
            }
        }
    }

    if (KleeneState::stateHashingPossible) {
        index += State::numberOfDeterministicStateFluents;
        Logger::logLine("  KleeneHashKeyBase: " +
                    to_string(KleeneState::hashKeyBases[index]));
    }
}

void SearchEngine::printRewardCPFInDetail() {
    printEvaluatableInDetail(rewardCPF);

    Logger::logLine("Minimal reward: " + to_string(rewardCPF->getMinVal()));
    Logger::logLine("Maximal reward: " + to_string(rewardCPF->getMaxVal()));
    Logger::logLine("Is action independent: " + to_string(rewardCPF->isActionIndependent()));
    Logger::logLine();
}

void SearchEngine::printActionPreconditionInDetail(int index) {
    printEvaluatableInDetail(actionPreconditions[index]);
    Logger::logLine();
}

void SearchEngine::printEvaluatableInDetail(Evaluatable* eval) {
    Logger::logLine(eval->name);
    Logger::log("  HashIndex: " + to_string(eval->hashIndex) + ",");

    if (!eval->isProbabilistic()) {
        Logger::log(" deterministic,");
    } else {
        Logger::log(" probabilistic,");
    }

    switch (eval->cachingType) {
    case Evaluatable::NONE:
        Logger::log(" no caching,");
        break;
    case Evaluatable::MAP:
    case Evaluatable::DISABLED_MAP:
        Logger::log(" caching in maps,");
        break;
    case Evaluatable::VECTOR:
        Logger::log(" caching in vectors,");
        break;
    }

    switch (eval->kleeneCachingType) {
    case Evaluatable::NONE:
        Logger::log(" no Kleene caching.");
        break;
    case Evaluatable::MAP:
    case Evaluatable::DISABLED_MAP:
        Logger::log(" Kleene caching in maps.");
        break;
    case Evaluatable::VECTOR:
        Logger::log(" Kleene caching in vectors of size " +
                    to_string(eval->kleeneEvaluationCacheVector.size()) + ".");
        break;
    }
    Logger::logLine();
    Logger::logLine();

    if (!eval->actionHashKeyMap.empty()) {
        Logger::logLine("  Action Hash Key Map:");
        for (size_t i = 0; i < eval->actionHashKeyMap.size(); ++i) {
            if (eval->actionHashKeyMap[i] != 0) {
                Logger::logLine("    " + actionStates[i].toCompactString() +
                                " : " + to_string(eval->actionHashKeyMap[i]));
            }
        }
    } else {
        Logger::logLine("  Has no positive dependencies on actions.");
    }

    // TODO: Replace print() method with toString()
    stringstream ss;
    ss << "  Formula: " << endl;
    eval->formula->print(ss);
    Logger::logLine(ss.str());
}
