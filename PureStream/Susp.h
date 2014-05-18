// This is a suspension for value of type T
// Value is accessed through a thunk
// Originally the thunk is set to "thunkForce",
// which will force the evaluation and memoization
// of the suspended value
// After that the thunk is switched to "thunkGet",
// which simply accesses the memoized value

// This is not thread safe!

template<class T>
class Susp
{
    // thunk
    static T const & thunkForce(Susp const * susp)
    {
        return susp->setMemo();
    }
    // thunk
    static T const & thunkGet(Susp const * susp)
    {
        return susp->getMemo();
    }

    T const & getMemo() const
    {
        return _memo;
    }
    T const & setMemo() const
    {
        _memo = _f();
        _thunk = thunkGet;
        return getMemo();
    }
public:
    Susp() {} // needed if you want to suspend a Susp
    explicit Susp(std::function<T()> f)
        : _f(f), _thunk(&thunkForce), _memo(T())
    {}
    T const & get() const
    {
        return _thunk(this);
    }
    // We use it for debugging
    bool isForced() const
    {
        return _thunk == &thunkGet;
    }
private:
    typedef T const & (*thunk_t)(Susp const *);
    mutable thunk_t _thunk;
    mutable T   _memo;

    std::function<T()> _f;
};

template<class T, class F>
auto fmap(Susp<T> const & susp, F f) -> Susp<decltype(f(susp.get()))>
{
    using S = decltype(f(susp.get()));
    static_assert(std::is_convertible<F, std::function<S(T)>>::value,
        "fmap requires a function type S(T)");

    return Susp<S>([susp, f]()
    {
        return f(susp.get());
    });
}

template<class T>
Susp<T> mjoin(Susp<Susp<T>> const & ssusp)
{
    return Susp<T>([ssusp]()
    {
        return ssusp.get().get();
    });
}

template<class T, class F>
auto mbind(Susp<T> const & susp, F f)->decltype(f(susp.get()))
{
    using SSus = decltype(f(susp.get()));
    static_assert(std::is_convertible<F, std::function<SSus(T)>>::value,
        "mbind requires a function type Susp<S>(T)");
    return SSus([susp, f]()
    {
        return f(susp.get()).get();
    });
}

template<class T>
Susp<T> munit(T t)
{
    return Susp<T>([t]()
    {
        return t;
    });
}