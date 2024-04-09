#pragma once

// parl
#include <backend/Environment.hpp>
#include <backend/Symbol.hpp>

namespace PArL {

class EnvStack {
   public:
    EnvStack();

    EnvStack& pushEnv();
    EnvStack& popEnv();

    EnvStack& setType(Environment::Type type);
    EnvStack& setName(std::string const& name);

    Environment* currentEnv();

    [[nodiscard]] Environment* getGlobal();
    [[nodiscard]] std::unique_ptr<Environment>
    extractGlobal();

    [[nodiscard]] bool isCurrentEnvGlobal() const;

   private:
    std::unique_ptr<Environment> mGlobal{};
    Environment* mCurrent{nullptr};
};

}  // namespace PArL
