// fmt
#include <fmt/core.h>
#include <fmt/format.h>

// parl
#include <lexer/Dfsa.hpp>
#include <parl/Core.hpp>

namespace PArL {

Dfsa::Dfsa(
    size_t noOfStates,
    size_t noOfCategories,
    std::vector<std::vector<int>> const& transitionTable,
    int initialState,
    std::unordered_set<int> const& finalStates
)
    : mNoOfStates(noOfStates),
      mNoOfCategories(noOfCategories),
      mTransitionTable(transitionTable),
      mInitialState(initialState),
      mFinalStates(finalStates) {
}

int Dfsa::getInitialState() const {
    return mInitialState;
}

bool Dfsa::isValidState(int state) const {
    return 0 <= state && state < mNoOfStates;
}

bool Dfsa::isValidCategory(int category) const {
    return 0 <= category && category < mNoOfCategories;
}

bool Dfsa::isFinalState(int state) const {
    return mFinalStates.count(state) > 0;
}

int Dfsa::getTransition(
    int state,
    std::vector<int> const& categories
) const {
    core::abort_if(
        !isValidState(state),
        "state {} does not exist",
        state
    );

    for (auto const& category : categories) {
        core::abort_if(
            !isValidCategory(category),
            "category {} does not exist",
            category
        );
        int nextState = mTransitionTable[state][category];

        if (nextState != INVALID_STATE) {
            return nextState;
        }
    }

    return INVALID_STATE;
}

}  // namespace PArL
