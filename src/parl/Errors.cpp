// parl
#include <parl/Errors.hpp>

namespace PArL {

UndefinedBuiltin::UndefinedBuiltin(const std::string &what)
    : std::runtime_error(what) {
}

}  // namespace PArL
