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
        assert(tail.isEmpty() || v <= tail.front());
    }
    bool isEmpty() const { return !_head; } // conversion to bool
    T front() const
    {
        assert(!isEmpty());
        return _head->_val;
    }
    OrdList popped_front() const
    {
        assert(!isEmpty());
        return OrdList(_head->_next);
    }
    // Additional utilities
    OrdList inserted(T v) const
    {
        if (isEmpty() || v <= front())
            return OrdList(v, OrdList(_head));
        else {
            return OrdList<T>(front(), popped_front().inserted(v));
        }
    }
    // For debugging
    int headCount() const { return _head.use_count(); }
private:
    std::shared_ptr<const Item> _head;
};


template<class T>
OrdList<T> merged(OrdList<T> const & a, OrdList<T> const & b)
{
    if (a.isEmpty())
        return b;
    if (b.isEmpty())
        return a;
    if (a.front() <= b.front())
        return OrdList<T>(a.front(), merged(a.popped_front(), b));
    else
        return OrdList<T>(b.front(), merged(a, b.popped_front()));
}

// For debugging
template<class T>
void print(OrdList<T> lst)
{
    if (lst.isEmpty()) {
        std::cout << std::endl;
    }
    else {
        std::cout << "(" << lst.front() << ", " << lst.headCount() - 1 << ") ";
        print(lst.popped_front());
    }
}
