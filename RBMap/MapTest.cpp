#include <iostream>
#include "RBMap.h"
#include <string>
#include <algorithm>
#include <vector>

template <typename T>
struct ChooseNewest
{
    T operator()(const T& oldest, const T& newest) const
    {
        (void)oldest;
        return newest;
    }
};

int main()
{
    RBMap<int, std::string> map;
    auto map1 = map.inserted(7, "foo").inserted(1, "bar").inserted(4, "baz");
    std::cout << map1 << std::endl;
    auto s = map1.findWithDefault("none", 3);
    std::cout << s << std::endl;
    auto s1 = map1.findWithDefault("none", 4);
    std::cout << s1 << std::endl;

    std::vector<std::pair<char, std::string>> v = { { 'c', "cockney" }, { 'a', "abba" }, { 'b', "bbc" } };
    auto map2 = fromListOfPairs<char, std::string>(v.begin(), v.end());
    std::cout << map2 << std::endl;

    auto map3 = map.inserted(3, "three").inserted(3, "iii");
    std::cout << "When a value already exists, it does *not* get replaced: " << map3.findWithDefault("none", 3) << std::endl;

    auto map4 = map.inserted(3, "three").insertedWith(3, "iii", ChooseNewest<std::string>());
    std::cout << "Use insertedWith to overwrite a value: " << map4.findWithDefault("none", 3) << std::endl;
    return 0;
};
