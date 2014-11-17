#include "OrdList.h"
#include <iostream>

void testOrdLst()
{
    OrdList<char> lst0;
    auto lst1 = lst0.inserted('m');
    print(lst1);
    auto lst2 = lst1.inserted('p');
    print(lst2);
    auto lst3 = lst2.inserted('c');
    auto lst4 = lst1.inserted('d');
    auto lst5 = lst1.inserted('e');
    std::cout << "Merging\n";
    print(lst3);
    print(lst5);
    auto lst = merged(lst3, lst5);
    print(lst);
}

void main() 
{
    testOrdLst();
}

