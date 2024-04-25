#pragma once

// common
#include "overload.hpp"
#include "types.hpp"

// std
#include <string>
#include <variant>

class TaggedDrawVectorWrapper {
   public:
    explicit TaggedDrawVectorWrapper(TaggedDrawVector& vector)
        : m_vector(vector)
    {
    }

    TaggedDrawVectorWrapper(const TaggedDrawVectorWrapper&) = delete;

    TaggedDrawVectorWrapper& operator=(const TaggedDrawVectorWrapper&) = delete;

    void adopt(const std::string& username)
    {
        for (auto iter = m_vector.rbegin(); iter != m_vector.rend(); iter++) {
            if (iter->username == username) {
                iter->adopted = true;
            }
        }
    }

    void update(const TaggedAction& tagged_action)
    {
        std::string username = tagged_action.username;

        std::visit(
            overload {
                [this, username](Draw& arg) {
                    handle(username, arg);
                },
                [this, username](Select& arg) {
                    handle(username, arg);
                },
                [this, username](Delete& arg) {
                    handle(username, arg);
                },
                [this, username](Undo& arg) {
                    handle(username, arg);
                },
                [this, username](Clear& arg) {
                    handle(username, arg);
                },
            },
            tagged_action.action
        );
    }

   private:
    void handle(std::string username, Draw arg)
    {
        m_vector.push_back(
            TaggedDraw { false, std::move(username), std::move(arg) }
        );
    }
    void handle(std::string username, Select arg)
    {
        m_vector[static_cast<size_t>(arg.id)]
            = { false, std::move(username), std::move(arg.draw) };
    }
    void handle(std::string, Delete arg)
    {
        m_vector.erase(m_vector.begin() + arg.id);
    }
    void handle(std::string username, Undo)
    {
        for (auto iter = m_vector.rbegin(); iter != m_vector.rend(); iter++) {
            if (iter->username == username) {
                m_vector.erase(iter.base());

                break;
            }
        }
    }
    void handle(std::string username, Clear arg)
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
