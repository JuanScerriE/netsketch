// std
#include <csignal>

// share
#include <share.hpp>

void create_client_timer() { }

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

void handle_timer(union sigval val)
{
    int(void) val;
    {
    }
}
