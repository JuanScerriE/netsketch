#pragma once

// common
#include "bytes.hpp"

// cereal
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

template <class T>
ByteString serialize(T object)
{
    std::stringstream ss {};

    {
        cereal::PortableBinaryOutputArchive ar { ss };

        ar(object);
    }

    return ss.str();
}

enum class DeserializeErrorCode {
    OK,
    EXCEPTION
};

struct DeserializeError {
    explicit DeserializeError(const char* what)
        : m_code(DeserializeErrorCode::EXCEPTION), m_what(what)
    {
    }

    explicit DeserializeError(std::string what)
        : m_code(DeserializeErrorCode::EXCEPTION), m_what(std::move(what))
    {
    }

    explicit DeserializeError(DeserializeErrorCode code)
        : m_code(code)
    {
    }

    operator DeserializeErrorCode() const
    {
        return m_code;
    }

    [[nodiscard]] std::string what() const
    {
        return m_what;
    }

   private:
    DeserializeErrorCode m_code { DeserializeErrorCode::OK };

    std::string m_what {};
};

template <class T>
std::pair<T, DeserializeError> deserialize(const ByteString& bytes) noexcept
{
    T object {};

    try {
        std::stringstream ss { bytes };

        cereal::PortableBinaryInputArchive ar { ss };

        ar(object);
    } catch (std::exception& exception) {
        return std::make_pair(object, DeserializeError { exception.what() });
    }

    return std::make_pair(
        object,
        DeserializeError { DeserializeErrorCode::OK }
    );
}
