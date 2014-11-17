#include "Queue.h"
#include <iostream>

template<class T>
void lazyPrint(Stream<T> const & s)
{
    if (s.isEmpty())
    {
        std::cout << "[]" << std::endl;
    }
    else if (s.isForced())
    {
        T v = s.get();
        auto tail = s.popped_front();
        std::cout << v << " ";
        lazyPrint(tail);
    }
    else
    {
        std::cout << "[?]\n";
    }
}

template<class T>
void printQ(Queue<T> const & q)
{
    std::cout << q.lenF() << ", ";
    lazyPrint(q.fStream());
    std::cout << q.lenR() << ", ";
    lazyPrint(q.rStream());
}

void main()
{
    Queue<int> q0;
    auto q1 = q0.pushed_back(10);
    printQ(q1);
    auto q2 = q1.pushed_back(20);
    printQ(q2);
    auto q3 = q2.pushed_back(30);
    printQ(q3);
    auto x = q3.front();
    std::cout << "Pop: " << x << std::endl;
    printQ(q3);
    std::cout << "Tail\n";
    auto t1 = q3.popped_front();
    printQ(t1);
}