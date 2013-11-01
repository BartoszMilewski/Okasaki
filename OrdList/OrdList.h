#include <cassert>
#include <memory>
#include <initializer_list>

template<class T>
// requires Ord<T>
class OrdList
{
	struct Item
	{
		Item(T v, std::shared_ptr<const Item> const & tail) : _val(v), _next(tail) {}
		T _val;
		std::shared_ptr<const Item> _next;
	};
	friend Item;
	explicit OrdList(std::shared_ptr<const Item> const & items) : _head(items) {}
public:
	// Empty list
	OrdList() {}
	// Cons
	OrdList(T v, OrdList const & tail) : _head(std::make_shared<Item>(v, tail._head))
	{
		assert(tail.isEmpty() || v <= tail.head());
	}
	bool isEmpty() const { return !_head; } // conversion to bool
	T head() const
	{
		assert(!isEmpty());
		return _head->_val;
	}
	OrdList tail() const
	{
		assert(!isEmpty());
		return OrdList(_head->_next);
	}
	// Additional utilities
	OrdList insert(T v) const
	{
		if (isEmpty() || v <= head())
			return OrdList(v, OrdList(_head));
		else {
			return OrdList<T>(head(), tail().insert(v));
		}
	}
	// For debugging
	int headCount() const { return _head.use_count(); }
private:
	std::shared_ptr<const Item> _head;
};


template<class T>
OrdList<T> merge(OrdList<T> const & a, OrdList<T> const & b)
{
	if (a.isEmpty())
		return b;
	if (b.isEmpty())
		return a;
	if (a.head() <= b.head())
		return OrdList<T>(a.head(), merge(a.tail(), b));
	else
		return OrdList<T>(b.head(), merge(a, b.tail()));
}

// For debugging
template<class T>
void print(OrdList<T> lst)
{
	if (lst.isEmpty()) {
		std::cout << std::endl;
	}
	else {
		std::cout << "(" << lst.head() << ", " << lst.headCount() - 1 << ") ";
		print(lst.tail());
	}
}
