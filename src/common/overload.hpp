#pragma once

// std
#include <typeinfo>
#include <variant>

// These are two helpers to make dealing with std::variant a bit
// easier especially when pattern matching on the type that
// std::variant holds

// ATTRIBUTION
// https://en.cppreference.com/w/cpp/utility/variant/visit

template <typename... Types>
struct overload : Types... {
    using Types::operator()...;
};

template <typename... Types>
overload(Types...) -> overload<Types...>;

// ATTRIBUTION
// https://stackoverflow.com/questions/53696720/get-currently-held-typeid-of-stdvariant-like-boostvariant-type

template <class V>
std::type_info const& var_type(V const& v)
{
    return std::visit(
        [](auto&& x) -> decltype(auto) {
            return typeid(x);
        },
        v
    );
}
