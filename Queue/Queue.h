#include <cassert>
#include <memory>
#include "List.h"

// Performance problems when used persistently
// When multiple copies have to perform check

template<class T>
class Queue
{
public:
    Queue(){}
    Queue(List<T> const & front, List<T> const & rear)
        :_front(front), _rear(rear) {}
    bool isEmpty() const { return _front.isEmpty(); }
    T front()
    {
        assert(!isEmpty());
        return _front.front();
    }
    Queue popped_front()
    {
        assert(!isEmpty());
        return check(_front.popped_front(), _rear);
    }
    Queue pushed_back(T v)
    {
        return check(_front, List<T>(v, _rear));
    }
    void printq()
    {
        print(_front);
        std::cout << " | ";
        print(_rear);
        std::cout << std::endl;
    }
private:
    static Queue check(List<T> const & f, List<T> const & r)
    {
        if (f.isEmpty())
        {
            if (!r.isEmpty())
                std::cout << "> Reversing\n";
            return Queue(reversed(r), List<T>());
        }
        else
            return Queue(f, r);
    }
    List<T> _front;
    List<T> _rear; // reversedd
};
