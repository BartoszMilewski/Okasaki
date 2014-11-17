#include "List.h"
#include <future>
#include <vector>
#include <algorithm>

List<int> fillList(int a, int b)
{
    List<int> lst;
    for (int i = a; i < b; ++i)
        lst = lst.pushed_front(i);
    return lst;
}

void combineL(std::vector<List<int>>::const_iterator it, 
              std::vector<List<int>>::const_iterator end, 
              std::vector<int> & acc)
{
    int sum = 0;
    std::vector<List<int>> tails;
    while (it != end)
    {
        if (it->isEmpty())
            return;
        sum += it->front();
        tails.push_back(it->popped_front());
        ++it;
    }
    acc.push_back(sum);
    combineL(tails.begin(), tails.end(), acc);
}

std::vector<int> combineLists(std::vector<List<int>> const & lsts)
{
    std::vector<int> acc;
    combineL(lsts.begin(), lsts.end(), acc);
    return acc;
}

std::vector<List<int>> run()
{
    const int N = 2;
    std::vector<std::future<List<int>>> v;
    for (int i = 0; i < N; ++i)
    {
        auto lstF = std::async(std::launch::async, fillList, N * i, N * i + N);
        v.push_back(std::move(lstF));
    }
    std::vector<List<int>> lsts;
    for (auto & fut : v)
    {
        auto lst = fut.get();
        lsts.push_back(lst);
    }
    return lsts;
}

void test()
{
    auto lsts = run();
    for (auto lst : lsts)
        print(lst);

    auto sums = combineLists(lsts);
    for (auto s : sums)
        std::cout << s << ", ";
    std::cout << std::endl;
}

void main()
{
    test();
}
