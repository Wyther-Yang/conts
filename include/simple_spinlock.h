/**
 *      a simple spinlock used like c++ standard lock, See mcs_spinlock.h for better implementation
 *      
 *      spin_lock sp;
 *      
 * thread A: 
 *      {
 *              lock_guard<spin_lock> locked(sp)
 *              // do what you want to
 *      }
 *      
 */

#pragma once

#include <atomic>

using std::atomic;


class spin_lock
{
public:
        template<typename >
        friend class std::lock_guard;

        spin_lock() = default;
        spin_lock(spin_lock const&) = delete;

private:
        atomic<void*> lock{};
};

namespace std {
        template<>
        class lock_guard<spin_lock>
        {       
        public:
                lock_guard(spin_lock& sp) : is_locked(sp.lock)
                {
                        void* expect = nullptr;
                        while (!is_locked.compare_exchange_weak(expect, (void*)this, 
                                                std::memory_order::acquire,
                                                std::memory_order::relaxed))
                        expect = nullptr;
                }

                ~lock_guard()
                {
                        is_locked.store(nullptr, std::memory_order::release);
                }

                lock_guard(lock_guard const&) = delete;

        private:
                atomic<void*>& is_locked;
        };
}
