#pragma once

// protocol
#include <protocol.hpp>

// std
#include <condition_variable>
#include <mutex>
#include <vector>

namespace common {

class ts_draw_list {
   public:
    ts_draw_list() = default;

    ts_draw_list(const ts_draw_list&) = delete;

    ts_draw_list& operator=(const ts_draw_list&) = delete;

    void update(const prot::Adopt& adopt)
    {
        {
            std::scoped_lock<std::mutex> lock { m_mutex };

            for (auto& tagged_draw : m_vector) {
                if (tagged_draw.username == adopt.username) {
                    tagged_draw.adopted = true;
                }
            }
        }
    }

    void update(const prot::TaggedCommand& tagged_command)
    {
        {
            std::scoped_lock<std::mutex> lock { m_mutex };

            std::string username = tagged_command.username;

            if (std::holds_alternative<prot::draw_t>(tagged_command.command)) {
                auto& draw = std::get<prot::draw_t>(tagged_command.command);

                handle_draw(username, draw);

                return;
            }

            if (std::holds_alternative<prot::select_t>(tagged_command.command
                )) {
                auto& select = std::get<prot::select_t>(tagged_command.command);

                handle_select(username, select);

                return;
            }

            if (std::holds_alternative<prot::delete_t>(tagged_command.command
                )) {
                auto& delete_
                    = std::get<prot::delete_t>(tagged_command.command);

                handle_delete(delete_);

                return;
            }

            if (std::holds_alternative<prot::undo_t>(tagged_command.command)) {

                handle_undo(username);

                return;
            }

            if (std::holds_alternative<prot::clear_t>(tagged_command.command)) {
                auto& clear = std::get<prot::clear_t>(tagged_command.command);

                handle_clear(username, clear);

                return;
            }

            ABORT("unreachable");
        }
        m_cond_var.notify_one();
    }

    std::vector<prot::TaggedDraw> read()
    {
        if (m_mutex.try_lock()) {
            auto copy = m_vector;
            m_mutex.unlock();
            return copy;
        }

        std::unique_lock lock { m_cond_mutex };
        m_cond_var.wait(lock, []() {
            return false;
        });
        return m_vector;
    }

   private:
    void handle_draw(std::string username, prot::draw_t draw)
    {
        m_vector.push_back({ false, std::move(username), std::move(draw) });
    }
    void handle_select(std::string username, prot::select_t select)
    {
        m_vector[static_cast<size_t>(select.id)]
            = { false, std::move(username), std::move(select.draw) };
    }
    void handle_delete(prot::delete_t delete_)
    {
        m_vector.erase(m_vector.begin() + delete_.id);
    }
    void handle_undo(std::string& username)
    {
        for (auto iter = m_vector.rbegin(); iter != m_vector.rend(); iter++) {
            if (iter->username == username) {
                m_vector.erase(iter.base());

                break;
            }
        }
    }
    void handle_clear(std::string& username, prot::clear_t clear)

    {
        switch (clear.quailifier) {
        case prot::qualifier_e::ALL:
            m_vector.clear();
            break;
        case prot::qualifier_e::MINE:
            std::vector<prot::TaggedDraw> filtered_commands {};

            filtered_commands.reserve(m_vector.size());

            for (auto& tagged_draw : m_vector) {
                if (tagged_draw.username != username) {
                    filtered_commands.push_back(std::move(tagged_draw));
                }
            }

            m_vector = std::move(filtered_commands);
            break;
        }
    }

    std::vector<prot::TaggedDraw> m_vector {};
    std::mutex m_mutex {};
    std::mutex m_cond_mutex {};
    std::condition_variable m_cond_var {};
};

class TaggedDrawVectorWrapper {
   public:
    explicit draw_list_wrapper(prot::TaggedDrawList& list)
        : m_list(list)
    {
    }

    draw_list_wrapper(const ts_draw_list&) = delete;

    draw_list_wrapper& operator=(const ts_draw_list&) = delete;

    void update(const prot::TaggedCommand& tagged_command)
    {
        std::string username = tagged_command.username;

        if (std::holds_alternative<prot::draw_t>(tagged_command.command)) {
            auto& draw = std::get<prot::draw_t>(tagged_command.command);

            handle_draw(username, draw);

            return;
        }

        if (std::holds_alternative<prot::select_t>(tagged_command.command)) {
            auto& select = std::get<prot::select_t>(tagged_command.command);

            handle_select(username, select);

            return;
        }

        if (std::holds_alternative<prot::delete_t>(tagged_command.command)) {
            auto& delete_ = std::get<prot::delete_t>(tagged_command.command);

            handle_delete(delete_);

            return;
        }

        if (std::holds_alternative<prot::undo_t>(tagged_command.command)) {

            handle_undo(username);

            return;
        }

        if (std::holds_alternative<prot::clear_t>(tagged_command.command)) {
            auto& clear = std::get<prot::clear_t>(tagged_command.command);

            handle_clear(username, clear);

            return;
        }

        ABORT("unreachable");
    }

   private:
    void handle_draw(std::string username, prot::draw_t draw)
    {
        m_list.push_back({ std::move(username), std::move(draw) });
    }
    void handle_select(std::string username, prot::select_t select)
    {
        m_list[static_cast<size_t>(select.id)]
            = { std::move(username), std::move(select.draw) };
    }
    void handle_delete(prot::delete_t delete_)
    {
        m_list.erase(m_list.begin() + delete_.id);
    }
    void handle_undo(std::string& username)
    {
        for (auto iter = m_list.rbegin(); iter != m_list.rend(); iter++) {
            if (iter->username == username) {
                m_list.erase(iter.base());

                break;
            }
        }
    }
    void handle_clear(std::string& username, prot::clear_t clear)

    {
        switch (clear.quailifier) {
        case prot::qualifier_e::ALL:
            m_list.clear();
            break;
        case prot::qualifier_e::MINE:
            std::vector<prot::TaggedDraw> filtered_commands {};

            filtered_commands.reserve(m_list.size());

            for (auto& tagged_draw : m_list) {
                if (tagged_draw.username != username) {
                    filtered_commands.push_back(std::move(tagged_draw));
                }
            }

            m_list = std::move(filtered_commands);
            break;
        }
    }

    prot::TaggedDrawList& m_list;
};

} // namespace common
