#pragma once

// common
#include "bytes.hpp"

// cereal
#include <cereal/archives/portable_binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

// We are wrapping everything in some
// templated function to allow for serialization
// and de-serialization of any object
// which has the necessary serialize method required
// by Cereal. In particular throughout the code we only
// really use the Payload type so we have to be a bit careful
// as this will result in the incorrect payload being sent.

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

    // NOTE: deserialization has to be wrapped in a general
    // try-catch since Cereal (the library we are using for
    // serializtion) does not perform any bounds checking
    // i.e. if the server receives and almost correct
    // packet which contains the length of a string, it is
    // very likely that the length will be a very large number
    // which the allocator cannot satisfy. If not handled
    // properly this will kill the server. Of course
    // we cannot have the server dying on such a request.
    // Luckily for us in such a scenario an exception is raised
    // either a std::length_error or an empty exception which
    // we can catch.
    //
    // In particular this is what the STL (in gcc 13) has to say
    // about the .reserve method for the std::vector container
    // type
    //
    // @brief  Attempt to preallocate enough memory for specified number of
    //         elements.
    // @param  __n  Number of elements required.
    // @throw  std::length_error  If @a n exceeds @c max_size().

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
