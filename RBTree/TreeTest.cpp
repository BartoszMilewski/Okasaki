#include "RBTree.h"
#include <iostream>
#include <string>
#include <algorithm>

template<class T>
void print(RBTree<T> const & t)
{
    forEach(t, [](T v)
    {
        std::cout << v << " ";
    });
    std::cout << std::endl;
}

void testInit()
{
    RBTree<int> t{ 50, 40, 30, 10, 20, 30, 100, 0, 45, 55, 25, 15 };
    print(t);
}


void main()
{
    testInit();
    std::string init =  "a red black tree walks into a bar "
                        "has johnny walker on the rocks "
                        "and quickly rebalances itself."
                        "A RED BLACK TREE WALKS INTO A BAR "
                        "HAS JOHNNY WALKER ON THE ROCKS "
                        "AND QUICKLY REBALANCES ITSELF.";
    auto t = inserted(RBTree<char>(), init.begin(), init.end());
    print(t);
    t.assert1();
    std::cout << "Black depth: " << t.countB() << std::endl;
    std::cout << "Member z: " << t.member('z') << std::endl;
    std::for_each(init.begin(), init.end(), [t](char c) 
    {
        if (!t.member(c))
            std::cout << "Error: " << c << " not found\n";
    });
}