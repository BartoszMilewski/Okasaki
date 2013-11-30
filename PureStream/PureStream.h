#include <cassert>
#include <functional>
#include <memory>
#include <atomic>

// This is a suspension for value of type T
// Value is accessed through a thunk
// Originally the thunk is set to "thunkForce",
// which will force the evaluation and memoization
// of the suspended value
// After that the thunk is switched to "thunkGet",
// which simply accesses the memoized value

template<class T>
class Susp
{
    // thunk
    static T const & thunkForce(Susp * susp)
    {
        return susp->setMemo();
    }
    // thunk
    static T const & thunkGet(Susp * susp)
    {
        return susp->getMemo();
    }

    T const & getMemo()
    {
        return _memo;
    }
    T const & setMemo()
    {
        _memo = _f();
        // release barrier (_memo becomes visible)
        _thunk.store(&thunkGet, std::memory_order_release);
        return getMemo();
    }
public:
    explicit Susp(std::function<T()> f)
        : _f(f), _thunk(&thunkForce), _memo(T())
    {}
    T const & get() 
    {
        // acquire barrier
        auto f = _thunk.load(std::memory_order_acquire);
        return f(this);
    }
    // We use it for debugging
	bool isForced() const
	{
        // acquire barrier
        auto f = _thunk.load(std::memory_order_acquire);
		return f == &thunkGet;
	}
private:
    // Atomic pointer to function with release/acquire semantics
    mutable std::atomic<T const & (*)(Susp *)> _thunk;
    mutable T   _memo;

    std::function<T()> _f;
};

template<class T>
class Stream;

// A Cell may be Empty or contain (own) an Item
// An Item contains a value and a (lazy) Stream

template<class T>
class Cell
{
public:
	Cell() {} // only to initialize _memo
	Cell(T v, Stream<T> const & tail)
		: _v(v), _tail(tail)
	{}
	T val() const 
	{
		return _v;
	}
	Stream<T> pop_front() const
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
	CellFun(T v, Stream<T> const & s) : _v(v), _s(s) {}

	Cell<T> operator()()
	{
		return Cell<T>(_v, _s);
	}
	T _v;
	Stream<T> _s;
};

// Streams are ref-counted
// Stream contains a suspended Cell

template<class T>
class Stream
{
private:
	std::shared_ptr <Susp<Cell<T>>> _lazyCell;
public:
	Stream() {}
	Stream(T v, Stream const & s)
	{
		auto f = CellFun<T>(v, s);
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
	Stream<T> pop_front() const
	{
		return _lazyCell->get().pop_front();
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
			auto t = cell->get().pop_front();
			return Cell<T>(v, t.take(n - 1));
		});
	}
	Stream drop(int n) const
	{
		if (n == 0)
			return *this;
		if (isEmpty())
			return Stream();
		auto t = pop_front();
		return t.drop(n - 1);
	}
    // Lazy reverse
	Stream reverse() const
	{
		return rev(Stream());
	}
private:
	Stream rev(Stream acc) const
	{
		if (isEmpty())
			return acc;
		auto v = get();
		auto t = pop_front();
		Stream nextAcc([=]
		{
			return Cell<T>(v, acc);
		});
		return t.rev(nextAcc);
	}
};

// Lazy concatentation of two streams

template<class T>
Stream<T> concat(Stream<T> const & lft
	, Stream<T> const & rgt)
{
	if (lft.isEmpty())
		return rgt;
	return Stream<T>([=]()
	{
		T val = lft.get();
		auto tail = lft.pop_front();
		return Cell<T>(val, concat<T>(tail, rgt));
	});
}

// Lazy FIFO queue with two lazy streams

template<class T>
class Queue
{
public:
	Queue() : _lenF(0), _lenR(0) {}
	Queue(int lf, Stream<T> f, int lr, Stream<T> r)
		: _lenF(lf), _front(f), _lenR(lr), _rear(r)
	{}
	bool isEmpty() const { return _lenF == 0; }
	Queue push_back(T x) const
	{
		return check(_lenF, _front, _lenR + 1, Stream<T>(x, _rear));
	}
	T front() const { return _front.get(); }
	
	Queue pop_front() const
	{
		return check(_lenF - 1, _front.pop_front(), _lenR, _rear);
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
        // Left stream is a lazy concatenation and reverse
		return Queue(lf + lr, concat(f, r.reverse()), 0, Stream<T>());
	}
private:
	int _lenF;
	Stream<T> _front;
	int _lenR;
	Stream<T> _rear;
};
