#include "LazyStream.h"

template<class T>
class Queue
{
public:
    Queue() : _lenF(0), _lenR(0) {}
    Queue(int lf, Stream<T> f, int lr, Stream<T> r)
        : _lenF(lf), _front(f), _lenR(lr), _rear(r)
    {}
    bool isEmpty() const { return _lenF == 0; }
    Queue pushed_back(T x) const
    {
        return check(_lenF, _front, _lenR + 1, Stream<T>(x, _rear));
    }
    T front() const { return _front.get(); }
    
    Queue popped_front() const
    {
        return check(_lenF - 1, _front.popped_front(), _lenR, _rear);
    }
    // for debugging only
    int lenF() const { return _lenF; }
    int lenR() const { return _lenR; }
    Stream<T> const & fStream() const { return _front; }
    Stream<T> const & rStream() const { return _rear; }
private:
    static Queue check(int lf, Stream<T> f, int lr, Stream<T> r)
    {
        if (lr <= lf) return Queue(lf, f, lr, r);
        return Queue(lf + lr, concat(f, r.reversed()), 0, Stream<T>());
    }
private:
    int _lenF;
    Stream<T> _front;
    int _lenR;
    Stream<T> _rear;
};