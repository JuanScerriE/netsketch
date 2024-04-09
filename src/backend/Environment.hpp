#pragma once

// parl
#include <backend/Symbol.hpp>

// std
#include <optional>
#include <string>
#include <unordered_map>

namespace PArL {

class Environment {
   public:
    enum class Type {
        GLOBAL,
        IF,
        ELSE,
        FOR,
        WHILE,
        FUNCTION,
        BLOCK
    };

    void addSymbol(
        std::string const& identifier,
        Symbol const& Symbol
    );
    [[nodiscard]] std::optional<Symbol> findSymbol(
        std::string const& identifier
    ) const;
    Symbol& getSymbolAsRef(std::string const& identifier);

    [[nodiscard]] Environment* getEnclosing() const;
    void setEnclosing(Environment* enclosing);

    [[nodiscard]] Type getType() const;
    void setType(Type type);

    [[nodiscard]] std::optional<std::string> getName(
    ) const;
    void setName(const std::string& name);

    std::vector<std::unique_ptr<Environment>>& children();

    [[nodiscard]] bool isGlobal() const;

    [[nodiscard]] size_t getIdx() const;
    void incIdx();
    void incIdx(size_t inc);

    [[nodiscard]] size_t getSize() const;
    void setSize(size_t size);

   private:
    std::unordered_map<std::string, Symbol> mMap{};
    Type mType{Type::GLOBAL};
    std::optional<std::string> mName{};
    Environment* mEnclosing{nullptr};
    std::vector<std::unique_ptr<Environment>> mChildren{};
    size_t mSize{0};
    size_t mIdx{0};
};

}  // namespace PArL
