#pragma once

// std
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

// parl
#include <parl/Core.hpp>
#include <parl/Errors.hpp>

namespace PArL {

static const std::unordered_map<std::string, core::Builtin>
    builtins{
        {"__width", core::Builtin::WIDTH},
        {"__height", core::Builtin::HEIGHT},
        {"__read", core::Builtin::READ},
        {"__random_int", core::Builtin::RANDOM_INT},
        {"__print", core::Builtin::PRINT},
        {"__delay", core::Builtin::DELAY},
        {"__write", core::Builtin::WRITE},
        {"__write_box", core::Builtin::WRITE_BOX},
        {"__clear", core::Builtin::CLEAR},
    };

struct Value {
    template <typename T>
    [[nodiscard]] bool is() const {
        return std::holds_alternative<T>(data);
    }

    template <typename T>
    [[nodiscard]] T as() const {
        core::abort_if(
            !std::holds_alternative<T>(data),
            "improper as<{}>() cast",
            typeid(T).name()
        );

        return std::get<T>(data);
    }

    std::variant<
        float,
        int,
        core::Color,
        bool,
        core::Builtin,
        std::string>
        data;
};

template <typename T>
static Value create(const std::string&) {
    core::abort("unimplemented");
}

template <>
Value create<bool>(const std::string& lexeme) {
    return {lexeme == "true"};
}

template <>
Value create<core::Color>(const std::string& lexeme) {
    return {core::Color{
        static_cast<uint8_t>(
            stoi(lexeme.substr(1, 2), nullptr, 16)
        ),
        static_cast<uint8_t>(
            stoi(lexeme.substr(3, 2), nullptr, 16)
        ),
        static_cast<uint8_t>(
            stoi(lexeme.substr(5, 2), nullptr, 16)
        )
    }};
}

template <>
Value create<float>(const std::string& lexeme) {
    return {std::stof(lexeme)};
}

template <>
Value create<int>(const std::string& lexeme) {
    return {std::stoi(lexeme)};
}

template <>
Value create<core::Builtin>(const std::string& lexeme) {
    if (builtins.count(lexeme) <= 0) {
        throw UndefinedBuiltin(lexeme + " is an undefined builtin");
    }

    return {builtins.at(lexeme)};
}

template <>
Value create<std::string>(const std::string& lexeme) {
    return {lexeme};
}

}  // namespace PArL
