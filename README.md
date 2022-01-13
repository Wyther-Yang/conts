[![C++17](https://img.shields.io/badge/dialect-C%2B%2B17-blue)](https://en.cppreference.com/w/cpp/17)
[![MIT license](https://img.shields.io/github/license/max0x7ba/atomic_queue)](https://github.com/max0x7ba/atomic_queue/blob/master/LICENSE)
![platform Linux 64-bit](https://img.shields.io/badge/platform-Linux%2064--bit-yellow)
![Latest release](https://img.shields.io/github/v/tag/max0x7ba/atomic_queue?label=latest%20release)
![Ubuntu continuous integration](https://github.com/max0x7ba/atomic_queue/workflows/Ubuntu%20continuous%20integration/badge.svg)

---

# conts

ConTS is a concurrent library, which covers message processing, message passing, task management, efficient and robust thread pool. combined with many characteristics of modern C + +, it focuses on efficiency and ease of use, and has been developed, tested and benchmarked on Linux.

It's also limitation:

* At least with gcc10, because partial features need template argument deduction in C++ 17.

Available objects exposed to user:

* `sender` && `receiver` - a handler of message passing.
* `normal_queue` && `steal_queue` && `msg_queue` - all are template alias, and as container of task and message seperately.
* `wrapper_thread` - a template alias, as its name, it's a wrapped class from std::thread for further control of child threads.
* `tuple` && `function` && `variant` - tool objects, they are specialized from STL to weaken compatibility and enhance special purposes.
* `default_pool` && `ctr_pool` - all are template alias, The former is an ordinary thread pool, and the latter has more powerful control over sub-threads, but the efficiency will be reduced


