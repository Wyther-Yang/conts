#ifndef GUID_D7E29D30_E5E3_47A6_822D_9785EC714926_YANG
#define GUID_D7E29D30_E5E3_47A6_822D_9785EC714926_YANG

#include "thread_pool.h"

#include <functional>
using namespace std::chrono_literals;

namespace concurrent {
namespace snippet {

// sort with thread pool
template<typename Pool, typename List>
void
parallel_sort(List& ls)
{
  struct sorter
  {

    Pool p;

    List do_sort(List& ls)
    {
      if (ls.empty())
        return ls;
      List res;
      res.splice(res.begin(), ls, ls.begin());
      auto const& divi_val = *res.begin();
      auto divi_point = std::partition(
        ls.begin(), ls.end(), [&](auto const& val) { return val < divi_val; });
      List lower_chunk;
      lower_chunk.splice(lower_chunk.begin(), ls, ls.begin(), divi_point);
      packaged_task<List()> processing_lower_chunk(
        std::bind(&sorter::do_sort, this, std::ref(lower_chunk)));
      future<List> f = processing_lower_chunk.get_future();
      p.submit(move(processing_lower_chunk));
      List higher_chunk(do_sort(ls));
      res.splice(res.end(), higher_chunk);
      while (f.wait_for(0s) != std::future_status::ready) {
        p.try_executing_one();
      }
      res.splice(res.begin(), f.get());
      return res;
    }

    void operator()(List& ls) { ls = do_sort(ls); }
  };

  sorter{}(ls);
}

//
template<typename Pool, typename Iterator, typename F>
void
parallel_for_each(Iterator first, Iterator last, F __f)
{
  unsigned long const length = distance(first, last);
  if (!length)
    return;
  Pool pool;
  unsigned constexpr min_elements_in_chunk = 2;
  unsigned const chunks =
    (length + min_elements_in_chunk - 1) / min_elements_in_chunk;
  Iterator chunk_first = first;
  vector<future<bool>> fs(chunks);
  for (unsigned i{}; i < chunks; ++i) {
    Iterator chunk_last = chunk_first;
    if (i == chunks - 1)
      chunk_last = last;
    else
      advance(chunk_last, min_elements_in_chunk);
    pool.submit(get_task(fs[i], [=, f = move(__f)]() mutable {
      for (; chunk_first != chunk_last; advance(chunk_first, 1)) {
        f(*chunk_first);
      }
      return true;
    }));

    /*    packaged_task<void()> task([=]() mutable {
          for (; chunk_first != chunk_last; advance(chunk_first, 1))
            __f(*chunk_first);
        });
        fs[i] = task.get_future();
        pool.submit(move(task));
    */
    chunk_first = chunk_last;
  }
  for (auto& _f : fs)
    _f.get();
}

} // namespace snippet

using snippet::parallel_for_each;
using snippet::parallel_sort;

}

#endif /* GUID_D7E29D30_E5E3_47A6_822D_9785EC714926_YANG */
