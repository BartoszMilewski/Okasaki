#if ! defined(LIST_H)
#define LIST_H

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
        Item(T v, std::shared_ptr<const Item> tail) 
            : _val(v), _next(std::move(tail)) 
        {}
        // singleton
        explicit Item(T v) : _val(v) {}
        //~Item() { std::cout << "~" << _val << std::endl; }
        T _val;
        std::shared_ptr<const Item> _next;
    };
    friend Item;
    explicit List(std::shared_ptr<const Item> items) 
        : _head(std::move(items)) {}
public:
    // Empty list
    List() {}
   // Cons
    List(T v, List const & tail) 
        : _head(std::make_shared<Item>(v, tail._head)) {}
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
    List popped_front() const
    {
        assert(!isEmpty());
        return List(_head->_next);
    }
    // Additional utilities
    List pushed_front(T v) const
    {
        return List(v, *this);
    }
    List take(int n)
    {
        if (n <= 0 || isEmpty()) return List();
        return popped_front().take(n - 1).pushed_front(front());
    }
    List insertedAt(int i, T v) const
    {
        if (i == 0)
            return pushed_front(v);
        else {
            assert(!isEmpty());
            return List<T>(front(), popped_front().insertedAt(i - 1, v));
        }
    }
    List removed(T v) const
    {
        if (isEmpty()) return List();
        if (v == front())
            return popped_front().removed(v);
        return List(front(), popped_front().removed(v));
    }
    List removed1(T v) const
    {
        if (isEmpty()) return List();
        if (v == front())
            return popped_front();
        return List(front(), popped_front().removed(v));
    }
    bool member(T v) const
    {
        if (isEmpty()) return false;
        if (v == front()) return true;
        return popped_front().member(v);
    }
    template<class F>
    void forEach(F f) const
    {
        Item const * it = _head.get();
        while (it != nullptr)
        {
            f(it->_val);
            it = it->_next.get();
        }
    }
    
    friend class FwdListIter<T>;
    // For debugging
    int headCount() const { return _head.use_count(); }
private:
    std::shared_ptr<const Item> _head;
};

template<class T, class P>
bool all(List<T> const & lst, P & p)
{
    if (lst.isEmpty())
        return true;
    if (!p(lst.front()))
        return false;
    return all(lst.popped_front(), p);
}

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
    return List<T>(a.front(), concat(a.popped_front(), b));
}

template<class T, class F>
auto fmap(F f, List<T> lst) -> List<decltype(f(lst.front()))>
{
    using U = decltype(f(lst.front()));
    static_assert(std::is_convertible<F, std::function<U(T)>>::value,
        "fmap requires a function type U(T)");
    if (lst.isEmpty()) 
        return List<U>();
    else
        return List<U>(f(lst.front()), fmap(f, lst.popped_front()));
}

template<class T, class P>
List<T> filter(P p, List<T> lst)
{
    static_assert(std::is_convertible<P, std::function<bool(T)>>::value, 
                 "filter requires a function type bool(T)");
    if (lst.isEmpty())
        return List<T>();
    if (p(lst.front()))
        return List<T>(lst.front(), filter(p, lst.popped_front()));
    else
        return filter(p, lst.popped_front());
}

template<class T, class U, class F>
U foldr(F f, U acc, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(T, U)>>::value, 
                 "foldr requires a function type U(T, U)");
    if (lst.isEmpty())
        return acc;
    else
        return f(lst.front(), foldr(f, acc, lst.popped_front()));
}

template<class T, class U, class F>
U foldl(F f, U acc, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(U, T)>>::value, 
                 "foldl requires a function type U(U, T)");
    if (lst.isEmpty())
        return acc;
    else
        return foldl(f, f(acc, lst.front()), lst.popped_front());
}

// Set difference a \ b
template<class T>
List<T> set_diff(List<T> const & as, List<T> const & bs)
{
    return foldl([](List<T> const & acc, T x) {
        return acc.removed(x);
    }, as, bs);
}

// Set union of two lists, xs u ys
// Assume no duplicates inside either list
template<class T>
List<T> set_union(List<T> const & xs, List<T> const & ys)
{
    // xs u ys = (ys \ xs) ++ xs
    // removed all xs from ys
    auto trimmed = foldl([](List<T> const & acc, T x) {
        return acc.removed(x);
    }, ys, xs);
    return concat(trimmed, xs);
}

template<class T>
List<T> concatAll(List<List<T>> const & xss)
{
    if (xss.isEmpty()) return List<T>();
    return concat(xss.front(), concatAll(xss.popped_front()));
}

// consumes the list when called: 
// forEach(std::move(lst), f);

template<class T, class F>
void forEach(List<T> lst, F f) 
{
    static_assert(std::is_convertible<F, std::function<void(T)>>::value, 
                 "forEach requires a function type void(T)");
    while (!lst.isEmpty()) {
        f(lst.front());
        lst = lst.popped_front();
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
    return iterateN(f, f(init), count - 1).pushed_front(init);
}

// Pass lst by value not reference!
template<class T>
void printRaw(List<T> lst)
{
    if (lst.isEmpty()) {
        std::cout << std::endl;
    }
    else {
        std::cout << "(" << lst.front() << ", " << lst.headCount() - 1 << ") ";
        printRaw(lst.popped_front());
    }
}

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
List<T> reversed(List<T> const & lst)
{
    return foldl([](List<T> const & acc, T v)
    {
        return List<T>(v, acc);
    }, List<T>(), lst);
}

#endif
