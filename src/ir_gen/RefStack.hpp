#pragma once

// parl
#include <backend/Environment.hpp>

namespace PArL {

class RefStack {
   public:
    RefStack& pushEnv();
    RefStack& pushEnv(size_t size);
    Environment* peekNextEnv();
    RefStack& popEnv();

    [[nodiscard]] Environment* getGlobal();
    [[nodiscard]] Environment* currentEnv();

    void init(Environment* global);
    void init(Environment* global, Environment* current);

    void reset();

   private:
    Environment* mGlobal{nullptr};
    Environment* mCurrent{nullptr};
    std::stack<size_t> mStack{{0}};
};

}  // namespace PArL
