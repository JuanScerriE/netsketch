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
        its.it_value.tv_sec = 30; // 5 minutes;
        its.it_value.tv_nsec = 0;

        data_raw->timer.set(0, &its, nullptr);
    }
}

// so, at first I thought I'd want
// to go with an unordered_map to actually
// store the timers. However, then
// I realised that I  need to destroy and remove these
// timers. Of course the ideal way to do this
// is within the thread which runs to service the timer
// going off in the first place.
//
// But then this timer needs to be aware of the
// timerid which started it to close
// the program (this can be done see the man page
// for timer_create() on man7.org). However,
// we cannot pass pointer to the memory location
// of the timer inside the unordered_map
// what if it moves the memory around we essentially
// have an invalid memory access.
//
// So the best solution for this approach is actually
// to limit the number of timers and by extension
// users because we want to make sure that they are where
// they claim to be. This of course means we need
// to refuse further connections. For know
// we'll just flat out refuse the connection.
//
// Again I have to do this because I cannot make any
// assumptions about the data structure which
// is a timer_t. Additionally, one could argue
// that you just pass in an int into the sigval
// but then its just a matter of time until you
// run out of space. I mean for all practical purposes
// the number of clients will almost never exceed the
// maximum number attainable by an integer but to
// be precise we will set an upper bound.

} // namespace server::timing
