#include "../PureStream/PureStream.h"
#include <iostream>
#include <tuple>
#include <cassert>

Stream<int> ints(int n, int m)
{
    if (n > m)
        return Stream<int>();
    return Stream<int>([n, m]()
    {
        return Cell<int>(n, ints(n + 1, m));
    });
}

Stream<int> intsFrom(int n)
{
    return Stream<int>([n]()
    {
        return Cell<int>(n, intsFrom(n + 1)); 
    });
}

template<class T, class F>
auto fmap(Stream<T> stm, F f)->Stream<decltype(f(stm.get()))>
{
    using U = decltype(f(stm.get()));
    static_assert(std::is_convertible<F, std::function<U(T)>>::value,
        "fmap requires a function type U(T)");

    if (stm.isEmpty()) return Stream<U>();
    return Stream<U>([stm, f]()
    {
        return Cell<U>(f(stm.get()), fmap(stm.popped_front(), f));
    });
}

template<class T, class F>
auto fmapv(Stream<T> stm, F f)->Stream<decltype(f())>
{
    using U = decltype(f());
    static_assert(std::is_convertible<F, std::function<U()>>::value,
        "fmapv requires a function type U()");

    if (stm.isEmpty()) return Stream<U>();
    return Stream<U>([stm, f]()
    {
        return Cell<U>(f(), fmapv(stm.popped_front(), f));
    });
}

template<class T>
Stream<T> mreturn(T v)
{
    return Stream<T>([v]()
    {
        return Cell<T>(v);
    });
}

template<class T>
Stream<T> mjoin(Stream<Stream<T>> stm)
{
    while (!stm.isEmpty() && stm.get().isEmpty())
    {
        stm = stm.popped_front();
    }
    if (stm.isEmpty()) return Stream<T>();
    return Stream<T>([stm]()
    {
        Stream<T> hd = stm.get();
        return Cell<T>(hd.get(), concat(hd.popped_front(), mjoin(stm.popped_front())));
    });
}

template<class T, class F>
auto mbind(Stream<T> stm, F f) -> decltype(f(stm.get()))
{
    return mjoin(fmap(stm, f));
}

template<class T, class F>
auto mthen(Stream<T> stm, F f) -> decltype(f())
{
    return mjoin(fmapv(stm, f));
}

// Use void* for a dummy
Stream<void*> guard(bool b)
{
    if (b) return Stream<void*>(nullptr);
    else return Stream<void*>();
}

Stream<std::tuple<int, int, int>> triples()
{
    return mbind(intsFrom(1), [](int z)
    {
        return mbind(ints(1, z), [z](int x)
        {
            return mbind(ints(x, z), [x, z](int y)
            {
                return mthen(guard(x*x + y*y == z*z), [x, y, z]()
                {
                    return mreturn(std::make_tuple(x, y, z));
                });
            });
        });
    });
}

void test()
{
    auto strm = triples().take(10);
    forEach(std::move(strm), [](std::tuple<int, int, int> const & t)
    {
        std::cout << std::get<0>(t) << ", " << std::get<1>(t) << ", " << std::get<2>(t) << std::endl;
    });
}

void testFmap()
{
    Stream<Stream<int>> ss = fmap(intsFrom(1), [](int z)
    {
        return ints(1, z);
    });

    forEach(std::move(ss.take(5)), [](Stream<int> const & s)
    {
        forEach(std::move(s), [](int i)
        {
            std::cout << i << " ";
        });
        std::cout << std::endl;
    });
}

void testBind()
{
    auto s = mbind(intsFrom(1), [](int z)
    {
        return ints(1, z);
    });

    forEach(std::move(s.take(10)), [](int i)
    {
        std::cout << i << " ";
    });
    std::cout << std::endl;
}

void main()
{
    test();
}
