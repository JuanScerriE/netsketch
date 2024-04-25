#pragma once

template <typename... Types>
struct overload : Types... {
    using Types::operator()...;
};

template <typename... Types>
overload(Types...) -> overload<Types...>;
