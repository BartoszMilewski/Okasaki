#if !defined (UTILS_H)
#define UTILS_H
#include <vector>
#include <ostream>

template<class T>
std::vector<T> concatAll(std::vector<std::vector<T>> && in)
{
    std::vector<T> res;
    std::for_each(in.begin(), in.end(), [&res](std::vector<T> const & v){
        std::move(v.begin(), v.end(), std::back_inserter(res));
    });
    return res;
}

template<class T, class U>
std::ostream& operator<<(std::ostream& os, std::pair<T, U> const & p)
{
    os << "(" << p.first << ", " << p.second << ") ";
    return os;
}

#endif
