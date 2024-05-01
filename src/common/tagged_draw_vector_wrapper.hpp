#pragma once

// common
#include "overload.hpp"
#include "serial.hpp"
#include "types.hpp"

// std
#include <algorithm>
#include <string>
#include <variant>

// This is a very simple wrapper around
// a tagged draw vector which implements
// all the necessary mutations

class TaggedDrawVectorWrapper {
   public:
    explicit TaggedDrawVectorWrapper(TaggedDrawVector& vector)
        : m_vector(vector)
    {
    }

    TaggedDrawVectorWrapper(const TaggedDrawVectorWrapper&) = delete;

    TaggedDrawVectorWrapper& operator=(const TaggedDrawVectorWrapper&) = delete;

    std::size_t hash()
    {
        std::size_t value { 0 };

        for (auto& tagged_action : m_vector) {
            ByteString bytes = serialize(tagged_action);

            value = value ^ (std::hash<ByteString> {}(bytes) << 1);
        }

        return value;
    }

    void adopt(const Adopt& adopt)
    {
        for (auto iter = m_vector.rbegin(); iter != m_vector.rend(); iter++) {
            if (iter->username == adopt.username) {
                iter->adopted = true;
            }
        }
    }

    void update(const TaggedAction& tagged_action)
    {
        std::string username = tagged_action.username;

        std::visit(
            overload {
                [this, username](const Clear& arg) {
                    handle(username, arg);
                },
                [this, username](const Undo& arg) {
                    handle(username, arg);
                },
                [this, username](const Delete& arg) {
                    handle(username, arg);
                },
                [this, username](const Select& arg) {
                    handle(username, arg);
                },
                [this, username](const Draw& arg) {
                    handle(username, arg);
                },
            },
            tagged_action.action
        );
    }

   private:
    void handle(const std::string& username, const Draw& arg)
    {
        m_vector.push_back(TaggedDraw { false, username, arg });
    }
    void handle(const std::string& username, const Select& arg)
    {
        m_vector[static_cast<size_t>(arg.id)] = { false, username, arg.draw };
    }
    void handle(const std::string&, const Delete& arg)
    {
        if (m_vector.empty())
            return;

        // NOTE: we have to be careful here since we can
        // actually exceed the size of the vector and the server will then
        // basically give up and crash

        long id
            = std::clamp(arg.id, 0l, static_cast<long>(m_vector.size() - 1));

        m_vector.erase(m_vector.begin() + id);
    }
    void handle(const std::string& username, const Undo&)
    {
        if (m_vector.empty())
            return;

        for (auto iter = m_vector.rbegin(); iter != m_vector.rend(); iter++) {
            if (iter->username == username) {
                m_vector.erase(iter.base());

                break;
            }
        }
    }
    void handle(const std::string& username, const Clear& arg)
    {
        switch (arg.qualifier) {
        case Qualifier::ALL:
            m_vector.clear();

            break;
        case Qualifier::MINE:
            TaggedDrawVector filtered_vector {};

            filtered_vector.reserve(m_vector.size());

            for (auto& tagged_draw : m_vector) {
                if (tagged_draw.username != username) {
                    filtered_vector.push_back(std::move(tagged_draw));
                }
            }

            m_vector = std::move(filtered_vector);

            break;
        }
    }

    TaggedDrawVector& m_vector;
};
