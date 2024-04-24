#include <thread>

class Channel {
public:
    void reader() {

    }

    void writer() {

    }

private:
    std::thread m_reader {};
    std::thread m_writer {};
};
