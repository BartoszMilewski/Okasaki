#include <cassert>
#include <functional>
#include <memory>
#include <mutex>

template<class T>
class Susp
{
public:
    Susp(std::function<std::unique_ptr<T>()> const & f)
        : _f(f)
    {}
    T const & force() const
    {
        // There probably is a better, lock-free, 
        // implememntation of this singleton
        std::lock_guard<std::mutex> lck(_mtx);
        if (!_memo)
            _memo = _f();
        return *_memo;
    }
    bool isForced() const 
    {
        std::lock_guard<std::mutex> lck(_mtx);
        return !!_memo;
    }
private:
    std::unique_ptr<T> mutable _memo;
    std::function<std::unique_ptr<T>()> _f;
    std::mutex mutable _mtx;
};

template<class T>
class Stream;

// Cell is a unique owner of an Item
// Item is a shared owner of Stream

template<class T>
class Cell
{
    class Item
    {
    public:
        Item(T v, Stream<T> const & tail)
            : _v(v), _tail(tail)
        {}
        T val() const { return _v; }
        Stream<T> tail() const { return _tail; }
    private:
        T _v;
        Stream<T> _tail;
    };
public:
    Cell() {}
    Cell(T v, Stream<T> const & tail)
        : _item(new Item(v, tail))
    {}
    Cell(const Cell &) = delete;

    bool isEmpty() const { return !_item; }
    T val() const { return _item->val(); }
    Stream<T> popped_front() const { return _item->tail(); }
private:
    std::unique_ptr<const Item> _item;
};

template<class T>
using FutCell = std::function<std::unique_ptr<const Cell<T>>()>;

template<class T>
class CellFun
{
public:
    CellFun(T v, Stream<T> const & s) : _v(v), _s(s) {}
    explicit CellFun(T v) : _v(v) {}
    std::unique_ptr<Cell<T>> operator()()
    {
        return std::unique_ptr<Cell<T>>(new Cell<T>(_v, _s));
    }
    T _v;
    Stream<T> _s;
};

// Streams are ref-counted
// Stream contains (lazy) Cell
template<class T>
class Stream
{
public:
    Stream() {}
    explicit Stream(T v)
    {
        FutCell<T> f = CellFun<T>(v);
        _lazyCell = std::make_shared<Susp<const Cell<T>>>(f);
    }
    Stream(T v, Stream const & s)
    {
        FutCell<T> f = CellFun<T>(v, s);
        _lazyCell = std::make_shared<Susp<const Cell<T>>>(f);
    }
    Stream(FutCell<T> f)
        : _lazyCell(std::make_shared<Susp<const Cell<T>>>(f))
    {}
    bool isEmpty() const
    {
        return !_lazyCell;
    }
    T get() const
    {
        return _lazyCell->force().val();
    }
    Stream<T> popped_front() const
    {
        return _lazyCell->force().popped_front();
    }
    // for debugging only
    bool isForced() const
    {
        return !isEmpty() && _lazyCell->isForced();
    }
    Stream take(int n) const
    {
        if (n == 0 || isEmpty())
            return Stream();
        auto v = get();
        auto t = popped_front();
        return Stream([=]()
        {
            return std::unique_ptr<Cell<T>>(new Cell<T>(v, t.take(n - 1)));
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
            return std::unique_ptr<Cell<T>>(new Cell<T>(v, acc));
        });
        return t.rev(nextAcc);
    }
private:
    std::shared_ptr < Susp<const Cell<T>>> _lazyCell;
};


template<class T>
Stream<T> concat(Stream<T> const & lft
                ,Stream<T> const & rgt)
{
    if (lft.isEmpty())
        return rgt;
    return Stream<T>([=]()
    {
        T val = lft.get();
        auto tail = lft.popped_front();
        return std::unique_ptr<const Cell<T>>(new Cell<T>(val, concat<T>(tail, rgt)));
    });
}

