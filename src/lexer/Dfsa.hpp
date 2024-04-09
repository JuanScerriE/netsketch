#pragma once

// std
#include <unordered_set>
#include <vector>

// macro definitions
#define INVALID_STATE (-1)

namespace PArL {

class Runner;

class Dfsa {
   public:
    Dfsa(
        size_t noOfStates,
        size_t noOfCategories,
        std::vector<std::vector<int>> const&
            transitionTable,
        int initialState,
        std::unordered_set<int> const& finalStates
    );

    [[nodiscard]] int getInitialState() const;

    [[nodiscard]] bool isValidState(int state) const;
    [[nodiscard]] bool isValidCategory(int category) const;

    [[nodiscard]] bool isFinalState(int state) const;

    [[nodiscard]] int getTransition(
        int state,
        std::vector<int> const& categories
    ) const;

   private:
    const size_t mNoOfStates;      // Q
    const size_t mNoOfCategories;  // Sigma
    const std::vector<std::vector<int>>
        mTransitionTable;                        // delta
    const int mInitialState;                     // q_0
    const std::unordered_set<int> mFinalStates;  // F
};

}  // namespace PArL
