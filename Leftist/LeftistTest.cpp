#include "LeftistHeap.h"
#include <iostream>

template<class T>
void printHeap(Heap<T> const & h)
{
    if (h.isEmpty())
        std::cout << std::endl;
    else
    {
        std::cout << h.front() << ", ";
        printHeap(h.popped_front());
    }
}

void testInit()
{
    Heap<int> h{ 50, 40, 30, 10, 20, 30, 100, 0, 45, 55, 25, 15 };
    printHeap(h);
}

void main()
{
    Heap<int> h;
    printHeap(h);
    Heap<int> h1(100);
    printHeap(h1);
    Heap<int> h2 = h1.inserted(102);
    printHeap(h2);
    testInit();
}