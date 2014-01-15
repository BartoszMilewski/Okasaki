#include <memory>
#include <cassert>

template<class T>
class Heap
{
private:
    struct Tree
    {
        Tree(T v) : _rank(1), _v(v) {}
        Tree(int rank, T v, Heap<T> const & left,Heap<T> const & right)
        : _rank(rank), _v(v), _left(left), _right(right)
        {}

        int _rank;
        T   _v;
        Heap<T> _left;
        Heap<T> _right;
    };
    std::shared_ptr<Tree> _tree;
private:
    Heap<T> & left() const { return _tree->_left; }
    Heap<T> & right() const { return _tree->_right; }
    void assertInv() const 
    { 
        if (!isEmpty())
        {
            // left bias
            assert(left().rank() >= right().rank());
            left().assertInv();
            right().assertInv();
        }
    }
    Heap(T x, Heap const & a, Heap const & b)
    {
        a.assertInv();
        b.assertInv();
        assert(a.isEmpty() || x <= a.front());
        assert(b.isEmpty() || x <= b.front());
        // rank is the length of the right spine
        if (a.rank() >= b.rank())
            _tree = std::make_shared<Tree>(b.rank() + 1, x, a, b);
        else
            _tree = std::make_shared<Tree>(a.rank() + 1, x, b, a);
        assertInv();
    }
public:
    Heap() {}
    Heap(T x) : _tree(std::make_shared<Tree>(x))
    {}
    Heap(std::initializer_list<T> init)
    {
        Heap h;
        for (T v : init)
        {
            h = h.insert(v);
        }
        _tree = h._tree;
        assertInv();
    }
    bool isEmpty() const { return !_tree; }
    int rank() const
    {
        if (isEmpty()) return 0;
        else return _tree->_rank;
    }
    T front() const { return _tree->_v; }
    Heap pop_front() const 
    {
        return merge(left(), right()); 
    }
    Heap insert(T x)
    {
        return merge(Heap(x), *this);
    }
    // merge along right spines: log time
    static Heap merge(Heap const & h1, Heap const & h2)
    {
        if (h1.isEmpty())
            return h2;
        if (h2.isEmpty())
            return h1;
        if (h1.front() <= h2.front())
            return Heap(h1.front(), h1.left(), merge(h1.right(), h2));
        else
            return Heap(h2.front(), h2.left(), merge(h1, h2.right()));
    }
};