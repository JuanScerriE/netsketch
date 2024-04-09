#pragma once

// std
#include <stdexcept>
#include <string>

namespace PArL {

class UndefinedBuiltin : public std::runtime_error {
   public:
    explicit UndefinedBuiltin(std::string const& what);
};

}  // namespace PArL
