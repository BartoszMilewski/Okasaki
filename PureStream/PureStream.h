#include <cassert>
#include <functional>
#include <memory>
#include "Susp.h"

template<class T>
class Stream;

// A Cell contains a value and a (lazy) Stream

template<class T>
class Cell
{
public:
    Cell() {} // only to initialize _memo
    Cell(T v, Stream<T> tail)
        : _v(v), _tail(std::move(tail))
    {}
    explicit Cell(T v) : _v(v) {}
    T val() const
    {
        return _v;
    }
    Stream<T> popped_front() const
    {
        return _tail;
    }
private:
    T _v;
    Stream<T> _tail;
};

// CellFun is a function object that creates a Cell 
// containing a given value and a Stream

template<class T>
class CellFun
{
public:
    CellFun(T v, Stream<T> s) : _v(v), _s(std::move(s)) {}
    explicit CellFun(T v) : _v(v) {}

    Cell<T> operator()()
    {
        return Cell<T>(_v, _s);
    }
    T _v;
    Stream<T> _s;
};

// Stream is either empty
// or contains a suspended Cell

template<class T>
class Stream
{
private:
    std::shared_ptr <Susp<Cell<T>>> _lazyCell;
public:
    Stream() {}
    explicit Stream(T v)
    {
        auto f = CellFun<T>(v);
        _lazyCell = std::make_shared<Susp<Cell<T>>>(f);
    }
    Stream(T v, Stream s)
    {
        auto f = CellFun<T>(v, std::move(s));
        _lazyCell = std::make_shared<Susp<Cell<T>>>(f);
    }
    Stream(std::function<Cell<T>()> f)
        : _lazyCell(std::make_shared<Susp<Cell<T>>>(f))
    {}
    bool isEmpty() const
    {
        return !_lazyCell;
    }
    T get() const
    {
        return _lazyCell->get().val();
    }
    Stream<T> popped_front() const
    {
        return _lazyCell->get().popped_front();
    }
    // for debugging only
    bool isForced() const
    {
        return !isEmpty() && _lazyCell->isForced();
    }
    // Lazy take
    Stream take(int n) const
    {
        if (n == 0 || isEmpty())
            return Stream();
        auto cell = _lazyCell;
        return Stream([cell, n]()
        {
            auto v = cell->get().val();
            auto t = cell->get().popped_front();
            return Cell<T>(v, t.take(n - 1));
        });
    }
    Stream drop(int n) const
    {
        if (n == 0)
            return *this;
        if (isEmpty())
            return Stream();
        auto t = popped_front();
        return t.drop(n - 1);
    }
    // Lazy reversed
    Stream reversed() const
    {
        return rev(Stream());
    }
private:
    Stream rev(Stream acc) const
    {
        if (isEmpty())
            return acc;
        auto v = get();
        auto t = popped_front();
        Stream nextAcc([=]
        {
            return Cell<T>(v, acc);
        });
        return t.rev(nextAcc);
    }
};

// Lazy concatentation of two streams

template<class T>
Stream<T> concat( Stream<T> lft
                , Stream<T> rgt)
{
    if (lft.isEmpty())
        return rgt;
    return Stream<T>([=]()
    {
        return Cell<T>(lft.get(), concat<T>(lft.popped_front(), rgt));
    });
}

template<class T, class U, class F>
auto zipWith(F f, Stream<T> lft, Stream<U> rgt) -> Stream<decltype(f(lft.get(), rgt.get()))>
{
    using S = decltype(f(lft.get(), rgt.get()));
    if (lft.isEmpty() || rgt.isEmpty())
        return Stream<S>();
    return Stream<S>([=]()
    {
        return Cell<S>(f(lft.get(), rgt.get()), zipWith(f, lft.popped_front(), rgt.popped_front()));
    });
}

Stream<int> ints(int n)
{
    return Stream<int>([=]()
    {
        return Cell<int>(n, ints(n + 1));
    });
}

template<class T, class F>
void forEach(Stream<T> strm, F f)
{
    while (!strm.isEmpty())
    {
        f(strm.get());
        strm = strm.popped_front();
    }
}


// Lazy FIFO queue with two lazy streams

template<class T>
class Queue
{
public:
    Queue() : _lenF(0), _lenR(0) {}
    Queue(int lf, Stream<T> f, int lr, Stream<T> r)
        : _lenF(lf), _front(std::move(f)), _lenR(lr), _rear(std::move(r))
    {}
    bool isEmpty() const { return _lenF == 0; }
    Queue pushed_back(T x) const
    {
        return check(_lenF, _front, _lenR + 1, Stream<T>(x, _rear));
    }
    T front() const { return _front.get(); }
    
    Queue popped_front() const
    {
        return check(_lenF - 1, _front.popped_front(), _lenR, _rear);
    }
    // for debugging only
    int lenF() const { return _lenF; }
    int lenR() const { return _lenR; }
    Stream<T> const & fStream() const { return _front; }
    Stream<T> const & rStream() const { return _rear; }
private:
    static Queue check(int lf, Stream<T> f, int lr, Stream<T> r)
    {
        if (lr <= lf) return Queue(lf, f, lr, r);
        // Left stream is a lazy concatenation and reversed
        return Queue(lf + lr, concat(f, r.reversed()), 0, Stream<T>());
    }
private:
    int _lenF;
    Stream<T> _front;
    int _lenR;
    Stream<T> _rear;
};
