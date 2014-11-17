#include <memory>
#include <cassert>
#include <initializer_list>

template<class T>
class Tree
{
    struct Node
    {
        Node(std::shared_ptr<const Node> const & lft, T val, std::shared_ptr<const Node> const & rgt)
            : _lft(lft), _val(val), _rgt(rgt)
        {}
        std::shared_ptr<const Node> _lft;
        T      _val;
        std::shared_ptr<const Node> _rgt;
    };
    explicit Tree(std::shared_ptr<const Node> const & node) : _root(node) {} 
public:
    Tree() {}
    Tree(Tree const & lft, T val, Tree const & rgt)
        : _root(std::make_shared<const Node>(lft._root, val, rgt._root))
    {
        assert(lft.isEmpty() || lft.root() < val);
        assert(rgt.isEmpty() || val < rgt.root());
    }
    Tree(std::initializer_list<T> init)
    {
        Tree t;
        for (T v: init)
        {
            t = t.inserted(v);
        }
        _root = t._root;
    }
    bool isEmpty() const { return !_root; }
    T root() const
    {
        assert(!isEmpty());
        return _root->_val;
    }
    Tree left() const
    {
        assert(!isEmpty());
        return Tree(_root->_lft);
    }
    Tree right() const
    {
        assert(!isEmpty());
        return Tree(_root->_rgt);
    }
    bool member(T x) const
    {
        if (isEmpty())
            return false;
        T y = root();
        if (x < y)
            return left().member(x);
        else if (y < x)
            return right().member(x);
        else
            return true;
    }
    Tree inserted(T x) const
    {
        if (isEmpty())
            return Tree(Tree(), x, Tree());
        T y = root();
        if (x < y)
            return Tree(left().inserted(x), y, right());
        else if (y < x)
            return Tree(left(), y, right().inserted(x));
        else
            return *this; // no duplicates
    }
private:
    std::shared_ptr<const Node> _root;
};

template<class T, class F>
void forEach(Tree<T> t, F f) {
    if (!t.isEmpty()) {
        forEach(t.left(), f);
        f(t.root());
        forEach(t.right(), f);
    }
}
