#pragma once

// cstd
#include <cstdint>

namespace server {

class Runner {
   public:
    Runner() = default;

    bool setup(uint16_t port, float time_out);

    [[nodiscard]] bool run() const;

    ~Runner();

   private:
    uint16_t m_port {};
};

} // namespace client
