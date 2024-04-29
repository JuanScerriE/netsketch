// cstd
#include <ctime>

// server
#include "share.hpp"

// common
#include "../common/tagged_draw_vector_wrapper.hpp"
#include "../common/threading.hpp"
#include "../common/types.hpp"

// spdlog
#include <spdlog/spdlog.h>

namespace server {

void handle_timer(union sigval val)
{
    // NOTE: please look at the note at the end in the main
    // file regarding the resource reclamation when using
    // timer_create
    auto* timer = static_cast<TimerData*>(val.sival_ptr);

    std::string username { timer->user };

    {
        Adopt adopt { username };

        {
            threading::unique_mutex_guard guard { share::update_mutex };

            TaggedDrawVectorWrapper { share::tagged_draw_vector }.adopt(adopt);

            share::payload_queue.emplace(adopt);
        }

        share::update_cond.notify_one();
    }

    {
        threading::mutex_guard guard { share::timers_mutex };

        for (auto iter = share::timers.cbegin(); iter != share::timers.cend();
             iter++) {
            if (iter->get() == timer) {
                share::timers.erase(iter);

                break;
            }
        }
    }

    spdlog::info("adopted {}'s draws", username);
}

void create_client_timer(const std::string& user)
{
    std::unique_ptr<TimerData> data = std::make_unique<TimerData>();

    data->user = user;

    // this is all the C setup for using a timer_create

    sigevent ev {};
    bzero(&ev, sizeof(struct sigevent));

    sigval value {};
    value.sival_ptr = data.get();

    ev.sigev_notify = SIGEV_THREAD;
    ev.sigev_value = value;
    ev.sigev_notify_function = handle_timer;

    data->timer.create(CLOCK_REALTIME, &ev);

    TimerData* data_raw = data.get();

    {
        threading::mutex_guard guard { share::timers_mutex };

        share::timers.push_front(std::move(data));

        itimerspec its {};
        bzero(&its, sizeof(itimerspec));
        its.it_value.tv_sec = 60 * 5; // 5 minutes;
        its.it_value.tv_nsec = 0;

        data_raw->timer.set(0, &its, nullptr);
    }
}

} // namespace server
