#include "ListGC.h"
#include <iostream>
#include <string>
#include <iterator>

List<char> test1()
{
    List<char> lst0;
    auto lst1 = lst0.pushed_front('a');
    // a: ref 1
    print(lst1);
    auto lst2 = lst1.pushed_front('b');
    // b: ref 1
    // a: ref 2
    print(lst2);
    return lst2;
    // lst1 is deleted decrementing ref count on a
}

List<char> test2()
{
    auto lst3 = test1(); // {b, a}
    // b: ref 1
    // a: ref 1
    print(lst3);
    auto lst4 = lst3.insertedAt(1, 'x'); // {b, x, a}
    // x: ref 1
    // b: ref 2
    // a: ref 1
    print (lst4);
    auto lst5 = concat(lst3, lst4);
    print(lst5);
    return lst5;
}

void testHigher()
{
    std::string str = "abcd";
    auto lst = fromIt(str.begin(), str.end());
    auto lst2 = fmap<char>(toupper, lst);
    print(lst2);
    auto result = foldr([](char c, std::string str)
    {
        return str + c;
    }, std::string(), lst2);
    std::cout << result << std::endl;
    auto result2 = foldl([](std::string str, char c)
    {
        return str + c;
    }, std::string(), lst2);
    std::cout << result2 << std::endl;

    std::string mix = "TooMuchInformation";
    auto lst3 = filter(isupper, fromIt(mix.begin(), mix.end()));
    print(lst3);
}

void testLst()
{
    List<int> lst1{ 1, 2, 3, 4, 5, 6, 7 };
    print(lst1);
    auto lst = test2();
    print(lst);
}

void main()
{
    testLst();
    testHigher();
}

