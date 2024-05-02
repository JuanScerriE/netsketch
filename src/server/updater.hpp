#pragma once

namespace server {

// NOTE: ideally we have a thread pool of updaters
// and each updater is tasked with updating a certain
// number of clients. This would allow us to tweak
// the ratio of clients to updater threads allowing us
// reduce the number of updaters whilst still
// maintaining performance since we are reducing the context
// switching and compensating for more threads
// by leveraging the single threaded performance of our
// CPU.

class Updater {
   public:
    [[noreturn]] void operator()();
};

} // namespace server
