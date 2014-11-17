#include "Queue.h"

void test()
{
    Queue<int> q;
    auto q1 = q.pushed_back(1);
    q1.printq();
    auto q2 = q1.pushed_back(2);
    q1.printq();
    q2.printq();
    auto q3 = q2.pushed_back(3);
    q1.printq();
    q2.printq();
    q3.printq();
    std::cout << "--Popping\n\n";
    std::cout << "pop " << q3.front() << std::endl;
    auto q4 = q3.popped_front();
    q4.printq();
    std::cout << "pop " << q4.front() << std::endl;
    auto q5 = q4.popped_front();
    q5.printq();
    std::cout << "pop " << q5.front() << std::endl;
    auto q6 = q5.popped_front();
    q6.printq();
}

void main()
{
    test();
}