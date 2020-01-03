#include <memory>
#include <cassert>

template<class T>
class Heap
{
private:
    struct Tree
    {
        Tree(T v) : _rank(1), _v(v) {}
        Tree(int rank
            , T v
            , std::shared_ptr<const Tree> const & left
            , std::shared_ptr<const Tree> const & right)
        : _rank(rank), _v(v), _left(left), _right(right)
        {}

        int _rank;
        T   _v;
        std::shared_ptr<const Tree> _left;
        std::shared_ptr<const Tree> _right;
    };
    std::shared_ptr<const Tree> _tree;
private:
    explicit Heap(std::shared_ptr<const Tree> const & tree) 
        : _tree(tree) {}
    Heap(T x, Heap const & a, Heap const & b)
    {
        a.assertInv();
        b.assertInv();
        assert(a.isEmpty() || x <= a.front());
        assert(b.isEmpty() || x <= b.front());
        // rank is the length of the right spine
        if (a.rank() >= b.rank())
            _tree = std::make_shared<const Tree>(b.rank() + 1, x, a._tree, b._tree);
        else
            _tree = std::make_shared<const Tree>(a.rank() + 1, x, b._tree, a._tree);
        assertInv();
    }
    Heap<T>  left() const
    {
        assert(!isEmpty());
        return Heap(_tree->_left);
    }
    Heap<T> right() const
    {
        assert(!isEmpty());
        return Heap(_tree->_right);
    }
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
public:
    Heap() {}
    explicit Heap(T x) : _tree(std::make_shared<const Tree>(x))
    {}
    Heap(std::initializer_list<T> init)
    {
        _tree = heapify(init.begin(), init.end())._tree;
        assertInv();
    }
    template<class Iter>
    static Heap heapify(Iter b, Iter e)
    {
        if (b == e)
            return Heap();
        if (e - b == 1)
            return Heap(*b);
        else
        {
            Iter mid = b + (e - b) / 2;
            return merged(heapify(b, mid), heapify(mid, e));
        }
    }
    bool isEmpty() const { return !_tree; }
    int rank() const
    {
        if (isEmpty()) return 0;
        else return _tree->_rank;
    }
    T front() const { return _tree->_v; }
    Heap popped_front() const 
    {
        return merged(left(), right()); 
    }
    Heap inserted(T x)
    {
        return merged(Heap(x), *this);
    }
    // merged along right spines: log time
    static Heap merged(Heap const & h1, Heap const & h2)
    {
        if (h1.isEmpty())
            return h2;
        if (h2.isEmpty())
            return h1;
        if (h1.front() <= h2.front())
            return Heap(h1.front(), h1.left(), merged(h1.right(), h2));
        else
            return Heap(h2.front(), h2.left(), merged(h1, h2.right()));
    }
};