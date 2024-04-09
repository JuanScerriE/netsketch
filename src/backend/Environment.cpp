// parl
#include <backend/Environment.hpp>

// std
#include <optional>

namespace PArL {

Environment* Environment::getEnclosing() const {
    return mEnclosing;
}

void Environment::setEnclosing(Environment* enclosing) {
    mEnclosing = enclosing;
}

Environment::Type Environment::getType() const {
    return mType;
}

void Environment::setType(Type type) {
    mType = type;
}

std::optional<std::string> Environment::getName() const {
    core::abort_if(
        mType == Type::FUNCTION && !mName.has_value(),
        "function scope must have a name"
    );

    return mName;
}

void Environment::setName(const std::string& name) {
    mName = name;
}

std::vector<std::unique_ptr<Environment>>&
Environment::children() {
    return mChildren;
};

bool Environment::isGlobal() const {
    return mEnclosing == nullptr;
}

void Environment::addSymbol(
    std::string const& identifier,
    Symbol const& symbol
) {
    core::abort_if(
        mMap.count(identifier) > 0,
        "{} is already a registered identifier",
        identifier
    );

    mMap.insert({identifier, symbol});
}

std::optional<Symbol> Environment::findSymbol(
    std::string const& identifier
) const {
    if (mMap.count(identifier) > 0) {
        return mMap.at(identifier);
    }

    return {};
}

Symbol& Environment::getSymbolAsRef(
    std::string const& identifier
) {
    core::abort_if(
        mMap.count(identifier) <= 0,
        "unchecked access to map"
    );

    return mMap.at(identifier);
}

size_t Environment::getIdx() const {
    return mIdx;
}

void Environment::incIdx() {
    mIdx++;

    core::abort_if(
        mIdx > mSize,
        "exceeded frame size"
    );
}

void Environment::incIdx(size_t inc) {
    mIdx += inc;

    core::abort_if(
        mIdx > mSize,
        "exceeded frame size"
    );
}

size_t Environment::getSize() const {
    return mSize;
}

void Environment::setSize(size_t size) {
    mSize = size;
}

}  // namespace PArL
