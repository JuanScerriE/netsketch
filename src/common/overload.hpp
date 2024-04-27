#pragma once

// std
#include <variant>
#include <typeinfo>

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
