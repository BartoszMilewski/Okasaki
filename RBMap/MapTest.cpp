#include "RBMap.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

void main()
{
    RBMap<int, std::string> map;
    auto map1 = map.insert(7, "foo").insert(1, "bar").insert(4, "baz");
    std::cout << map1 << std::endl;
    auto s = map1.findWithDefault("none", 3);
    std::cout << s << std::endl;
    auto s1 = map1.findWithDefault("none", 4);
    std::cout << s1 << std::endl;

    std::vector<std::pair<char, std::string>> v = { { 'c', "cockney" }, { 'a', "abba" }, { 'b', "bbc" } };
    auto map2 = fromListOfPairs<char, std::string>(v.begin(), v.end());
    std::cout << map2 << std::endl;
};
