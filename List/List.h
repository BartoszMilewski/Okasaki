#include <cassert>
#include <memory>
#include <iostream> // debugging

template<class T>
class List
{
	struct Item
	{
		Item(T v, std::shared_ptr<const Item> const & tail) : _val(v), _next(tail) {}
		// For debugging only
		~Item() { std::cout << "! " << _val << std::endl; }
		T _val;
		std::shared_ptr<const Item> _next;
	};
	friend Item;
	explicit List(std::shared_ptr<const Item> const & items) : _head(items) {}
public:
	// Empty list
	List() {}
	// Cons
	List(T v, List const & tail) : _head(std::make_shared<Item>(v, tail._head)) {}
	// Optionally, use initializer list

	bool isEmpty() const { return !_head; } // conversion to bool
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
			return List<T>(head(), tail().insertAt(i - 1, v));
		}
	}
	// For debugging
	int headCount() const { return _head.use_count(); }
private:
	std::shared_ptr<const Item> _head;
};


template<class T>
List<T> concat(List<T> const & a, List<T> const & b)
{
	if (a.isEmpty())
		return b;
	return List<T>(a.head(), concat(a.tail(), b));
}

template<class T, class F>
void forEach(List<T> lst, F f) {
	if (!lst.isEmpty()) {
		f(lst.head());
		forEach(lst.tail(), f);
	}
}


template<class T, class U, class F>
List<U> fmap(F f, List<T> lst)
{
	if (isEmpty(lst)) 
		return List<U>();
	else
		return List<U>(f(lst.head()), map(f, lst.tail()));
}

template<class T, class U, class F>
U foldl(F f, U acc, List<T> lst)
{
	if (lst.isEmpty())
		return acc;
	else
		return foldl(f, f(acc, lst.head()), lst.tail());
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

template<class T>
void print(List<T> lst)
{
	if (lst.isEmpty()) {
		std::cout << std::endl;
	}
	else {
		std::cout << "(" << lst.head() << ", " << lst.headCount() - 1 << ") ";
		print(lst.tail());
	}
}
