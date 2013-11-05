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
	T const & force()
	{
		// There probably is a better, lock-free, 
		// implememntation of this singleton
		std::lock_guard<std::mutex> lck(_mtx);
		if (!_memo)
			_memo = _f();
		return *_memo;
	}
private:
	std::unique_ptr<T> mutable _memo;
	std::function<std::unique_ptr<T>()> _f;
	std::mutex _mtx;
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
	Stream<T> tail() const { return _item->tail(); }
private:
	std::unique_ptr<const Item> _item;
};

template<class T>
using FutCell = std::function<std::unique_ptr<const Cell<T>>()>;

// Streams are ref-counted
// Stream contains (lazy) Cell
template<class T>
class Stream
{
public:
	Stream() = delete;
	Stream(FutCell<T> f)
		: _lazyCell(std::make_shared<Susp<const Cell<T>>>(f))
	{}
	bool isEmpty() const
	{
		return _lazyCell->force().isEmpty();
	}
	T get() const
	{
		return _lazyCell->force().val();
	}
	Stream<T> tail() const
	{
		return _lazyCell->force().tail();
	}
private:
	std::shared_ptr < Susp<const Cell<T>>> _lazyCell;
};

template<class T>
class CellGen
{
public:
	CellGen(T v, Stream<T> const & s) : _v(v), _s(s) {}

	std::unique_ptr<Cell<T>> operator()()
	{
		return std::unique_ptr<Cell<T>>(new Cell<T>(_v, _s));
	}
	T _v;
	Stream<T> _s;
};

template<class T>
class EmptyCellGen
{
public:
	EmptyCellGen() {}
	std::unique_ptr<Cell<T>> operator()()
	{
		return std::unique_ptr<Cell<T>>(new Cell<T>());
	}
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
		auto tail = lft.tail();
		return std::unique_ptr<const Cell<T>>(new Cell<T>(val, concat<T>(tail, rgt)));
	});
}

