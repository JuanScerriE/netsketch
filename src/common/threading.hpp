#pragma once

// std
#include <cerrno>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// pthreads
#include <pthread.h>

#define USE_POP (1)

namespace threading {

// The below is a list of C++ wrappers around the underlying POSIX mechanism.
// The whole point of going through all of this trouble is guarantee
// compatibility with all POSIX functions. In general using any of the
// multi-threading primitives provided by the STL will limit. This is because
// the C++ STL tries to generalize over a number of implementations for
// threading/synchronization primitives which will all have their own quirks.
// The problem with this is that the STL has skimped on a number of useful
// features provided by the pthread POSIX API. Specifically, pthread_cancel.
// This function is powerful because it is capable of waking up a thread even in
// situations where the thread is blocking some resource, of course this
// function employs the help of the kernel (please look at man pthread_cancel
// and specifically cancellation points). Below attribution is made specifically
// for the C++ wrapper of pthreads essentially providing the same interface as
// std::thread. With all these helpers we can have albeit a very
// rudimentary, version of the STL primitives which guarantee a
// 100% compatibility with POSIX (in particular we can employ
// pthread_cleanup handlers)

// ATTRIBUTION:
// https://codereview.stackexchange.com/questions/189089/simplified-stdthread-implementation-using-pthreads

namespace details {
    template <typename Wrapper>
    void del_target(void* target)
    {
        auto* callable = reinterpret_cast<Wrapper*>(target);

        (*callable).die();

        delete callable;
    }

    template <typename Wrapper>
    void* call_target(void* target)
    {
        auto* callable = reinterpret_cast<Wrapper*>(target);

        pthread_cleanup_push(del_target<Wrapper>, callable);

        (*callable)();

        pthread_cleanup_pop(USE_POP);

        return nullptr;
    }

    template <typename Callable, typename... ArgTypes>
    class thread_wrapper {
       public:
        thread_wrapper(const thread_wrapper<Callable, ArgTypes...>&) = delete;

        thread_wrapper<Callable, ArgTypes...>&
        operator=(const thread_wrapper<Callable, ArgTypes...>&)
            = delete;

        explicit thread_wrapper(
            bool* is_alive,
            Callable&& callable,
            ArgTypes&&... args
        )
            : is_alive { is_alive }
            , callable { std::forward<Callable>(callable) }
            , args { std::forward<ArgTypes>(args)... }
        {
        }

        void die()
        {
            *is_alive = false;
        }

        void operator()()
        {
            std::apply(callable, args);
        }

       private:
        bool* is_alive { nullptr };

        Callable callable;

        std::tuple<std::decay_t<ArgTypes>...> args;
    };

} // namespace details

class thread {
   public:
    thread() = default;

    template <typename Callable, typename... ArgTypes>
    explicit thread(Callable&& callable, ArgTypes&&... args)
    {
        m_is_alive = new bool { true };

        auto* target = new details::thread_wrapper<Callable, ArgTypes...>(
            m_is_alive,
            std::forward<Callable>(callable),
            std::forward<ArgTypes>(args)...
        );

        using signature = std::remove_reference_t<decltype(*target)>;

        auto creation_result = pthread_create(
            &thread_handle,
            nullptr,
            &details::call_target<signature>,
            target
        );

        if (creation_result != 0) {
            delete target;

            *m_is_alive = false;

            throw std::runtime_error { strerror(creation_result) };
        }
    }

    thread(const thread&) = delete;

    thread& operator=(const thread&) = delete;

    thread(thread&& other) noexcept
        : m_is_alive { std::exchange(other.m_is_alive, nullptr) }
        , thread_handle {
            std::exchange(other.thread_handle, static_cast<pthread_t>(-1))
        }
    {
    }

    thread& operator=(thread&& other) noexcept
    {
        m_is_alive = std::exchange(other.m_is_alive, nullptr);

        thread_handle
            = std::exchange(other.thread_handle, static_cast<pthread_t>(-1));

        return *this;
    }

    [[nodiscard]] pthread_t native_handle() const
    {
        return thread_handle;
    }

