[![C++17](https://img.shields.io/badge/dialect-C%2B%2B17-blue)](https://en.cppreference.com/w/cpp/17)
[![MIT license](https://img.shields.io/github/license/max0x7ba/atomic_queue)](https://github.com/max0x7ba/atomic_queue/blob/master/LICENSE)
![platform Linux 64-bit](https://img.shields.io/badge/platform-Linux%2064--bit-yellow)
![Latest release](https://img.shields.io/github/v/tag/max0x7ba/atomic_queue?label=latest%20release)
![Ubuntu](https://github.com/max0x7ba/atomic_queue/workflows/Ubuntu%20continuous%20integration/badge.svg)

---

# conts

ConTS is a concurrent library, which covers message processing, message passing, task management, efficient and robust thread pool. combined with many characteristics of modern C + +, it focuses on efficiency and ease of use, and has been developed, tested and benchmarked on Linux.

It's also limitation:

* At least with gcc10, because partial features need template argument deduction in C++ 17.

Available objects exposed to user:

* `sender` && `receiver` - a handler of message passing.

* `normal_queue` && `steal_queue` && `msg_queue` - all are template alias, and as container of task and message seperately.

* `hl_queue` - a template class, is suitable as the main interaction queue in high-load scenarios. depending on the specific situation maybe need adjust slightly. 

* `ll_stack` && `hl_stack` - all are template alias, designed for low(ll) and high(hl) load requirement.

* `wrapper_thread` - a template alias, as its name, it's a wrapped class from std::thread for further control of child threads.

* `tuple` && `function` && `variant` && `any` - tool objects, they are specialized from STL to weaken compatibility and enhance special purposes.

* `default_pool` && `ctr_pool` - all are template alias, The former is an ordinary thread pool, and the latter has more powerful control over sub-threads, but the efficiency will be reduced.

Available core API exposed to user:

* `wait()` - from `receiver`, starting interface for processing message.

* `send(_M& )`  - from `sender`, message passing interface.

* `push(T )` - from `normal_queue` and `steal_queue`. In general, normal_queue is faster because it has a lower granularity than steal_queue is  designed to exchange space for time.

* `pull()` && `pull(T& )` - from `normal_queue`. it's blocked on condition_variable.

* `try_pull(T& )` - from `normal_queue` and `steal_queue`. it's not blocked, except blocking on mutex.

* `interrupt_point()`. it gives the feature of interruption for improving responsiveness with concurrency.

* `interrupt_wait(...)`, a variadic parameter template interface. like `interrupt_point()`, but it targets the blocking process on std::condition_variable or std::future;

* `interrupt()` - from `wrapper_thread`, it active interrupt sub-thread.

* `apply(F&& , Tp&&)` , it applys some kind of operation to each element of `tuple`.

* `apply(F&& )` - from `variant`, it applys an operation to an element.

* `try_executing_one()` - from `default_pool` and `ctr_pool`, it gives the ability of other threads to perform tasks in the thread pool.

* `sumit(F&& )` - from `default_pool` and `ctr_pool`.

# Using the library
The lib provides header-only class templates, no building/installing is necessary.

## Install from GitHub
1. Clone the project:
```
https://github.com/Wyther-Yang/conts.git
```
2. Add `conts/include` directory (use full path) to the include paths of your build system.

3. `#include <conts.h>` in your c++ source.

## Implementation Notes
Almost all API in the lib appear as the style of passe by value , so Rvalue ref constructor and assign must be provided if want to be efficient. Template argument deduction from constructor and variadic parameter template is needed for easy to use and simple to interface.

## Benchmarks
`bm_test(F , unsigned)` from `benchmark.h` is used as benchmark interface, with algorithms from `con_algo.h` test the correctness and efficiency of the components. 

### Contributing
The project uses Mozilla's `clang-format` to automate formatting. Pull requests are expected to be formatted using the same settings.

---

Copyright (c) 2022  MIT License. See the full licence in file LICENSE.
