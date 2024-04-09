#pragma once

// fmt
#include <fmt/core.h>

// std
#include <sstream>
#include <stack>
#include <variant>

namespace PArL::core {

#ifdef NDEBUG
template <typename... T>
inline void abort(fmt::format_string<T...>, T &&...) {
}

template <typename... T>
inline void
abort_if(bool, fmt::format_string<T...>, T &&...) {
}
#else
template <typename... T>
inline void
abort(fmt::format_string<T...> fmt, T &&...args) {
    fmt::println(stderr, fmt, args...);

    std::abort();
}

template <typename... T>
inline void abort_if(
    bool cond,
    fmt::format_string<T...> fmt,
    T &&...args
) {
    if (cond) {
        abort(fmt, args...);
    }
}
#endif

class Position {
   public:
    Position(int row, int col)
        : mRow(row), mCol(col) {
    }

    [[nodiscard]] int row() const {
        return mRow;
    }

    [[nodiscard]] int col() const {
        return mCol;
    }

   private:
    int mRow;
    int mCol;
};

class Color {
   public:
    Color(uint8_t r, uint8_t g, uint8_t b)
        : mR(r), mG(g), mB(b) {
    }

    [[nodiscard]] uint8_t r() const {
        return mR;
    }

    [[nodiscard]] uint8_t g() const {
        return mG;
    }

    [[nodiscard]] uint8_t b() const {
        return mB;
    }

   private:
    uint8_t mR;
    uint8_t mG;
    uint8_t mB;
};

enum class Operation {
    ADD,
    AND,
    DIV,
    EQ,
    GE,
    GT,
    LE,
    LT,
    MUL,
    NEQ,
    NOT,
    OR,
    SUB,
};

inline std::string operationToString(Operation op) {
    switch (op) {
        case Operation::ADD:
            return "+";
        case Operation::AND:
            return "and";
        case Operation::DIV:
            return "/";
        case Operation::EQ:
            return "==";
        case Operation::GE:
            return ">=";
        case Operation::GT:
            return ">";
        case Operation::LE:
            return "<=";
        case Operation::LT:
            return "<";
        case Operation::MUL:
            return "*";
        case Operation::NEQ:
            return "!=";
        case Operation::NOT:
            return "not";
        case Operation::OR:
            return "or";
        case Operation::SUB:
            return "sub";
    };
}

// credit: Jonathan,
// https://www.foonathan.net/2022/05/recursive-variant-box/

template <typename T>
class box {
    // Wrapper over unique_ptr.
    std::unique_ptr<T> _impl{nullptr};

   public:
    // Automatic construction from a `T`, not a `T*`.
    explicit box(T &&obj)
        : _impl(new T(std::move(obj))) {
    }
    explicit box(const T &obj)
        : _impl(new T(obj)) {
    }

    // Copy constructor copies `T`.
    box(const box &other)
        : box(*other._impl) {
    }
    box &operator=(const box &other) {
        *_impl = *other._impl;
        return *this;
    }

    // unique_ptr destroys `T` for us.
    ~box() = default;

    // Access propagates constness.
    T &operator*() {
        return *_impl;
    }
    const T &operator*() const {
        return *_impl;
    }

    T *operator->() {
        return _impl.get();
    }
    const T *operator->() const {
        return _impl.get();
    }

    T *get() {
        return _impl.get();
    }
    [[nodiscard]] const T *get() const {
        return _impl.get();
    }
};

struct Primitive;

enum class Base {
    BOOL,
    COLOR,
    FLOAT,
    INT,
};

struct Array {
    size_t size;
    box<Primitive> type;
};

struct Primitive {
    template <typename T>
    [[nodiscard]] bool is() const {
        return std::holds_alternative<T>(data);
    }

    template <typename T>
    [[nodiscard]] const T &as() const {
        return std::get<T>(data);
    }

    explicit Primitive() = default;

    template <typename T>
    explicit Primitive(T const &data)
        : data(data) {
    }

    template <typename T>
    Primitive &operator=(T const &data_) {
        data = data_;

        return *this;
    }

    bool operator==(Primitive const &other) {
        const Primitive *lPtr = this;
        const Primitive *rPtr = &other;

        while (lPtr != nullptr && rPtr != nullptr) {
            if (lPtr->is<Base>() && rPtr->is<Base>()) {
                return lPtr->as<Base>() == rPtr->as<Base>();
            }

            if (lPtr->is<Array>() && rPtr->is<Array>()) {
                size_t lSize = lPtr->as<Array>().size;
                size_t rSize = rPtr->as<Array>().size;

                lPtr = lPtr->as<Array>().type.get();
                rPtr = rPtr->as<Array>().type.get();

                if (lSize != rSize)
                    return false;

                continue;
            }

            return false;
        }

        return false;
    }

    bool operator!=(Primitive const &other) {
        return !operator==(other);
    }

    std::variant<std::monostate, Base, Array> data{};
};

inline std::string baseToString(Base type) {
    switch (type) {
        case Base::BOOL:
            return "bool";
        case Base::COLOR:
            return "color";
        case Base::FLOAT:
            return "float";
        case Base::INT:
            return "int";
    };
}

inline std::string primitiveToString(Primitive *primitive) {
    const Primitive *current = primitive;

    std::stack<std::string> string{};

    while (current != nullptr) {
        if (current->is<Base>()) {
            string.push(baseToString(current->as<Base>()));

            current = nullptr;

            continue;
        }

        if (current->is<Array>()) {
            string.push(fmt::format(
                "[{}]",
                current->as<Array>().size
            ));

            current = current->as<Array>().type.get();

            continue;
        }
    }

    std::ostringstream buffer{};

    std::string result{};

    while (!string.empty()) {
        buffer << string.top();

        string.pop();
    }

    return buffer.str();
}

enum class Builtin {
    CLEAR,
    DELAY,
    HEIGHT,
    PRINT,
    RANDOM_INT,
    READ,
    WIDTH,
    WRITE,
    WRITE_BOX,
};

}  // namespace PArL::core
