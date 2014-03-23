#include <cassert>
#include <memory>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <iostream> // print

template<class T> class FwdListIter;

template<class T>
class List
{
    struct Item
    {
        Item(T v, std::shared_ptr<const Item> const & tail) : _val(v), _next(tail) {}
        // singleton
        explicit Item(T v) : _val(v) {}
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
    // Singleton
    explicit List(T v) : _head(std::make_shared<Item>(v)) {}
    // From initializer list
    List(std::initializer_list<T> init)
    {
        for (auto it = std::rbegin(init); it != std::rend(init); ++it)
        {
            _head = std::make_shared<Item>(*it, _head);
        }
    }

    bool isEmpty() const { return !_head; } // conversion to bool
    T front() const
    {
        assert(!isEmpty());
        return _head->_val;
    }
    List pop_front() const
    {
        assert(!isEmpty());
        return List(_head->_next);
    }
    // Additional utilities
    List push_front(T v) const
    {
        return List(v, *this);
    }
    List take(int n)
    {
        if (n <= 0 || isEmpty()) return List();
        return pop_front().take(n - 1).push_front(front());
    }
    List insertAt(int i, T v) const
    {
        if (i == 0)
            return push_front(v);
        else {
            assert(!isEmpty());
            return List<T>(front(), pop_front().insertAt(i - 1, v));
        }
    }
    List remove(T v) const
    {
        if (isEmpty()) return List();
        if (v == front())
            return pop_front().remove(v);
        return List(front(), pop_front().remove(v));
    }
    bool member(T v) const
    {
        if (isEmpty()) return false;
        if (v == front()) return true;
        return pop_front().member(v);
    }
	
	friend class FwdListIter<T>;
    // For debugging
    int headCount() const { return _head.use_count(); }
private:
    std::shared_ptr<const Item> _head;
};

template<class T>
class FwdListIter : public std::iterator<std::forward_iterator_tag, T>
{
public:
	FwdListIter() {} // end
	FwdListIter(List<T> const & lst) : _cur(lst._head)
	{}
	T operator*() const { return _cur->_val; }
	FwdListIter & operator++()
	{
		_cur = _cur->_next;
		return *this;
	}
	bool operator==(FwdListIter<T> const & other)
	{
		return _cur == other._cur;
	}
	bool operator!=(FwdListIter<T> const & other)
	{
		return !(*this == other);
	}
private:
	std::shared_ptr<const typename List<T>::Item> _cur;
};

template<class T>
class OutListIter : public std::iterator<std::output_iterator_tag, T>
{
public:
	OutListIter() {}
	T & operator*() { return _val; }
	OutListIter & operator++()
	{
		_lst = List<T>(_val, _lst);
		return *this;
	}
	List<T> getList() const { return _lst; }
private:
	T _val;
	List<T> _lst;
};


namespace std
{
	template<class T> 
	FwdListIter<T> begin(List<T> const & lst)
	{
		return FwdListIter<T>(lst);
	}
	template<class T> 
	FwdListIter<T> end(List<T> const & lst)
	{
		return FwdListIter<T>();
	}
}


template<class T>
List<T> concat(List<T> const & a, List<T> const & b)
{
    if (a.isEmpty())
        return b;
    return List<T>(a.front(), concat(a.pop_front(), b));
}

template<class U, class T, class F>
List<U> fmap(F f, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(T)>>::value, 
                 "fmap requires a function type U(T)");
    if (lst.isEmpty()) 
        return List<U>();
    else
        return List<U>(f(lst.front()), fmap<U>(f, lst.pop_front()));
}

template<class T, class P>
List<T> filter(P p, List<T> lst)
{
    static_assert(std::is_convertible<P, std::function<bool(T)>>::value, 
                 "filter requires a function type bool(T)");
    if (lst.isEmpty())
        return List<T>();
    if (p(lst.front()))
        return List<T>(lst.front(), filter(p, lst.pop_front()));
    else
        return filter(p, lst.pop_front());
}

template<class T, class U, class F>
U foldr(F f, U acc, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(T, U)>>::value, 
                 "foldr requires a function type U(T, U)");
    if (lst.isEmpty())
        return acc;
    else
        return f(lst.front(), foldr(f, acc, lst.pop_front()));
}

template<class T, class U, class F>
U foldl(F f, U acc, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(U, T)>>::value, 
                 "foldl requires a function type U(U, T)");
    if (lst.isEmpty())
        return acc;
    else
        return foldl(f, f(acc, lst.front()), lst.pop_front());
}

// Set difference a \ b
template<class T>
List<T> diff(List<T> const & as, List<T> const & bs)
{
    return foldl([](List<T> const & acc, T x) {
        return acc.remove(x);
    }, as, bs);
}

// Set union of two lists, xs u ys
// Assume no duplicates inside either list
template<class T>
List<T> unionize(List<T> const & xs, List<T> const & ys)
{
    // xs u ys = (ys \ xs) ++ xs
    // remove all xs from ys
    auto trimmed = foldl([](List<T> const & acc, T x) {
        return acc.remove(x);
    }, ys, xs);
    return concat(trimmed, xs);
}

template<class T>
List<T> concatAll(List<List<T>> const & xss)
{
    if (xss.isEmpty()) return List<T>();
    return concat(xss.front(), concatAll(xss.pop_front()));
}

template<class T, class F>
void forEach(List<T> lst, F f) 
{
    static_assert(std::is_convertible<F, std::function<void(T)>>::value, 
                 "forEach requires a function type void(T)");
    if (!lst.isEmpty()) {
        f(lst.front());
        forEach(lst.pop_front(), f);
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

template<class T, class F>
List<T> iterateN(F f, T init, int count)
{
    if (count <= 0) return List<T>();
    return iterateN(f, f(init), count - 1).push_front(init);
}

/*
// Pass lst by value not reference!
template<class T>
void print(List<T> lst)
{
    if (lst.isEmpty()) {
        std::cout << std::endl;
    }
    else {
        std::cout << "(" << lst.front() << ", " << lst.headCount() - 1 << ") ";
        print(lst.pop_front());
    }
}
*/

template<class T>
std::ostream& operator<<(std::ostream& os, List<T> const & lst)
{
    os << "[";
    forEach(lst, [&os](T v) {
        os << v << " ";
    });
    os << "]";
    return os;
}

template<class T>
List<T> reverse(List<T> const & lst)
{
    return foldl([](List<T> const & acc, T v)
    {
        return List<T>(v, acc);
    }, List<T>(), lst);
}
