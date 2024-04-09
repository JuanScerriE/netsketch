// parl
#include <analysis/EnvStack.hpp>

namespace PArL {

EnvStack::EnvStack() {
    mGlobal = std::make_unique<Environment>();

    mCurrent = mGlobal.get();
}

EnvStack& EnvStack::pushEnv() {
    auto& ref = mCurrent->children().emplace_back(
        std::make_unique<Environment>()
    );

    ref->setEnclosing(mCurrent);

    mCurrent = ref.get();

    return *this;
}

EnvStack& EnvStack::popEnv() {
    mCurrent = mCurrent->getEnclosing();

    return *this;
}

EnvStack& EnvStack::setType(Environment::Type type) {
    mCurrent->setType(type);

    return *this;
}

EnvStack& EnvStack::setName(std::string const& name) {
    mCurrent->setName(name);

    return *this;
}

Environment* EnvStack::currentEnv() {
    return mCurrent;
}

Environment* EnvStack::getGlobal() {
    return mGlobal.get();
}

std::unique_ptr<Environment> EnvStack::extractGlobal() {
    return std::move(mGlobal);
}

bool EnvStack::isCurrentEnvGlobal() const {
    return mCurrent->isGlobal();
}

}  // namespace PArL
