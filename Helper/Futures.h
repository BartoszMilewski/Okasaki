#if !defined (FUTURES_H)
#define FUTURES_H

#include <future>
#include "../List/List.h"

template<class T>
std::future<T> make_ready_future(T val)
{
    std::promise<T> promise;
    promise.set_value(val);
    return promise.get_future();
}

template<class T>
std::vector<T> when_all_vec(std::vector<std::future<T>> & ftrs)
{
    std::vector<T> lst;
    while (!ftrs.empty())
    {
        std::future<T> f = std::move(ftrs.back());
        lst.push_back(f.get());
        ftrs.pop_back();
    }
    return lst;
}

template<class T>
List<T> when_all(List<std::future<T>> & ftrs)
{
    List<T> lst;
    while (!ftrs.empty())
    {
        std::future<T> f = std::move(ftrs.front());
        lst = lst.pushed_front(f.get());
        ftrs.popped_front();
    }
    return lst;
}

#endif
