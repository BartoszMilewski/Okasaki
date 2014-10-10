#include <cassert>
#include <memory>

enum Color { R, B };

// 1. No red node has a red child.
// 2. Every path from rootKey to empty node contains the same
// number of black nodes.

template<class K, class V>
class RBMap
{
    struct Node
    {
        Node(Color c,
            std::shared_ptr<const Node> const & lft,
            K key, V val,
            std::shared_ptr<const Node> const & rgt)
            : _c(c), _lft(lft), _key(key), _val(val), _rgt(rgt)
        {}
        Color _c;
        std::shared_ptr<const Node> _lft;
        K _key;
        V _val;
        std::shared_ptr<const Node> _rgt;
    };
    explicit RBMap(std::shared_ptr<const Node> const & node) : _root(node) {}
    Color rootColor() const
    {
        assert(!isEmpty());
        return _root->_c;
    }
public:
    RBMap() {}
    RBMap(Color c, RBMap const & lft, K key, V val, RBMap const & rgt)
        : _root(std::make_shared<const Node>(c, lft._root, key, val, rgt._root))
    {
        assert(lft.isEmpty() || lft.rootKey() < key);
        assert(rgt.isEmpty() || key < rgt.rootKey());
    }
    bool isEmpty() const { return !_root; }
    K rootKey() const
    {
        assert(!isEmpty());
        return _root->_key;
    }
    V rootValue() const
    {
        assert(!isEmpty());
        return _root->_val;
    }
    RBMap left() const
    {
        assert(!isEmpty());
        return RBMap(_root->_lft);
    }
    RBMap right() const
    {
        assert(!isEmpty());
        return RBMap(_root->_rgt);
    }
    bool member(K x) const
    {
        if (isEmpty())
            return false;
        K y = rootKey();
        if (x < y)
            return left().member(x);
        else if (y < x)
            return right().member(x);
        else
            return true;
    }
    V findWithDefault(V dflt, K key) const
    {
        if (isEmpty())
            return dflt;
        K y = rootKey();
        if (key < y)
            return left().findWithDefault(dflt, key);
        else if (y < key)
            return right().findWithDefault(dflt, key);
        else
            return rootValue();
    }
    RBMap inserted(K x, V v) const
    {
        RBMap t = ins(x, v);
        return RBMap(B, t.left(), t.rootKey(), t.rootValue(), t.right());
    }
    template<class F>
    RBMap insertedWith(K k, V v, F combine)
    {
        RBMap t = insWith(k, v, combine);
        return RBMap(B, t.left(), t.rootKey(), t.rootValue(), t.right());
    }
    // 1. No red node has a red child.
    void assert1() const
    {
        if (!isEmpty())
        {
            auto lft = left();
            auto rgt = right();
            if (rootColor() == R)
            {
                assert(lft.isEmpty() || lft.rootColor() == B);
                assert(rgt.isEmpty() || rgt.rootColor() == B);
            }
            lft.assert1();
            rgt.assert1();
        }
    }
    // 2. Every path from root to empty node contains the same
    // number of black nodes.
    int countB() const
    {
        if (isEmpty())
            return 0;
        int lft = left().countB();
        int rgt = right().countB();
        assert(lft == rgt);
        return (rootColor() == B) ? 1 + lft : lft;
    }
private:
    RBMap ins(K x, V v) const
    {
        assert1();
        if (isEmpty())
            return RBMap(R, RBMap(), x, v, RBMap());
        K y = rootKey();
        V yv = rootValue();
        Color c = rootColor();
        if (rootColor() == B)
        {
            if (x < y)
                return balance(left().ins(x, v), y, yv, right());
            else if (y < x)
                return balance(left(), y, yv, right().ins(x, v));
            else
                return *this; // no duplicates
        }
        else
        {
            if (x < y)
                return RBMap(c, left().ins(x, v), y, yv, right());
            else if (y < x)
                return RBMap(c, left(), y, yv, right().ins(x, v));
            else
                return *this; // no duplicates
        }
    }
    template<class F>
    RBMap insWith(K x, V v, F combine) const
    {
        assert1();
        if (isEmpty())
            return RBMap(R, RBMap(), x, v, RBMap());
        K y = rootKey();
        V yv = rootValue();
        Color c = rootColor();
        if (rootColor() == B)
        {
            if (x < y)
                return balance(left().insWith(x, v, combine), y, yv, right());
            else if (y < x)
                return balance(left(), y, yv, right().insWith(x, v, combine));
            else
                return RBMap(c, left(), y, combine(yv, v), right());
        }
        else
        {
            if (x < y)
                return RBMap(c, left().insWith(x, v, combine), y, yv, right());
            else if (y < x)
                return RBMap(c, left(), y, yv, right().insWith(x, v, combine));
            else
                return RBMap(c, left(), y, combine(yv, v), right());
        }
    }
    // Called only when parent is black
    static RBMap balance(RBMap const & lft, K x, V v, RBMap const & rgt)
    {
        if (lft.doubledLeft())
            return RBMap(R
            , lft.left().paint(B)
            , lft.rootKey()
            , lft.rootValue()
            , RBMap(B, lft.right(), x, v, rgt));
        else if (lft.doubledRight())
            return RBMap(R
            , RBMap(B, lft.left(), lft.rootKey(), lft.rootValue(), lft.right().left())
            , lft.right().rootKey()
            , lft.right().rootValue()
            , RBMap(B, lft.right().right(), x, v, rgt));
        else if (rgt.doubledLeft())
            return RBMap(R
            , RBMap(B, lft, x, v, rgt.left().left())
            , rgt.left().rootKey()
            , rgt.left().rootValue()
            , RBMap(B, rgt.left().right(), rgt.rootKey(), rgt.rootValue(), rgt.right()));
        else if (rgt.doubledRight())
            return RBMap(R
            , RBMap(B, lft, x, v, rgt.left())
            , rgt.rootKey()
            , rgt.rootValue()
            , rgt.right().paint(B));
        else
            return RBMap(B, lft, x, v, rgt);
    }
    bool doubledLeft() const
    {
        return !isEmpty()
            && rootColor() == R
            && !left().isEmpty()
            && left().rootColor() == R;
    }
    bool doubledRight() const
    {
        return !isEmpty()
            && rootColor() == R
            && !right().isEmpty()
            && right().rootColor() == R;
    }
    RBMap paint(Color c) const
    {
        assert(!isEmpty());
        return RBMap(c, left(), rootKey(), rootValue(), right());
    }
private:
    std::shared_ptr<const Node> _root;
};

template<class K, class V, class F>
void forEach(RBMap<K, V> const & t, F f) {
    if (!t.isEmpty()) {
        forEach(t.left(), f);
        f(t.rootKey(), t.rootValue());
        forEach(t.right(), f);
    }
}

template<class K, class V, class I>
RBMap<K, V> fromListOfPairs(I beg, I end)
{
    RBMap<K, V> map;
    for (auto it = beg; it != end; ++it)
        map = map.inserted(it->first, it->second);
    return map;
}

template<class K, class V>
void print(RBMap<K, V> const & map)
{
    forEach(map, [](K k, V v) {
        std::cout << k << "-> " << v << std::endl;
    });
    std::cout << std::endl;
}

template<class K, class V>
std::ostream& operator<<(std::ostream& os, RBMap<K, V> const & map)
{
    forEach(map, [&os](K k, V v) {
        os << k << "-> " << v << std::endl;
    });
    os << std::endl;
    return os;
}

