#include <cassert>
#include <type_traits>
#include <iostream>

// Imagine garbage collection

template<class T>
class List
{
	struct Item
	{
		Item(T v, Item const * tail) : _val(v), _next(tail) {}
		T _val;
		Item const * _next;
	};
	friend Item;
	explicit List (Item const * items) : _head(items) {}
public:
	// Empty list
	List() : _head(nullptr) {}
	// Cons
	List(T v, List tail) : _head(new Item(v, tail._head)) {}
	// Optionally, use initializer list

	bool isEmpty() const { return !_head; }
	T head() const
	{
		assert(!isEmpty());
		return _head->_val;
	}
	List tail() const
	{
		assert(!isEmpty());
		return List(_head->_next);
	}
	// Additional utilities
	List prepend(T v) const
	{
		return List(v, *this);
	}
	List insertAt(int i, T v) const
	{
		if (i == 0)
			return prepend(v);
		else {
			assert(!isEmpty());
			return List(head(), tail().insertAt(i - 1, v));
		}
	}
private:
	// Old C++ trick to encode a Maybe value
	Item const * _head;
};

template<class T>
List<T> concat(List<T> a, List<T> b)
{
	if (a.isEmpty())
		return b;
	return List<T>(a.head(), concat(a.tail(), b));
}

template<class U, class T, class F>
List<U> fmap(F f, List<T> lst)
// requires Convertible<F, U(T)>
{
	// static_assert(std::is_convertible<F, U(T)>::value, "fmap requires a function type U(T)");
	if (lst.isEmpty()) 
		return List<U>();
	else
		return List<U>(f(lst.head()), fmap<U>(f, lst.tail()));
}

template<class T, class U, class F>
// requires Convertible<F, U(T, U)>
U foldr(F f, U acc, List<T> lst)
{
	if (lst.isEmpty())
		return acc;
	else
		return f(lst.head(), foldr(f, acc, lst.tail()));
}



template<class T, class U, class F>
// requires Convertible<F, U(U, T)>
U foldl(F f, U acc, List<T> lst)
{
	if (lst.isEmpty())
		return acc;
	else
		return foldl(f, f(acc, lst.head()), lst.tail());
}

template<class T, class F>
void forEach(List<T> lst, F f) 
{
	if (!lst.isEmpty()) {
		f(lst.head());
		forEach(lst.tail(), f);
	}
}

template<class Beg, class End>
auto fromIt(Beg it, End end) -> List<typename Beg::value_type>
{
	typedef typename Beg::value_type T;
	if (it == end)
		return List<T>();
	T item = *it;
	return List<T>(item, fromIt(++it, end));
}

template<class T>
void print(List<T> lst)
{
	forEach(lst, [](T v) 
	{
		std::cout << "(" << v << ") "; 
	});
	std::cout << std::endl;
}