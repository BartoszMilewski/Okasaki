#include <cassert>
#include <functional>
#include <memory>
#include <mutex>

template<class T>
using FutPtr = std::function<std::unique_ptr<T>()>;

template<class T>
class Susp
{
public:
	Susp(std::function<std::unique_ptr<T>()> const & f)
		: _f(f)
	{}
	T * force()
	{
		// There probably is a better, lock-free, 
		// implememntation of a singleton
		std::lock_guard lck(_mtx);
		if (!_memo)
			_memo = _f();
		return _memo;
	}
private:
	std::unique_ptr<T> * _memo;
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
		Item(T v, std::shared_ptr<const Stream<T>> const & tail)
			: _v(v), _tail(tail)
		{}
		T val() const { return _v; }
		std::shared_ptr<const Stream<T>> tail() const { return _tail; }
	private:
		T _v;
		std::shared_ptr<const Stream<T>> _tail;
	};
public:
	Cell() {}
	Cell(T v, std::shared_ptr<const Stream<T>> const & tail)
		: _item(new Item(v, tail))
	{}
	bool isEmpty() const { return !_item; }
	T val() const { return _item->val(); }
	std::shared_ptr<const Stream<T>> tail() const { return _item->tail(); }
private:
	std::unique_ptr<const Item> _item;
};

template<class T>
using FutCell = std::function<std::unique_ptr<const Cell<T>>()>;

// Streams are to be shared
// Stream contains (lazy) Cell
template<class T>
class Stream
{
public:
	Stream(FutCell<T> f)
		: _futCell(f)
	{}
	bool isEmpty() const
	{
		return force().isEmpty();
	}
	T get() const
	{
		return force().val();
	}
	std::shared_ptr<const Stream<T>> tail() const
	{
		return force().tail();
	}
private:
	Cell<T> const & force() const // not thread safe
	{
		if (!_memo)
		{
			std::unique_ptr<const Cell<T>> cell = _futCell();
			// ((Stream<T> *) this)->
			_memo = std::move(cell);
		}
		return *_memo;
	}
private:
	std::unique_ptr<const Cell<T>>  mutable _memo;
	FutCell<T>						_futCell;
};

template<class T>
std::shared_ptr<const Stream<T>> concat( std::shared_ptr<const Stream<T>> const & lft
										,std::shared_ptr<const Stream<T>> const & rgt)
{
	if (lft->isEmpty())
		return rgt;
	return std::make_shared<const Stream<T>>([=]()
	{
		T val = lft->get();
		auto tail = lft->tail();
		return std::unique_ptr<const Cell<T>>(new Cell<T>(val, concat<T>(tail, rgt)));
	});
}

