/**
 *      from: https://lwn.net/Articles/590243/
 *      MCS-spinlock is a very interesting structure, let me implement it;
 * 
 *      how to use:
 *      
 *      main thrad:
 *              mcs_lock m;         
 *     
 *      work thread:
 *              // you can use it like c++ standard lock
 *              {
 *                      std::lock_guard<mcs_lock> lk(m);
 *                      
 *                      // do what you want to  
 *                      
 *              }
 *      
 */

#pragma once

#include <atomic>
#include <thread>

using std::atomic;
using std::memory_order::relaxed;
using std::memory_order::release;
using std::memory_order::acquire;


class mcs_lock
{
public:

        template<typename >
        friend class std::lock_guard;

        struct node {
                atomic<bool> is_owned{false};
                atomic<node*> next{nullptr};
        };

        mcs_lock() = default;
        mcs_lock(mcs_lock const&) = delete;

        // dangerous, while() no guarantee that there are no escaped nodes
        ~mcs_lock()
        {
                while (tail.load(relaxed))
                        std::this_thread::yield();
        }

private:
        atomic<node*> tail{};
};


namespace std {
        template<>
        class lock_guard<mcs_lock>
        {
        public:
                lock_guard(mcs_lock& inlk) : lk(inlk.tail)
                {
                        auto* tail = lk.exchange(&n, relaxed);
                        if (tail)
                        {
                                tail->next.store(&n, relaxed);
                                
                                // stuck in roll
                                while (!n.is_owned.load(acquire));
                        }
                }

                ~lock_guard()
                {
                        auto* tail = &n;
                        if (!lk.compare_exchange_strong(tail, nullptr, relaxed))
                        {
                                // baby time
                                while (!n.next.load(relaxed));

                                auto* next = n.next.load(relaxed);
                                next->is_owned.store(true, release);
                        }
                }

                lock_guard(const lock_guard&) = delete;
                lock_guard operator=(lock_guard&) = delete;

        private:
                atomic<mcs_lock::node*>& lk;

                alignas(64)
                mcs_lock::node n;
        };
}