    void join() const
    {
        auto ret = pthread_join(thread_handle, nullptr);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void detach() const
    {
        auto ret = pthread_detach(thread_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void cancel() const
    {
        auto ret = pthread_cancel(thread_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    static void test_cancel() noexcept
    {
        pthread_testcancel();
    }

    static thread self()
    {
        thread this_thread {};

        *this_thread.m_is_alive = true;

        this_thread.thread_handle = pthread_self();

        return this_thread;
    }

    ~thread()
    {
        delete m_is_alive;
    }

    [[nodiscard]] bool is_alive() const
    {
        if (m_is_alive)
            return *m_is_alive;

        throw std::logic_error { "uninitialized thread" };
    }

    [[nodiscard]] bool is_initialized() const
    {
        return m_is_alive;
    }

   private:
    bool* m_is_alive { nullptr };

    pthread_t thread_handle { static_cast<pthread_t>(-1) };
};

class mutex {
   public:
    mutex(const mutex&) = delete;

    mutex& operator=(const mutex&) = delete;

    mutex(mutex&& other) noexcept
        : mutex_handle {
            std::exchange(other.mutex_handle, PTHREAD_MUTEX_INITIALIZER)
        }
    {
    }

    mutex& operator=(mutex&& other) noexcept
    {
        mutex_handle
            = std::exchange(other.mutex_handle, PTHREAD_MUTEX_INITIALIZER);

        return *this;
    }

    mutex()
    {
        auto ret = pthread_mutex_init(&mutex_handle, nullptr);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    ~mutex() noexcept(false)
    {
        auto ret = pthread_mutex_destroy(&mutex_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void lock()
    {
        auto ret = pthread_mutex_lock(&mutex_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    bool try_lock()
    {
        auto ret = pthread_mutex_trylock(&mutex_handle);

        if (ret == EBUSY) {
            return false;
        }

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }

        return true;
    }

    void unlock()
    {
        auto ret = pthread_mutex_unlock(&mutex_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    pthread_mutex_t* native_handle_ptr()
    {
        return &mutex_handle;
    }

   private:
    pthread_mutex_t mutex_handle = PTHREAD_MUTEX_INITIALIZER;
};

class mutex_guard {
   public:
    explicit mutex_guard(mutex& mutex_ref)
        : mutex_ref(mutex_ref)
    {
        mutex_ref.lock();
    }

    mutex_guard(const mutex_guard&) = delete;

    mutex_guard& operator=(const mutex_guard&) = delete;

    ~mutex_guard()
    {
        mutex_ref.unlock();
    }

   private:
    mutex& mutex_ref;
};

enum class unique_guard_policy {
    try_to_lock,
};

class unique_mutex_guard {
   public:
    explicit unique_mutex_guard(mutex& mutex_ref)
        : mutex_ref(mutex_ref)
    {
        lock();
    }

    explicit unique_mutex_guard(mutex& mutex_ref, unique_guard_policy policy)
        : mutex_ref(mutex_ref)
    {
        switch (policy) {
        case threading::unique_guard_policy::try_to_lock:
            try_lock();

            break;
        };
    }

    unique_mutex_guard(const unique_mutex_guard&) = delete;

    unique_mutex_guard& operator=(const unique_mutex_guard&) = delete;

    void lock()
    {
        if (owning == false) {
            mutex_ref.lock();

            owning = true;
        }
    }

    bool try_lock()
    {
        bool acquired = mutex_ref.try_lock();

        if (acquired) {
            owning = true;
        }

        return acquired;
    }

    void unlock()
    {
        if (owning == true) {
            mutex_ref.unlock();
        }

        owning = false;
    }

    [[nodiscard]] bool is_owning() const
    {
        return owning;
    }

    ~unique_mutex_guard()
    {
        unlock();
    }

    pthread_mutex_t* native_handle_ptr()
    {
        return mutex_ref.native_handle_ptr();
    }

   private:
    bool owning { false };

    mutex& mutex_ref;
};

class cond_var {
   public:
    cond_var(const cond_var& other) = delete;

    cond_var& operator=(const cond_var& other) = delete;

    cond_var(cond_var&& other) noexcept
        : cond_handle {
            std::exchange(other.cond_handle, PTHREAD_COND_INITIALIZER)
        }
    {
    }

    cond_var& operator=(cond_var&& other) noexcept
    {
        cond_handle
            = std::exchange(other.cond_handle, PTHREAD_COND_INITIALIZER);

        return *this;
    }

    cond_var()
    {
        auto ret = pthread_cond_init(&cond_handle, nullptr);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    ~cond_var() noexcept(false)
    {
        auto ret = pthread_cond_destroy(&cond_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void notify_one()
    {
        auto ret = pthread_cond_signal(&cond_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void notify_all()
    {
        auto ret = pthread_cond_broadcast(&cond_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void wait(unique_mutex_guard& lock)
    {
        auto ret = pthread_cond_wait(&cond_handle, lock.native_handle_ptr());

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void wait(unique_mutex_guard& lock, const std::function<bool()>& pred)
    {
        while (pred()) {
            auto ret
                = pthread_cond_wait(&cond_handle, lock.native_handle_ptr());

            if (ret != 0) {
                throw std::runtime_error { strerror(ret) };
            }
        }
    }

   private:
    pthread_cond_t cond_handle = PTHREAD_COND_INITIALIZER;
};

class rwlock {
   public:
    rwlock(const rwlock&) = delete;

    rwlock& operator=(const rwlock&) = delete;

    rwlock(rwlock&& other) noexcept
        : rwlock_handle {
            std::exchange(other.rwlock_handle, PTHREAD_RWLOCK_INITIALIZER)
        }
    {
    }

    rwlock& operator=(rwlock&& other) noexcept
    {
        rwlock_handle
            = std::exchange(other.rwlock_handle, PTHREAD_RWLOCK_INITIALIZER);

        return *this;
    }

    rwlock()
    {
        auto ret = pthread_rwlock_init(&rwlock_handle, nullptr);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    ~rwlock() noexcept(false)
    {
        auto ret = pthread_rwlock_destroy(&rwlock_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    void rdlock()
    {
        auto ret = pthread_rwlock_rdlock(&rwlock_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    bool try_rdlock()
    {
        auto ret = pthread_rwlock_tryrdlock(&rwlock_handle);

        if (ret == EBUSY) {
            return false;
        }

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }

        return true;
    }

    void wrlock()
    {
        auto ret = pthread_rwlock_wrlock(&rwlock_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    bool try_wrlock()
    {
        auto ret = pthread_rwlock_trywrlock(&rwlock_handle);

        if (ret == EBUSY) {
            return false;
        }

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }

        return true;
    }

    void unlock()
    {
        auto ret = pthread_rwlock_unlock(&rwlock_handle);

        if (ret != 0) {
            throw std::runtime_error { strerror(ret) };
        }
    }

    pthread_rwlock_t* native_handle_ptr()
    {
        return &rwlock_handle;
    }

   private:
    pthread_rwlock_t rwlock_handle = PTHREAD_RWLOCK_INITIALIZER;
};

class rwlock_rdguard {
   public:
    explicit rwlock_rdguard(rwlock& rwlock_ref)
        : rwlock_ref(rwlock_ref)
    {
        rwlock_ref.rdlock();
    }

    rwlock_rdguard(const rwlock_rdguard&) = delete;

    rwlock_rdguard& operator=(const rwlock_rdguard&) = delete;

    ~rwlock_rdguard()
    {
        rwlock_ref.unlock();
    }

   private:
    rwlock& rwlock_ref;
};

class unique_rwlock_rdguard {
   public:
    explicit unique_rwlock_rdguard(rwlock& rwlock_ref)
        : rwlock_ref(rwlock_ref)
    {
        lock();
    }

    explicit unique_rwlock_rdguard(
        rwlock& rwlock_ref,
        unique_guard_policy policy
    )
        : rwlock_ref(rwlock_ref)
    {
        switch (policy) {
        case threading::unique_guard_policy::try_to_lock:
            try_lock();

            break;
        };
    }

    unique_rwlock_rdguard(const unique_rwlock_rdguard&) = delete;

    unique_rwlock_rdguard& operator=(const unique_rwlock_rdguard&) = delete;

    void lock()
    {
        if (owning == false) {
            rwlock_ref.rdlock();

            owning = true;
        }
    }

    bool try_lock()
    {
        bool acquired = rwlock_ref.try_rdlock();

        if (acquired) {
            owning = true;
        }

        return acquired;
    }

    void unlock()
    {
        if (owning == true) {
            rwlock_ref.unlock();
        }

        owning = false;
    }

    [[nodiscard]] bool is_owning() const
    {
        return owning;
    }

    ~unique_rwlock_rdguard()
    {
        unlock();
    }

    pthread_rwlock_t* native_handle_ptr()
    {
        return rwlock_ref.native_handle_ptr();
    }

   private:
    bool owning { false };

    rwlock& rwlock_ref;
};

class rwlock_wrguard {
   public:
    explicit rwlock_wrguard(rwlock& rwlock_ref)
        : rwlock_ref(rwlock_ref)
    {
        rwlock_ref.wrlock();
    }

    rwlock_wrguard(const rwlock_wrguard&) = delete;

    rwlock_wrguard& operator=(const rwlock_wrguard&) = delete;

    ~rwlock_wrguard()
    {
        rwlock_ref.unlock();
    }

   private:
    rwlock& rwlock_ref;
};

class unique_rwlock_wrguard {
   public:
    explicit unique_rwlock_wrguard(rwlock& rwlock_ref)
        : rwlock_ref(rwlock_ref)
    {
        lock();
    }

    explicit unique_rwlock_wrguard(
        rwlock& rwlock_ref,
        unique_guard_policy policy
    )
        : rwlock_ref(rwlock_ref)
    {
        switch (policy) {
        case threading::unique_guard_policy::try_to_lock:
            try_lock();

            break;
        };
    }

    unique_rwlock_wrguard(const unique_rwlock_wrguard&) = delete;

    unique_rwlock_wrguard& operator=(const unique_rwlock_wrguard&) = delete;

    void lock()
    {
        if (owning == false) {
            rwlock_ref.wrlock();

            owning = true;
        }
    }

    bool try_lock()
    {
        bool acquired = rwlock_ref.try_wrlock();

        if (acquired) {
            owning = true;
        }

        return acquired;
    }

    void unlock()
    {
        if (owning == true) {
            rwlock_ref.unlock();
        }

        owning = false;
    }

    [[nodiscard]] bool is_owning() const
    {
        return owning;
    }

    ~unique_rwlock_wrguard()
    {
        unlock();
    }

    pthread_rwlock_t* native_handle_ptr()
    {
        return rwlock_ref.native_handle_ptr();
    }

   private:
    bool owning { false };

    rwlock& rwlock_ref;
};

} // namespace threading
